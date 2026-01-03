#ifndef __PWM_HW_H
#define __PWM_HW_H

#include <stdint.h>
#include <stdbool.h>
#include "../gpio/gpio.h"

void app_pwm_hw_init(void);
void app_set_pwm_hw_fade(pwm_hw_pins pin, uint16_t target_idx, uint16_t duration_ms);
void pwm_hw_set_duty(pwm_hw_pins pin, uint16_t duty);
bool app_pwm_hw_add_pin(pwm_hw_pins hw_pin);
#endif // __PWM_H