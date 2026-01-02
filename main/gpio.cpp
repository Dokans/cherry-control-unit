#include "gpio.hpp"

pins::GPIO<EZ_WHEEL_1_ON_OFF> o_ez_wheel_1_on_off();
pins::GPIO<EZ_WHEEL_2_ON_OFF> o_ez_wheel_2_on_off();
pins::GPIO<HBRIDGE_1>         o_hbridge_1();
pins::GPIO<HBRIDGE_2>         o_hbridge_2();
pins::GPIO<SYNC_BUTTON>       i_sync_button(GPIO_MODE_INPUT);