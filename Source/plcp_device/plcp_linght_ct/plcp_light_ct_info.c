#include "plcp_light_ct_info.h"
#include <string.h>

static plcp_light_ct_pin_t light_ct_pins;
static light_ct_t powe_up_status;

void plcp_light_ct_pins_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);

    // 2 路PWM调光,双色温
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_PULLUP, GPIO_PIN_0 | GPIO_PIN_1);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1);
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_0 | GPIO_PIN_1);

    light_ct_pins.led_y = PWM_PB0;
    light_ct_pins.led_w = PWM_PB1;

    // 干接点中断触发引脚
    rcu_periph_clock_enable(RCU_GPIOA);
    gpio_mode_set(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_0); // 上拉
    syscfg_exti_line_config(EXTI_SOURCE_GPIOA, EXTI_SOURCE_PIN0);        // 引脚映射到中断线
    // exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_FALLING);                // 下降沿触发
    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_BOTH); // 双边触发(上升沿/下降沿)
    exti_interrupt_flag_clear(EXTI_0);

    nvic_irq_enable(EXTI0_1_IRQn, 2);
}

fmc_state_enum light_ct_info_read(void)
{
    fmc_state_enum ret;
    // app_flash_erase_page(FLASH_LIGHT_CT_TABLE);
    ret = app_flash_read(FLASH_LIGHT_CT_TABLE, (uint32_t *)&powe_up_status, sizeof(powe_up_status));
    if (ret != FMC_READY)
        return ret;
    if (powe_up_status.brightness == 0xFFFF) { // 第一次烧录,使用默认配置
        powe_up_status.brightness = 0;
        powe_up_status.color_temp = 4600;
        powe_up_status.P_flag = 1;     // 使用默认渐变
        powe_up_status.grad_time = 10; // 100ms
        powe_up_status.timer = 0;
        powe_up_status.keep_time = 0;
        powe_up_status.memory = 0;
        powe_up_status.brightness_type = false; // 默认使用百分比亮度
    }
    APP_PRINTF("\n----- light_ct_t -----\n");

    APP_PRINTF("brightness     : %u\n", powe_up_status.brightness);
    APP_PRINTF("color_temp     : %u\n", powe_up_status.color_temp);
    APP_PRINTF("P_flag         : %u\n", powe_up_status.P_flag);
    APP_PRINTF("grad_time      : %u (x100ms)\n", powe_up_status.grad_time);
    APP_PRINTF("timer          : %lu\n", (unsigned long)powe_up_status.timer);
    APP_PRINTF("keep_time      : %u\n", powe_up_status.keep_time);
    APP_PRINTF("memory         : %u\n", powe_up_status.memory);
    APP_PRINTF("brightness_type: %s\n", powe_up_status.brightness_type ? "true" : "false");
    APP_PRINTF("----------------------\n\n");
    return ret;
}

fmc_state_enum light_ct_info_save(void)
{
    fmc_state_enum ret;

    ret = app_flash_program(FLASH_LIGHT_CT_TABLE, (uint32_t *)&powe_up_status, sizeof(powe_up_status), true);
    if (ret != FMC_READY)
        return ret;

    return ret;
}

void APP_Read_light_ct_info(void)
{
    light_ct_info_read();
}

void APP_Save_light_ct_info(const light_ct_t *save_data)
{
    APP_PRINTF("APP_Save_light_ct_info\n");

    memcpy(&powe_up_status, save_data, sizeof(powe_up_status));
    light_ct_info_save();
}

const plcp_light_ct_pin_t *get_light_ct_pins(void)
{
    return &light_ct_pins;
}
const light_ct_t *get_light_info(void)
{
    return &powe_up_status;
}