#ifndef _PLCP_LIGHT_CT_INFO_H_
#define _PLCP_LIGHT_CT_INFO_H_

#include <stdint.h>
#include "../../Source/device/device_manager.h"
#include "../../Source/flash/flash.h"
#include "../../Source/gpio/gpio.h"

#define FLASH_LIGHT_CT_TABLE (0x0801B800U) // page 110

typedef struct
{
    pwm_hw_pins led_y;
    pwm_hw_pins led_w;
} plcp_light_ct_pin_t;

typedef struct {
    uint16_t brightness; // 亮度
    uint16_t color_temp; // 色温
    uint8_t P_flag;      // 渐变类型 0-3
    uint8_t grad_time;   // 渐变时间，单位100ms，仅 P_flag=2 有效
    uint32_t timer;      // 定时功能，Ti_flag=1 时有效
    uint16_t keep_time;  // 维持时间，K_flag=1 且 Ti_flag=0 时有效
    uint8_t memory;      // 断电记忆，M_flag=1 且 Ti_flag/K_flag=0 时有效
    bool brightness_type;
} light_ct_t;

// fmc_state_enum light_ct_info_read(void);
// fmc_state_enum light_ct_info_save(void);
void APP_Read_light_ct_info(void);
void APP_Save_light_ct_info(const light_ct_t *save_data);

void plcp_light_ct_pins_init(void);
const plcp_light_ct_pin_t *get_light_ct_pins(void);
const light_ct_t *get_light_info(void);
#endif