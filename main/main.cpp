#include <stdio.h>
#include "pinout.hpp"
#include "gpio.hpp"
#include "pwm.hpp"
#include "dac.hpp"
// RTOS stuff
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
// ESP32 specific
#include "sdkconfig.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"


static const char *TAG = "Master";
void setup();
void debug_info();
void main_loop();

extern "C" void app_main(void)
{
    debug_info();
    setup();

    main_loop();
}


void debug_info(){
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    ESP_LOGI(
        TAG, "Chip: %s (%d cores) and %s%s%s%s",
        CONFIG_IDF_TARGET,
        chip_info.cores,
        (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
        (chip_info.features & CHIP_FEATURE_BT) ? "BT/" : "",
        (chip_info.features & CHIP_FEATURE_BLE) ? "BLE/" : "",
        (chip_info.features & CHIP_FEATURE_IEEE802154) ? "ZigBee" : ""
    );

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    ESP_LOGI(TAG, "Silicone revison v%d.%d", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        ESP_LOGE(TAG, "Couldn't get flash size");
    } else {
        //ESP_LOGI(TAG, "%" PRIu32 "MB %s flash", flash_size / (uint32_t)(1024*1024));
    }
}

void setup(){
    // Setup of bluetooth specific stuff, flash, event handlers and more
    pins::o_ez_wheel_1_on_off.set_level(0);
    pins::o_ez_wheel_2_on_off.set_level(1);
    pins::ez_wheel_1_dac.set_level(255);
    pins::ez_wheel_2_dac.set_level(0);

    
}

void main_loop(){
    uint8_t i = 0;
    while (true){
        ESP_LOGI(TAG, "Time since boot: %lu", xTaskGetTickCount());
        pins::o_ez_wheel_1_on_off.invert_level();
        pins::o_ez_wheel_2_on_off.invert_level();
        bool buttonState = pins::i_sync_button.get_state();

        pins::motor.set_duty(i/2.55, buttonState);
        pins::ez_wheel_1_dac.set_level(i);
        pins::ez_wheel_2_dac.set_level(255-i);

        ESP_LOGI(TAG,"Button (PA%d): %d", SYNC_BUTTON, buttonState);
        vTaskDelay(100/portTICK_PERIOD_MS);

        i += 10;
    }
}