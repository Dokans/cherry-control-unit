#pragma once
#include "driver/dac_types.h"
#include "driver/gpio.h"

//TODO: Get correct pin for all non DAC pins
const auto EZ_WHEEL_1_DAC = DAC_CHAN_0;
const auto EZ_WHEEL_1_ON_OFF = GPIO_NUM_35; 

const auto EZ_WHEEL_2_DAC = DAC_CHAN_1;
const auto EZ_WHEEL_2_ON_OFF = GPIO_NUM_45; // TODO: Think about this pin more

const auto SYNC_BUTTON = GPIO_NUM_4;

const auto HBRIDGE_L = GPIO_NUM_37;
const auto HBRIDGE_H = GPIO_NUM_18;
const auto HSS_CONTROL = GPIO_NUM_10;
