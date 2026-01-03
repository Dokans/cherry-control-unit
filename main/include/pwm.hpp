#pragma once
#include "driver/ledc.h"
#include "esp_err.h"
#include "pinout.hpp"

namespace pins
{
    class PWM
    {
    private:
        uint32_t _duty;
        bool forward;
        ledc_channel_config_t pwm_l;
        ledc_channel_config_t pwm_h;


        void apply()
        {
            if (forward){
                ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, _duty);
                ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
                ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
            } else {
                ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, 0);
                ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, _duty);
                ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
                ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);
            }
        }
        


    public:
        PWM(gpio_num_t p_pwm_h, gpio_num_t p_pwm_l){
            ledc_timer_config_t tcfg = {
                .speed_mode = LEDC_LOW_SPEED_MODE,
                .duty_resolution = LEDC_TIMER_10_BIT,
                .timer_num = LEDC_TIMER_0,
                .freq_hz = 10000,
                .clk_cfg = LEDC_AUTO_CLK,
            };
            
            ledc_timer_config(&tcfg);

            pwm_h = {
                .gpio_num = p_pwm_h,
                .speed_mode = LEDC_LOW_SPEED_MODE,
                .channel = LEDC_CHANNEL_0,
                .intr_type = LEDC_INTR_DISABLE,
                .timer_sel = LEDC_TIMER_0,
                .duty = 0,
                .hpoint = 0,   
            };

            pwm_l = {
                .gpio_num = p_pwm_l,
                .speed_mode = LEDC_LOW_SPEED_MODE,
                .channel = LEDC_CHANNEL_1,
                .intr_type = LEDC_INTR_DISABLE,
                .timer_sel = LEDC_TIMER_0,
                .duty = 0,
                .hpoint = 0,   
            };

            ledc_channel_config(&pwm_h);
            ledc_channel_config(&pwm_l);
        };

        //void frewheel();
        //void brake();
        //void forward();
        //void backward();
        void set_duty(float duty, bool forward){
            duty = duty < 0 ? 0 : duty;
            duty = duty > 100 ? 100 : duty;
            _duty = ((1 << 10) - 1) * duty;
            return apply();
        }
    };

    PWM motor(HBRIDGE_H, HBRIDGE_L);
} // namespace pins
