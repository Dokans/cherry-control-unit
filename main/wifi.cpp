#include "wifi.hpp"
#include "packet.hpp"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "string.h"
// Internet protocol stuff
#include "lwip/sockets.h"
#include "lwip/netdb.h"


static const char *TAG = "WiFi";

static int s_retry_num = 0;
static EventGroupHandle_t s_wifi_event_group;


#define WIFI_SSID "MotorUnit"
#define WIFI_PASS "nevim"
#define COM_PORT 6767

SemaphoreHandle_t tx_mutex = xSemaphoreCreateBinary();

// Handler functions for UDP tasks
static void wifi_motor_task(void *pvParams);
static void wifi_handle_task(void *pvParams);

static void wifi_ap_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d, reason=%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    }
}

static void wifi_sta_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        s_retry_num++;
        ESP_LOGI(TAG, "retry to connect to the AP, try: %u", s_retry_num);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, BIT0);
    }
}

void wifi_ap_init(){
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, 
        &wifi_ap_event_handler, NULL, NULL));
    
    
    wifi_config_t wifi_cfg = {
        .ap = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .ssid_len = strlen(WIFI_SSID),
            .channel = 10,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .max_connection = 10,
            .pmf_cfg = {
                .required = true,
            },
        },
    };

    esp_wifi_set_mode(WIFI_MODE_AP);
    esp_wifi_set_config(WIFI_IF_AP, &wifi_cfg);
    esp_wifi_start();
    ESP_LOGI(TAG, "Wifi AP Started");
    xTaskCreate(wifi_motor_task, "udp_server", 4096, NULL, 5, NULL);  // Start UDP server task
};

void wifi_sta_init(){
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_sta_event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifi_sta_event_handler,
                                                        NULL,
                                                        &instance_got_ip));
    
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold = {
                .authmode = WIFI_AUTH_WPA2_PSK,
            },
            .sae_pwe_h2e = WPA3_SAE_PWE_HASH_TO_ELEMENT,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    xTaskCreate(wifi_handle_task, "udp_client", 4096, NULL, 5, NULL);
}

static void wifi_motor_task(void *pvParams){
    char rx_buffer[128];
    char addr_str[128];
    int addr_family = AF_INET;
    int ip_protocol = IPPROTO_IP;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(COM_PORT);

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "Socket created");

    int err = bind(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
    if (err < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        close(sock);
        vTaskDelete(NULL);
        return;
    }
    ESP_LOGI(TAG, "UDP server listening on port %d", COM_PORT);

    while (1) {
        ESP_LOGI(TAG, "Waiting for data...");

        struct sockaddr_in source_addr;
        socklen_t socklen = sizeof(source_addr);
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0,
                           (struct sockaddr *)&source_addr, &socklen);

        if (len < 0) {
            ESP_LOGE(TAG, "recvfrom failed: errno %d", errno);
            break;
        } else {
            inet_ntoa_r(source_addr.sin_addr, addr_str, sizeof(addr_str) - 1);
            ESP_LOGI(TAG, "Received %d bytes from %s:%d", len, addr_str, ntohs(source_addr.sin_port));
            packet p;
            memcpy(&p, rx_buffer, len);
            ESP_LOGI(TAG, "Packet-> Time: %lu, ADC1: %lu, ADC2: %lu, B1: %u, B2: %u", 
                p.timeStamp, p.adc1_lsb, p.adc2_lsb, p.button1, p.button2);
        }
    }

    if (sock != -1) {
        ESP_LOGI(TAG, "Shutting down socket...");
        shutdown(sock, 0);
        close(sock);
    }
    vTaskDelete(NULL);
}

static void wifi_handle_task(void *pvParams){
    // Wait for Wi-Fi connection
    xEventGroupWaitBits(s_wifi_event_group, BIT0,
                        pdFALSE, pdTRUE, portMAX_DELAY);

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr("192.168.4.1");
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(COM_PORT);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }


    while (1) {
        char payload[] = "Hello from ESP32 client";
        int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

        if (err < 0) {
            ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
            break;
        }
        ESP_LOGI(TAG, "Message sent");

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    close(sock);
    vTaskDelete(NULL);
}
