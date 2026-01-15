#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "stdint.h"

typedef struct
{
    TickType_t timeStamp;
    uint32_t adc1_lsb;
    uint32_t adc2_lsb;
    bool     button1;
    bool     button2;

} packet;


packet assemblePacket(uint32_t adc1_lsb, uint32_t adc2_lsb, bool button1, bool button2){
    return packet {
        .timeStamp = xTaskGetTickCount(),
        .adc1_lsb = adc1_lsb,
        .adc2_lsb = adc2_lsb,
        .button1 = button1,
        .button2 = button2
    };
}

