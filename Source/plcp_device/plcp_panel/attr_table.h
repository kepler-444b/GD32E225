
#ifndef _ATTR_PANEL_H_
#define _ATTR_PANEL_H_
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/plcp_device/plcp_panel/panel_adapter.h"

#define relay_powe_up_table_max 6

//  继电器类
void attr_relay_table_save(void);
void attr_relay_table_recover(void);
void attr_relay_table_reset(void);
uint8_t attr_relay_table_set(uint8_t index, uint8_t onoff);
uint8_t attr_relay_table_get(uint8_t index);
void relay_table_init(void);

// LED 类
void attr_led_table_save(void);
void attr_led_table_recover(void);
uint8_t attr_led_table_set(uint8_t index, uint8_t onoff);
uint8_t attr_led_table_get(uint8_t index);

// 可调 背光LED 类
void ad_led_b_table_init(void);
void attr_ad_led_b_table_save();
void attr_ad_led_b_state_table_recover(void);
uint8_t attr_led_b_table_set(uint8_t index, uint8_t onoff);
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
uint8_t attr_kj_mode_table_set(uint8_t index, uint8_t state);
uint8_t attr_kj_mode_table_get(uint8_t index);

// 按键状态
uint8_t attr_key_state_table_set(uint8_t id, uint8_t on_off);
uint8_t attr_key_state_table_get(uint8_t id);
void attr_key_state_table_recover(void);

// 上报状态
uint8_t attr_report_type_set(report *report);
report *attr_report_type_get(void);
const report *app_report_get(void);

// 外部接口
void switch_status_by_bits(const uint8_t *data, uint8_t data_len);
void parse_control_commands(const uint8_t *buf, uint8_t buf_len);

#endif
