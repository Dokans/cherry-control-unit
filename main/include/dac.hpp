#pragma once
#include "driver/dac.h"
#include "pinout.hpp"

namespace pins
{
    template<dac_channel_t ch>
    class dac
    {
    public:
        dac()
        {
            dac_output_enable(ch);
            set_level(0);
        }

        bool set_level(uint8_t value)
        {
            level = value;
            return dac_output_voltage(ch, level) == ESP_OK;
        }

        uint8_t get_level() const
        {
            return level;
        }

    private:
        uint8_t level = 0;
    };

    dac<EZ_WHEEL_1_DAC> ez_wheel_1_dac;
    dac<EZ_WHEEL_2_DAC> ez_wheel_2_dac;
} // namespace pins