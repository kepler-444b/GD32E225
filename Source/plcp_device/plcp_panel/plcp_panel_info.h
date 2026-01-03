#ifndef _PLCP_PANEL_INFO_
#define _PLCP_PANEL_INFO_
#include <stdint.h>
#include "../../Source/device/device_manager.h"
#include "../../Source/gpio/gpio.h"

#if defined PANEL_1KEY
#define PANEL_VOL_RANGE_DEF          \
    [0] = {.vol_range = {0, 5}},     \
    [1] = {.vol_range = {30, 40}},   \
    [2] = {.vol_range = {85, 95}},   \
    [3] = {.vol_range = {145, 166}}, \
    [4] = {.vol_range = {200, 210}}, \
    [5] = {.vol_range = {240, 250}}
#define RELAY_GPIO_MAP_DEF {PB12, PB13, PB14, PB15}                               // 继电器 GPIO 映射
#define LED_W_GPIO_MAP_DEF {PWM_PB0, PWM_PB1, PWM_PB6, PWM_PB7, PWM_PA3, PWM_PA8} // LED 白灯 GPIO
#define LED_Y_GPIO_MAP_DEF {PA15, PB3, PB4, PB5, PA1, PA2}                        // LED 黄灯 GPIO

#elif defined PANEL_4KEY || defined PANEL_2KEY

#define PANEL_VOL_RANGE_DEF          \
    [0] = {.vol_range = {25, 40}},   \
    [1] = {.vol_range = {88, 95}},   \
    [2] = {.vol_range = {145, 155}}, \
    [3] = {.vol_range = {0, 10}}

#define RELAY_GPIO_MAP_DEF {PB12, PB13, PB14, PB15}             // 继电器 GPIO 映射
#define LED_W_GPIO_MAP_DEF {PWM_PB1, PWM_PB6, PWM_PB7, PWM_PB0} // LED 白灯 GPIO
#define LED_Y_GPIO_MAP_DEF {PB3, PB4, PB5, PA15}                // LED 黄灯 GPIO

#elif defined PANEL_3KEY
#define PANEL_VOL_RANGE_DEF        \
    [0] = {.vol_range = {0, 10}},  \
    [1] = {.vol_range = {25, 40}}, \
    [2] = {.vol_range = {88, 95}}, \
    [3] = {.vol_range = {145, 155}}

#define RELAY_GPIO_MAP_DEF {PB12, PB13, PB14, PB15}             // 继电器 GPIO 映射
#define LED_W_GPIO_MAP_DEF {PWM_PB0, PWM_PB1, PWM_PB6, PWM_PB7} // LED 白灯 GPIO
#define LED_Y_GPIO_MAP_DEF {PA15, PB3, PB4, PB5}                // LED 黄灯 GPIO

#endif

typedef struct {
    // uint8_t relay_table[RELAY_NUMBER];
    // uint8_t bk_table[KEY_NUMBER];
    // uint16_t delay_table[RELAY_NUMBER];

    // uint16_t bk_table[KEY_NUMBER];
    // uint16_t delay_timer_counter[RELAY_NUMBER];
    // uint16_t delay_state[RELAY_NUMBER];

} plcp_panel_info_t;

typedef struct
{
    pwm_hw_pins led_w_pin[8]; // 按键所控白灯
    gpio_pin_t led_y_pin[8];  // 按键所控黄灯
    gpio_pin_t relay_pin[8];  // 按键所控继电器
} plcp_panel_pin_t;

typedef enum {
    SCENE_e   = 0x01,
    SWITCH_e  = 0x04,
    CURTAIN_e = 0x06,
} key_type_e;

void plcp_panel_pins_init(void);
const plcp_panel_info_t *get_panel_info(void);
const plcp_panel_pin_t *get_panel_pins(void);
#endif