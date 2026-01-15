#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
// Init functions for WiFi
void wifi_ap_init();
void wifi_sta_init();

extern SemaphoreHandle_t tx_mutex;
