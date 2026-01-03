#include "pwm_hw.h"
#include "../../Source/timer/timer.h"
#include "../device/device_manager.h"
#include "../gpio/gpio.h"
#include "curve_table.h"
#include "gd32e23x.h"
#include <string.h>

#define PWM_FREQ_HZ 1000       // 统一PWM频率(10khz)
#define SYSTEM_CLOCK 64000000U // 系统时钟
#define PWM_FADE_CH_MAX 6
#define FADE_TIMER_MS 1

typedef struct {
    pwm_hw_pins pin;
    uint16_t start_idx;
    uint16_t target_idx;
    uint16_t current_idx;
    uint16_t fade_counter;
    uint16_t fade_steps;
    bool active;
} pwm_fade_ctrl_t;

static pwm_fade_ctrl_t fade_channels[PWM_FADE_CH_MAX];
static bool timer2_inited = false;
static bool timer15_inited = false;
static bool timer16_inited = false;
static bool timer0_inited = false;

static uint16_t timer0_period = 999;
static uint16_t timer2_period = 999;
static uint16_t timer15_period = 999;
static uint16_t timer16_period = 999;

void pwm_hw_fade_update(void *arg);
void app_pwm_hw_init(void)
{
    app_timer_start(1, pwm_hw_fade_update, true, NULL, "pwm_hw_update");
}

// ==================== 设置占空比 ====================
void pwm_hw_set_duty(pwm_hw_pins pin, uint16_t duty)
{
    if (duty > 999)
        duty = 999;

    switch (pin) {
    case PWM_PB0:
        timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_2, duty);
        break;
    case PWM_PB1:
        timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_3, duty);
        break;
    case PWM_PB6:
        timer_channel_output_pulse_value_config(TIMER15, TIMER_CH_0, duty);
        break;
    case PWM_PB7:
        timer_channel_output_pulse_value_config(TIMER16, TIMER_CH_0, duty);
        break;
    case PWM_PA8:
        timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, duty);
        break;
    default:
        break;
    }
}

void app_set_pwm_hw_fade(pwm_hw_pins pin, uint16_t target_idx, uint16_t duration_ms)
{
    if (target_idx > FADE_TABLE_SIZE - 1)
        target_idx = FADE_TABLE_SIZE - 1;
    if (duration_ms == 0)
        duration_ms = 1;

    pwm_fade_ctrl_t *ch = NULL;
    for (int i = 0; i < PWM_FADE_CH_MAX; i++) {
        if (fade_channels[i].pin == pin) {
            ch = &fade_channels[i];
            break;
        }
        if (!fade_channels[i].active && ch == NULL)
            ch = &fade_channels[i];
    }
    if (!ch)
        return;

    uint16_t current_idx = 0;
    switch (pin) {
    case PWM_PB0:
        current_idx = TIMER_CH2CV(TIMER2) * (FADE_TABLE_SIZE - 1) / timer2_period;
        break;
    case PWM_PB1:
        current_idx = TIMER_CH3CV(TIMER2) * (FADE_TABLE_SIZE - 1) / timer2_period;
        break;
    case PWM_PB6:
        current_idx = TIMER_CH0CV(TIMER15) * (FADE_TABLE_SIZE - 1) / timer15_period;
        break;
    case PWM_PB7:
        current_idx = TIMER_CH0CV(TIMER16) * (FADE_TABLE_SIZE - 1) / timer16_period;
        break;
    case PWM_PA3:
        break;
    case PWM_PA8:
        current_idx = TIMER_CH0CV(TIMER0) * (FADE_TABLE_SIZE - 1) / timer0_period;
        break;
    }
    ch->pin = pin;
    ch->start_idx = current_idx;
    ch->target_idx = target_idx;
    ch->current_idx = current_idx;
    ch->fade_steps = (duration_ms + FADE_TIMER_MS - 1) / FADE_TIMER_MS;
    ch->fade_counter = 0;
    ch->active = (ch->fade_steps > 0 && ch->start_idx != ch->target_idx);
}

