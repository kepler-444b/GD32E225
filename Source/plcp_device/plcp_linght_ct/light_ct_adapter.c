#include "light_ct_adapter.h"
#include "../../Source/pwm/pwm_hw.h"
#include "../../Source/plcp_device/plcp_linght_ct/plcp_light_ct_info.h"
#include "../../Source/timer/timer.h"

#define CT_MIN  2700
#define CT_MAX  6500
#define PWM_MAX 1000

static void light_execute(const light_ct_t *ctrl);
static void delay_exe_light(void *arg);

void light_adapter_ctrl(const light_ct_t *ctrl)
{
    uint16_t duration_ms = 0;

    // 渐变时间判断
    if (ctrl->P_flag == 2) {
        duration_ms = ctrl->grad_time * 100;
    } else {
        duration_ms = get_light_info()->grad_time;
    }

    if (ctrl->timer > 0) { // 延时时间
        app_timer_stop("delay_exe");
        app_timer_start(ctrl->timer * 1000, delay_exe_light, false, (void *)ctrl, "delay_exe");
        return;
    }

    if (ctrl->keep_time > 0) { // 维持时间
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
    if (!arg) return;
    light_ct_t *ctrl = (light_ct_t *)arg;
    light_execute(ctrl);
}

static void light_execute(const light_ct_t *ctrl)
{
    uint16_t brightness = ctrl->brightness;
    uint16_t color_temp = ctrl->color_temp;

    if (ctrl->brightness_type) {
        brightness = brightness * 100; // 0~100 转为 0~10000
    }
    if (brightness > 10000) brightness = 10000;

    // 获取渐变时间(单位 100ms)
    uint16_t duration_ms = (ctrl->P_flag == 2) ? ctrl->grad_time * 100 : get_light_info()->grad_time;

    // 色温限幅
    if (color_temp < CT_MIN) color_temp = CT_MIN;
    if (color_temp > CT_MAX) color_temp = CT_MAX;

    float ratio = (float)(color_temp - CT_MIN) / (float)(CT_MAX - CT_MIN);

    uint16_t warm_pwm = (uint16_t)(brightness * (1.0f - ratio) / 10000.0f * PWM_MAX);
    uint16_t cool_pwm = (uint16_t)(brightness * ratio / 10000.0f * PWM_MAX);

    if (warm_pwm > PWM_MAX) warm_pwm = PWM_MAX;
    if (cool_pwm > PWM_MAX) cool_pwm = PWM_MAX;
    app_set_pwm_hw_fade(PWM_PB0, warm_pwm, duration_ms * 100);
    app_set_pwm_hw_fade(PWM_PB1, cool_pwm, duration_ms * 100);
}
