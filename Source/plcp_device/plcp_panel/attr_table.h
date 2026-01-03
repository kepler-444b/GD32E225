
#ifndef _ATTR_PANEL_H_
#define _ATTR_PANEL_H_
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"

#define relay_table_max         4
#define led_relay_table_max     6
#define delay_table_max         6

#define relay_powe_up_table_max 6

//  继电器类
void attr_relay_table_save(void);
void attr_relay_table_recover(void);
void attr_relay_table_reset(void);
uint8_t attr_relay_table_set(uint8_t index, uint8_t onoff);
uint8_t attr_relay_table_get(uint8_t index);
void relay_table_init(void);

// 延时类
void attr_delay_table_save(void);
void attr_delay_table_recover(void);
uint8_t attr_delay_table_set(uint8_t index, uint16_t time);
uint16_t attr_delay_table_get(uint8_t index);
void delay_table_init(void);
uint8_t delay_timer_active(uint8_t id, uint8_t state);

// 闪烁类
void attr_bk_table_save(void);
void attr_bk_table_recover(void);
uint8_t attr_bk_table_set(uint8_t index, uint8_t onoff);
uint8_t attr_bk_table_get(uint8_t index);
void bk_table_init(void);

// LED 类
void attr_led_table_save(void);
void attr_led_table_recover(void);
uint8_t attr_led_table_set(uint8_t index, uint8_t onoff);
uint8_t attr_led_table_get(uint8_t index);
void led_table_init(void);

// 背光LED类
void led_b_table_init(void);
void attr_led_b_state_table_recover(void);
void attr_led_b_table_reset();
uint8_t attr_led_b_table_set(uint8_t index, uint8_t onoff);
uint8_t attr_led_b_table_get(uint8_t index);

// LED 继电器关联表
void attr_led_enable_table_recover(void);
void attr_led_enable_table_reset(void);
uint8_t attr_led_relay_table_set(uint8_t index, uint8_t state);
uint8_t attr_led_relay_table_get(uint8_t index);
// 可调 背光LED 类
void ad_led_b_table_init(void);
void attr_ad_led_b_state_table_recover(void);
uint8_t attr_ad_led_b_table_set(uint8_t index, uint8_t onoff);
uint8_t attr_ad_led_b_table_get(uint8_t index);

// LED 控制模式
void attr_led_state_table_save(void);
void attr_led_state_table_recover(void);
uint8_t attr_led_state_table_set(uint8_t index, uint8_t state);
uint8_t attr_led_state_table_get(uint8_t index);
void led_ctrl_with_state(uint8_t id, uint8_t onoff);

// 继电器上电状态
void attr_relay_power_up_table_save(void);
void attr_relay_power_up_table_recover(void);
uint8_t attr_relay_power_up_table_set(uint8_t index, uint8_t state);
uint8_t attr_relay_power_up_table_get(uint8_t index);
void relay_powe_up_table_init(void);

// 按键类型
void attr_kj_mode_table_save(void);
void attr_kj_mode_table_reset(void);
void attr_kj_mode_table_recover(void);
uint8_t attr_button_mode_table_set(uint8_t index, uint8_t state);
uint8_t attr_kj_mode_table_get(uint8_t index);

// 按键状态
uint8_t attr_key_state_table_set(uint8_t id, uint8_t on_off);
uint8_t attr_key_state_table_get(uint8_t id);

void switch_status_by_bits(const uint8_t *data, uint8_t data_len);

void parse_control_commands(const uint8_t *buf, uint8_t buf_len);

#endif