void pwm_hw_fade_update(void *arg)
{
    for (int i = 0; i < PWM_FADE_CH_MAX; i++) {
        pwm_fade_ctrl_t *ch = &fade_channels[i];
        if (!ch->active)
            continue;

        ch->fade_counter++;
        uint16_t idx = ch->fade_counter * FADE_TABLE_SIZE / ch->fade_steps;
        if (idx >= FADE_TABLE_SIZE)
            idx = FADE_TABLE_SIZE - 1;

        int32_t range = (int32_t)ch->target_idx - ch->start_idx;
        int32_t new_idx = ch->start_idx + (range * idx) / FADE_TABLE_SIZE;
        if (new_idx >= FADE_TABLE_SIZE)
            new_idx = FADE_TABLE_SIZE - 1;

        uint16_t period = 0;
        switch (ch->pin) {
        case PWM_PB0:
            period = timer2_period;
            break;
        case PWM_PB1:
            period = timer2_period;
            break;
        case PWM_PB6:
            period = timer15_period;
            break;
        case PWM_PB7:
            period = timer16_period;
            break;
        case PWM_PA8:
            period = timer0_period;
            break;
        default:
            return;
        }

        uint16_t duty = fade_table[new_idx] * period / (FADE_TABLE_SIZE - 1);
        pwm_hw_set_duty(ch->pin, duty);
        ch->current_idx = new_idx;

        if (ch->fade_counter >= ch->fade_steps) {
            duty = fade_table[ch->target_idx] * period / (FADE_TABLE_SIZE - 1);
            pwm_hw_set_duty(ch->pin, duty);
            ch->active = false;
        }
    }
}

