#include "gpio.hpp"
#include "pwm.hpp"
#include "dac.hpp"

pins::gpio<EZ_WHEEL_1_ON_OFF> o_ez_wheel_1_on_off();
pins::gpio<EZ_WHEEL_2_ON_OFF> o_ez_wheel_2_on_off();
pins::gpio<HSS_CONTROL>       o_hss_control();

pins::gpio<SYNC_BUTTON>       i_sync_button(GPIO_MODE_INPUT);

pins::dac<EZ_WHEEL_1_DAC> ez_wheel_1_dac();
pins::dac<EZ_WHEEL_2_DAC> ez_wheel_2_dac();
