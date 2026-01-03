#include "plcp_light_ct_info.h"
#include <string.h>

static plcp_light_ct_pin_t light_ct_pins;
static light_ct_t powe_up_status;

void plcp_light_ct_pins_init(void)
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);

    // 2 路PWM调光,双色温
    gpio_mode_set(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO_PIN_0 | GPIO_PIN_1);
    gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0 | GPIO_PIN_1);
    gpio_af_set(GPIOB, GPIO_AF_1, GPIO_PIN_0 | GPIO_PIN_1);

    light_ct_pins.led_y = PWM_PB0;
    light_ct_pins.led_w = PWM_PB1;
}

fmc_state_enum light_ct_info_read(void)
{
    fmc_state_enum ret;
    // app_flash_erase_page(FLASH_LIGHT_CT_TABLE);
    ret = app_flash_read(FLASH_LIGHT_CT_TABLE, (uint32_t *)&powe_up_status, sizeof(powe_up_status));
    if (ret != FMC_READY) return ret;
    powe_up_status.grad_time = 10;
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
    if (ret != FMC_READY) return ret;

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