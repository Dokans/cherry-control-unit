#pragma once
#include "soc/dac_channel.h"
#include "driver/gpio.h"

//TODO: Get correct pin for all non DAC pins
const auto EZ_WHEEL_1_DAC = DAC_CHAN0_GPIO_NUM;
const auto EZ_WHEEL_1_ON_OFF = GPIO_NUM_10; 

const auto EZ_WHEEL_2_DAC = DAC_CHAN1_GPIO_NUM;
const auto EZ_WHEEL_2_ON_OFF = GPIO_NUM_11; 

const auto SYNC_BUTTON = GPIO_NUM_12; 
const auto HBRIDGE_1 = GPIO_NUM_13;
const auto HBRIDGE_2 = GPIO_NUM_14;





