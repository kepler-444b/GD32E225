
#if 0
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "gd32e23x.h"
#include "pwm.h"
#include "curve_table.h"
#include "../gpio/gpio.h"
#include "../device/device_manager.h"
#include "../timer/timer.h"

// ==================== 配置参数 ====================
#define SYSTEM_CLOCK_FREQ        64000000U // 系统时钟频率 72 MHz
#define TIMER_PERIOD             49U       // 定时器周期 -> 50us 中断
#define PWM_RESOLUTION           1000U     // PWM 分辨率
#define MAX_FADE_TIME_MS         5000U     // 最大渐变时间
#define FADE_UPDATE_MS           10U       // 每 10ms 更新一次渐变
#define INTERRUPTS_PER_FADE_TICK 200U      // 10ms / 50us = 200
#define PWM_MIN_DUTY             50        // 最低占空比

// 定义 MIN 宏(取较小值)
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// 定义 CLAMP 宏(限制值在 [min_val, max_val] 范围内)
#define CLAMP(x, min, max) ((x) <= (min) ? (min) : ((x) >= (max) ? (max) : (x)))

// PWM控制结构体
typedef struct {
    volatile uint16_t current_duty; // 当前PWM占空比(0-1000)
    volatile uint16_t target_duty;  // 目标PWM占空比(0-1000)
    volatile bool is_fading;        // 是否正在渐变中
    volatile uint16_t fade_counter; // 渐变计数器
    volatile uint16_t fade_steps;   // 渐变总步数
    bool is_active;                 // 是否已激活
    gpio_pin_t gpio;                // 对应的GPIO引脚
    uint16_t start_duty;            // 渐变起始占空比
} pwm_control_t;

static pwm_control_t pwm_channels[PWM_MAX_CHANNELS];  // PWM通道控制数组
volatile static uint8_t active_channel_count = 0;     // 已激活的pwm通道数量
volatile static bool pwm_initialized         = false; // pwm模块是否已经初始化
 
static int find_pwm_index(gpio_pin_t pin);

void pwm_update(void *arg);
void app_pwm_init(void)
{
    if (pwm_initialized) return;

    for (int i = 0; i < PWM_MAX_CHANNELS; i++) {
        pwm_channels[i].is_active = false;
    }
    active_channel_count = 0;

    // 启动 TIMER13
    rcu_periph_clock_enable(RCU_TIMER13);
    timer_deinit(TIMER13);

    timer_parameter_struct timer_cfg;
    timer_cfg.prescaler        = (SYSTEM_CLOCK_FREQ / 1000000U) - 1U; // 分频到1MHz
    timer_cfg.alignedmode      = TIMER_COUNTER_EDGE;
    timer_cfg.counterdirection = TIMER_COUNTER_UP;
    timer_cfg.period           = TIMER_PERIOD;
    timer_cfg.clockdivision    = TIMER_CKDIV_DIV1;
    timer_init(TIMER13, &timer_cfg);

    // 中断配置
    timer_interrupt_flag_clear(TIMER13, TIMER_INT_FLAG_UP);
    timer_interrupt_enable(TIMER13, TIMER_INT_UP);
    nvic_irq_enable(TIMER13_IRQn, 0);

    timer_enable(TIMER13);

    pwm_initialized = true;
    app_timer_start(10, pwm_update, true, NULL, "pwm_update");
    // APP_PRINTF("app_pwm_init ok\n");
}

void pwm_update(void *arg)
{
    app_pwm_update_fade();
}

bool app_pwm_add_pin(gpio_pin_t pin)
{
    if (!pwm_initialized || active_channel_count >= PWM_MAX_CHANNELS) {
        return false;
    }
    if (find_pwm_index(pin) >= 0) {
        return false;
    }
    for (int i = 0; i < PWM_MAX_CHANNELS; i++) {
        if (!pwm_channels[i].is_active) {
            pwm_channels[i].current_duty = 0;
            pwm_channels[i].target_duty  = 0;
            pwm_channels[i].fade_counter = 0;
            pwm_channels[i].fade_steps   = 0;
            pwm_channels[i].is_fading    = false;
            pwm_channels[i].is_active    = true;
            pwm_channels[i].gpio         = pin;

            APP_SET_GPIO(pin, true);
            active_channel_count++;
            return true;
        }
    }
    return false;
}

