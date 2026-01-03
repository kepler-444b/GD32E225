#ifndef _PANEL_ADAPTER_H_
#define _PANEL_ADAPTER_H_

#include <stdint.h>
#include <stdbool.h>

#define FLASH_PANEL_LED_TABLE        (0x0801B800U) // page 110
#define FLASH_PANEL_LED_B_TABLE      (0x0801BC00U) // page 111
#define FLASH_PANEL_LED_STATE_TABLE  (0x0801C000U) // page 112
#define FLASH_PANEL_RELAY_TABLE      (0x0801C400U) // page 113
#define FLASH_PANEL_POWE_UP_TABLE    (0x0801C800U) // page 114
#define FLASH_PANEL_KEY_TYPE_TABLE   (0x0801CC00U) // page 115
#define FLASH_PANEL_AD_LED_B_TABLE   (0x0801D000U) // page 116
#define FLASH_PANEL_LED_ENABLE_TABLE (0x0801D800U) // page 117

uint16_t switch_adapter_bk_table_read(uint8_t *bk_table);

// 背光LED 状态读写
bool switch_adapter_led_b_table_read(uint8_t *led_b_table, uint8_t len);
bool switch_adapter_led_b_table_save(uint8_t *led_b_table, uint8_t len);

// 可调背光LED 状态读写
bool switch_adapter_ad_led_b_table_read(uint8_t *led_b_table, uint8_t len);
bool switch_adapter_ad_led_b_table_save(uint8_t *led_b_table, uint8_t len);

// LED 状态读写
bool switch_adapter_led_table_read(uint8_t *led_table, uint8_t len);
bool switch_adapter_led_table_save(uint8_t *led_table, uint8_t len);

// LED RELAY 关联表
bool switch_adapter_led_enable_table_read(uint8_t *led_enable_table, uint8_t len);
bool switch_adapter_led_relay_table_save(uint8_t *led_enable_table, uint8_t len);
// 继电器 状态读写
bool switch_adapter_relay_table_read(uint8_t *relay_table, uint8_t len);
bool switch_adapter_relay_table_save(uint8_t *relay_table, uint8_t len);

// 上电状态读写
bool switch_adapter_relay_powe_up_table_read(uint8_t *relay_powe_up_table, uint8_t len);
bool switch_adapter_relay_powe_up_table_save(uint8_t *relay_powe_up_table, uint8_t len);

// LED 控制模式读写
bool switch_adapter_led_state_table_read(uint8_t *led_state_table, uint8_t len);
bool switch_adapter_led_state_table_save(uint8_t *led_state_table, uint8_t len);

// 按键类型读写
bool switch_adapter_kj_mode_table_read(uint8_t *kj_mode_table, uint8_t len);
bool switch_adapter_kj_mode_table_save(uint8_t *kj_mode_table, uint8_t len);

bool switch_adapter_delay_table_read(uint8_t *delay_table);

void switch_adapter_relay_ctrl(uint8_t id, uint8_t connect);
void switch_adapter_led_bk_ctrl(uint8_t id, uint8_t onoff);
void switch_adapter_led_ctrl(uint8_t id, uint8_t onoff);
void switch_adapter_led_b_ctrl(uint8_t id, uint8_t onoff);
void switch_adapter_ad_led_b_ctrl(uint8_t id, uint8_t lum, uint8_t duration_ms);

#endif