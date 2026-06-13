#include "light_ct_adapter.h"
#include "../../Source/plcp_device/plcp_linght_ct/plcp_light_ct_info.h"
#include "../../Source/pwm/pwm_hw.h"
#include "../../Source/timer/timer.h"

#define CT_MIN 2700
#define CT_MAX 6500
#define PWM_MAX 1000

static void light_execute(const light_ct_t *ctrl);
static void delay_exe_light(void *arg);

void light_adapter_ctrl(const light_ct_t *ctrl)
{
    if (ctrl->timer > 0) { // 延时时间
        app_timer_stop("delay_exe");
        app_timer_start(ctrl->timer * 1000, delay_exe_light, false, (void *)ctrl, "delay_exe");
        return;
    }
    if (ctrl->keep_time > 0) { // 维持时间,暂未实现
        return;
    }
    if (ctrl->memory) { // 断电记忆
        APP_Save_light_ct_info(ctrl);
    }
    light_execute(ctrl);
}

void light_adapter_close(uint8_t grad_time)
{ 
    app_set_pwm_hw_fade(PWM_PB0, 0, grad_time * 100); // 暖白 PB0
    app_set_pwm_hw_fade(PWM_PB1, 0, grad_time * 100); // 白光 PB1
}

static void delay_exe_light(void *arg)
{
    if (!arg)
        return;
    light_ct_t *ctrl = (light_ct_t *)arg;
    light_execute(ctrl);
}

static void light_execute(const light_ct_t *ctrl)
{
    APP_PRINTF(
        "\n"
        "brightness      : %d\n"
        "color_temp      : %d\n"
        "P_flag          : %d\n"
        "grad_time       : %d (100ms)\n"
        "timer           : %d\n"
        "keep_time       : %d\n"
        "memory          : %d\n"
        "brightness_type : %d\n",

        ctrl->brightness,
        ctrl->color_temp,
        ctrl->P_flag,
        ctrl->grad_time,
        ctrl->timer,
        ctrl->keep_time,
        ctrl->memory,
        ctrl->brightness_type);

    uint16_t brightness = ctrl->brightness;
    uint16_t color_temp = ctrl->color_temp;

    if (ctrl->brightness_type) {
        brightness = brightness * 100; // 0~100 转为 0~10000
    }
    if (brightness > 10000)
        brightness = 10000;

    // 获取渐变时间(单位 100ms)
    uint16_t duration_ms = 0;
    switch (ctrl->P_flag) {
    case 0: // 不渐变
        duration_ms = 0;
        break;
    case 1: // 渐变,使用默认
        duration_ms = get_light_info()->grad_time * 100;
        break;
    case 2: // 渐变,后随渐变时间
        duration_ms = ctrl->grad_time * 100;
        break;
    case 3: // 跳变,随机跳变
        break;
    }

    // 色温限幅
    if (color_temp < CT_MIN)
        color_temp = CT_MIN;
    if (color_temp > CT_MAX)
        color_temp = CT_MAX;

    float ratio = (float)(color_temp - CT_MIN) / (float)(CT_MAX - CT_MIN);

    uint16_t warm_pwm = (uint16_t)(brightness * (1.0f - ratio) / 10000.0f * PWM_MAX);
    uint16_t cool_pwm = (uint16_t)(brightness * ratio / 10000.0f * PWM_MAX);

    if (warm_pwm > PWM_MAX)
        warm_pwm = PWM_MAX;
    if (cool_pwm > PWM_MAX)
        cool_pwm = PWM_MAX;
    app_set_pwm_hw_fade(PWM_PB0, warm_pwm, duration_ms);
    app_set_pwm_hw_fade(PWM_PB1, cool_pwm, duration_ms);
}