void app_set_pwm_duty(gpio_pin_t pin, uint16_t duty)
{
    int index = find_pwm_index(pin);
    if (index < 0) return;

    if (duty > PWM_RESOLUTION) duty = PWM_RESOLUTION;

    // 限制最低占空比
    if (duty > 0 && duty < PWM_MIN_DUTY) {
        duty = PWM_MIN_DUTY;
    }

    pwm_channels[index].current_duty = duty;
    pwm_channels[index].target_duty  = duty;
    pwm_channels[index].is_fading    = false;
}

void app_set_pwm_fade(gpio_pin_t pin, uint16_t duty, uint16_t fade_time_ms)
{
    int index = find_pwm_index(pin);
    if (index < 0) return;

    duty = CLAMP(duty, 0, PWM_RESOLUTION);
    if (fade_time_ms == 0) {
        app_set_pwm_duty(pin, duty);
        return;
    }
    fade_time_ms = MIN(fade_time_ms, MAX_FADE_TIME_MS);

    uint32_t steps = (fade_time_ms + FADE_UPDATE_MS / 2U) / FADE_UPDATE_MS;
    if (steps == 0) steps = 1;

    pwm_channels[index].start_duty   = pwm_channels[index].current_duty;
    pwm_channels[index].target_duty  = duty;
    pwm_channels[index].fade_steps   = (uint16_t)steps;
    pwm_channels[index].fade_counter = 0;
    pwm_channels[index].is_fading    = (pwm_channels[index].current_duty != duty);
}

uint16_t app_get_pwm_duty(gpio_pin_t pin)
{
    int index = find_pwm_index(pin);
    if (index < 0) return 0;
    return pwm_channels[index].current_duty;
}

void TIMER13_IRQHandler(void)
{
    if (!timer_interrupt_flag_get(TIMER13, TIMER_INT_FLAG_UP))
        return;
    timer_interrupt_flag_clear(TIMER13, TIMER_INT_FLAG_UP);
    static uint16_t pwm_counter          = 0;
    static int pwm_acc[PWM_MAX_CHANNELS] = {0}; // 抖动累积寄存器

    pwm_counter = (pwm_counter + 1) % PWM_RESOLUTION;

    for (uint8_t i = 0; i < PWM_MAX_CHANNELS; i++) {
        if (!pwm_channels[i].is_active) continue;

        // 累积误差算法
        pwm_acc[i] += pwm_channels[i].current_duty;
        bool pwm_output = false;
        if (pwm_acc[i] >= PWM_RESOLUTION) {
            pwm_output = true;
            pwm_acc[i] -= PWM_RESOLUTION;
        }
        APP_SET_GPIO(pwm_channels[i].gpio,
#if defined PWM_DIR
                     !pwm_output
#else
                     pwm_output
#endif
        );
    }
}

void app_pwm_update_fade(void)
{
    // APP_PRINTF(":%d\n",app_get_pwm_duty(PB0));
    for (int i = 0; i < PWM_MAX_CHANNELS; i++) {
        pwm_control_t *ch = &pwm_channels[i];
        if (!ch->is_active || !ch->is_fading) continue;

        ch->fade_counter++;

        uint16_t progress = ch->fade_counter * FADE_TABLE_SIZE / ch->fade_steps;
        if (progress >= FADE_TABLE_SIZE) progress = FADE_TABLE_SIZE - 1;

        uint16_t curve_value = fade_table[progress];
        int32_t range        = (int32_t)ch->target_duty - ch->start_duty;

        int32_t new_duty = ch->start_duty + (range * curve_value) / FADE_TABLE_SIZE;
        ch->current_duty = CLAMP(new_duty, 0, PWM_RESOLUTION);

        if (ch->fade_counter >= ch->fade_steps) {
            ch->current_duty = ch->target_duty;
            ch->is_fading    = false;
        }
    }
}

static int find_pwm_index(gpio_pin_t pin)
{
    for (int i = 0; i < PWM_MAX_CHANNELS; i++) {
        if (pwm_channels[i].is_active &&
            pwm_channels[i].gpio.port == pin.port &&
            pwm_channels[i].gpio.pin == pin.pin) {
            return i;
        }
    }
    return -1;
}
#endif