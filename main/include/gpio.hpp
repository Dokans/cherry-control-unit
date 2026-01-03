#pragma once

#include "driver/gpio.h"
#include "pinout.hpp"

namespace pins
{
    template<gpio_num_t p>
    class gpio {
    public:
        gpio(){
            gpio_reset_pin(p);
            gpioMode = GPIO_MODE_OUTPUT;
            set_direction();
        }
        explicit gpio(gpio_mode_t direction){
            gpio_reset_pin(p);
            gpioMode = direction;
            set_direction();
        }
        bool set_level(bool high){
            level = high ? 1 : 0;
            return gpio_set_level(p, level) == ESP_OK;
        }
        bool invert_level(){
            level = !level;
            return set_level(level);
        }

        bool get_state(){
            return gpio_get_level(p) == 1;
        }
    private:
        uint32_t level = 0;
        gpio_mode_t gpioMode;

        void set_direction(){
            gpio_set_direction(p, gpioMode);
        }
    };
    // Outputs
    gpio<EZ_WHEEL_1_ON_OFF> o_ez_wheel_1_on_off;
    gpio<EZ_WHEEL_2_ON_OFF> o_ez_wheel_2_on_off;
    gpio<HSS_CONTROL>       o_hss_control;
    // Inputs
    gpio<SYNC_BUTTON>       i_sync_button;
    

} // namespace gpio