bool app_pwm_hw_add_pin(pwm_hw_pins hw_pin)
{
    timer_parameter_struct timer_initpara;
    timer_oc_parameter_struct timer_ocinitpara;
    switch (hw_pin) {
    case PWM_PB0:
    case PWM_PB1: {
        if (!timer2_inited) {
            rcu_periph_clock_enable(RCU_TIMER2);
            timer_deinit(TIMER2);

            timer_struct_para_init(&timer_initpara);
            timer_initpara.prescaler = SYSTEM_CLOCK / (PWM_FREQ_HZ * 1000) - 1; // 64MHz -> 1kHz * 1000
            timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
            timer_initpara.counterdirection = TIMER_COUNTER_UP;
            timer_initpara.period = 999; // 分辨率1000
            timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
            timer_initpara.repetitioncounter = 0;
            timer_init(TIMER2, &timer_initpara);
            timer_auto_reload_shadow_enable(TIMER2);
            timer_enable(TIMER2);

            timer2_period = timer_initpara.period;
            timer2_inited = true;
        }
        // uint16_t channel = hw_pin = PWM_PB0 ? TIMER_CH_2 : TIMER_CH_3;
        uint16_t channel = (hw_pin == PWM_PB0) ? TIMER_CH_2 : TIMER_CH_3;

        timer_channel_output_struct_para_init(&timer_ocinitpara);
        timer_ocinitpara.outputstate = TIMER_CCX_ENABLE;
        timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;
        timer_ocinitpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
        timer_ocinitpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
        timer_ocinitpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
        timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

        timer_channel_output_config(TIMER2, channel, &timer_ocinitpara);
        timer_channel_output_mode_config(TIMER2, channel, TIMER_OC_MODE_PWM0);
        timer_channel_output_shadow_config(TIMER2, channel, TIMER_OC_SHADOW_DISABLE);
        timer_channel_output_pulse_value_config(TIMER2, channel, 0);
    } break;
    case PWM_PB6: {

        if (!timer15_inited) {
            rcu_periph_clock_enable(RCU_TIMER15);
            timer_deinit(TIMER15);
            timer_struct_para_init(&timer_initpara);
            timer_initpara.prescaler = SYSTEM_CLOCK / (PWM_FREQ_HZ * 1000) - 1;
            timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
            timer_initpara.counterdirection = TIMER_COUNTER_UP;
            timer_initpara.period = 999;
            timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
            timer_initpara.repetitioncounter = 0;
            timer_init(TIMER15, &timer_initpara);
            timer_primary_output_config(TIMER15, ENABLE);
            timer_auto_reload_shadow_enable(TIMER15);
            timer_enable(TIMER15);
            timer15_period = timer_initpara.period;
            timer15_inited = true;
        }

        timer_channel_output_struct_para_init(&timer_ocinitpara);
        timer_ocinitpara.outputstate = TIMER_CCX_DISABLE;
        timer_ocinitpara.outputnstate = TIMER_CCXN_ENABLE;
        timer_ocinitpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
        timer_ocinitpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
        timer_ocinitpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
        timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

        timer_channel_output_config(TIMER15, TIMER_CH_0, &timer_ocinitpara);
        timer_channel_output_mode_config(TIMER15, TIMER_CH_0, TIMER_OC_MODE_PWM0);
        timer_channel_output_shadow_config(TIMER15, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
        timer_channel_output_pulse_value_config(TIMER15, TIMER_CH_0, 0);
    } break;
    case PWM_PB7: {
        if (!timer16_inited) {
            rcu_periph_clock_enable(RCU_TIMER16);
            timer_deinit(TIMER16);

            timer_struct_para_init(&timer_initpara);
            timer_initpara.prescaler = SYSTEM_CLOCK / (PWM_FREQ_HZ * 1000) - 1;
            timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
            timer_initpara.counterdirection = TIMER_COUNTER_UP;
            timer_initpara.period = 999;
            timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
            timer_initpara.repetitioncounter = 0;
            timer_init(TIMER16, &timer_initpara);

            timer_primary_output_config(TIMER16, ENABLE);
            timer_auto_reload_shadow_enable(TIMER16);
            timer_enable(TIMER16);

            timer16_period = timer_initpara.period;
            timer16_inited = true;
        }

        timer_channel_output_struct_para_init(&timer_ocinitpara);
        timer_ocinitpara.outputstate = TIMER_CCX_DISABLE;
        timer_ocinitpara.outputnstate = TIMER_CCXN_ENABLE;
        timer_ocinitpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
        timer_ocinitpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
        timer_ocinitpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
        timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

        timer_channel_output_config(TIMER16, TIMER_CH_0, &timer_ocinitpara);
        timer_channel_output_mode_config(TIMER16, TIMER_CH_0, TIMER_OC_MODE_PWM0);
        timer_channel_output_shadow_config(TIMER16, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
        timer_channel_output_pulse_value_config(TIMER16, TIMER_CH_0, 0);
    } break;
    case PWM_PA8: {
        if (!timer0_inited) {
            rcu_periph_clock_enable(RCU_TIMER0);
            timer_deinit(TIMER0);

            timer_struct_para_init(&timer_initpara);
            timer_initpara.prescaler = SYSTEM_CLOCK / (PWM_FREQ_HZ * 1000) - 1; // 分频
            timer_initpara.alignedmode = TIMER_COUNTER_EDGE;
            timer_initpara.counterdirection = TIMER_COUNTER_UP;
            timer_initpara.period = 999; // 分辨率1000
            timer_initpara.clockdivision = TIMER_CKDIV_DIV1;
            timer_initpara.repetitioncounter = 0;
            timer_init(TIMER0, &timer_initpara);

            timer_primary_output_config(TIMER0, ENABLE); // 高端/主输出使能
            timer_auto_reload_shadow_enable(TIMER0);
            timer_enable(TIMER0);

            timer0_period = timer_initpara.period;
            timer0_inited = true;
        }

        timer_channel_output_struct_para_init(&timer_ocinitpara);
        timer_ocinitpara.outputstate = TIMER_CCX_ENABLE;
        timer_ocinitpara.outputnstate = TIMER_CCXN_DISABLE;
        timer_ocinitpara.ocpolarity = TIMER_OC_POLARITY_HIGH;
        timer_ocinitpara.ocnpolarity = TIMER_OCN_POLARITY_HIGH;
        timer_ocinitpara.ocidlestate = TIMER_OC_IDLE_STATE_LOW;
        timer_ocinitpara.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;

        timer_channel_output_config(TIMER0, TIMER_CH_0, &timer_ocinitpara);
        timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM0);
        timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_DISABLE);
        timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, 0);
    } break;
    default:
        return false;
    }
    return true;
}
