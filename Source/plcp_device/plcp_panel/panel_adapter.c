#include "panel_adapter.h"
#include "../../Source/base/base.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/flash/flash.h"
#include "../../Source/gpio/gpio.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_info.h"
#include "../../Source/pwm/pwm_hw.h"
#include <string.h>

/* ****************************************************************************************************** */
// 继电器
bool switch_adapter_relay_table_read(uint8_t *relay_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_PANEL_RELAY_TABLE, (uint32_t *)relay_table, len);
    if (ret != FMC_READY) {
        return false;
    }
    for (uint8_t i = 0; i < len; i++) {
        if (relay_table[i] == 0xFF) {
            relay_table[i] = 0;
        }
    }
    return true;
}
bool switch_adapter_relay_table_save(uint8_t *relay_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_PANEL_RELAY_TABLE, (uint32_t *)relay_table, len, true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

/* ****************************************************************************************************** */
// 延时参数
bool switch_adapter_delay_table_read(uint8_t *delay_table)
{
    uint16_t temp[8] = {0, 0, 0, 0, 1, 1, 1, 1};
    memcpy(delay_table, temp, sizeof(temp));
    return true;
}

uint16_t switch_adapter_bk_table_read(uint8_t *bk_table)
{
    return 0;
}

/* ****************************************************************************************************** */
// LED
bool switch_adapter_led_table_read(uint8_t *led_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_PANEL_LED_TABLE, (uint32_t *)led_table, len);
    if (ret != FMC_READY) {
        return false;
    }
    for (uint8_t i = 0; i < len; i++) {
        if (led_table[i] == 0xFF) {
            led_table[i] = 0x00;
        }
    }
    return true;
}

bool switch_adapter_led_table_save(uint8_t *led_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_PANEL_LED_TABLE, (uint32_t *)led_table, len, true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

/* ****************************************************************************************************** */
// LED RELAY 关联
bool switch_adapter_led_enable_table_read(uint8_t *led_enable_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_PANEL_LED_ENABLE_TABLE, (uint32_t *)led_enable_table, len);
    if (ret != FMC_READY) {
        return false;
    }
    for (uint8_t i = 0; i < len; i++) {
        if (led_enable_table[i] == 0xFF) {
            led_enable_table[i] = 0;
        }
    }
    return true;
}
bool switch_adapter_led_relay_table_save(uint8_t *led_enable_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_PANEL_LED_ENABLE_TABLE, (uint32_t *)led_enable_table, len, true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

/* ****************************************************************************************************** */
// LED控制模式
bool switch_adapter_led_state_table_read(uint8_t *led_state_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_PANEL_LED_STATE_TABLE, (uint32_t *)led_state_table, len);
    if (ret != FMC_READY) {
        return false;
    }
    for (uint8_t i = 0; i < len; i++) {
        if (led_state_table[i] == 0xFF) {
            led_state_table[i] = 0x01;
        }
    }
    return true;
}

bool switch_adapter_led_state_table_save(uint8_t *led_state_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_PANEL_LED_STATE_TABLE, (uint32_t *)led_state_table, len, true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

/* ****************************************************************************************************** */
// 背光LED
bool switch_adapter_led_b_table_read(uint8_t *led_b_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_PANEL_LED_B_TABLE, (uint32_t *)led_b_table, len);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

bool switch_adapter_led_b_table_save(uint8_t *led_b_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_PANEL_LED_B_TABLE, (uint32_t *)led_b_table, len, true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

/* ****************************************************************************************************** */
// 可调背光LED
bool switch_adapter_ad_led_b_table_read(uint8_t *ad_led_b_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_PANEL_AD_LED_B_TABLE, (uint32_t *)ad_led_b_table, len);
    if (ret != FMC_READY) {
        return false;
    }
    for (uint8_t i = 0; i < len; i++) {
        if (ad_led_b_table[i] == 0xFF) {
            ad_led_b_table[i] = 0x64;
        }
    }
    return true;
}

bool switch_adapter_ad_led_b_table_save(uint8_t *ad_led_b_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_PANEL_AD_LED_B_TABLE, (uint32_t *)ad_led_b_table, len, true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

/* ****************************************************************************************************** */
// 上电状态
bool switch_adapter_relay_powe_up_table_read(uint8_t *relay_powe_up_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_PANEL_POWE_UP_TABLE, (uint32_t *)relay_powe_up_table, len);
    if (ret != FMC_READY) {
        return false;
    }
    for (uint8_t i = 0; i < len; i++) {
        if (relay_powe_up_table[i] == 0xFF) {
            relay_powe_up_table[i] = 0;
        }
    }
    return true;
}

bool switch_adapter_relay_powe_up_table_save(uint8_t *relay_powe_up_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_PANEL_POWE_UP_TABLE, (uint32_t *)relay_powe_up_table, len, true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

/* ****************************************************************************************************** */
// 按键类型
bool switch_adapter_kj_mode_table_read(uint8_t *kj_mode_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_PANEL_KEY_TYPE_TABLE, (uint32_t *)kj_mode_table, len);
    if (ret != FMC_READY) {
        APP_PRINTF("switch_adapter_kj_mode_table_read error\n");
        return false;
    }
    for (uint8_t i = 0; i < len; i++) {
        if (kj_mode_table[i] == 0xFF) {
            if (i < KEY_NUMBER) {
                kj_mode_table[i] = SCENE_e; // 默认设置为"场景按键"
            }
        }
    }
    return true;
}
bool switch_adapter_kj_mode_table_save(uint8_t *kj_mode_table, uint8_t len)
{
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_PANEL_KEY_TYPE_TABLE, (uint32_t *)kj_mode_table, len, true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

// 写入key个数
bool key_num_save(uint8_t key_num)
{
    fmc_state_enum ret;
    uint32_t key_number = key_num;
    ret = app_flash_program(FLASH_PANEL_KYE_NUM, (uint32_t *)&key_number, sizeof(key_number), true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

// 读取key个数
bool key_num_read(uint8_t *key_num)
{
    fmc_state_enum ret;
    uint32_t key_number = 0;

    ret = app_flash_read(FLASH_PANEL_KYE_NUM, (uint32_t *)&key_number, sizeof(key_number));
    if (ret != FMC_READY) {
        APP_PRINTF("switch_adapter_kj_mode_table_read error\n");
        return false;
    }
    if (key_number == 0xFFFFFFFF) {

        key_number = 0x06; // 使用默认值
    }
    *key_num = (uint8_t)key_number;
    return true;
}

// 保存上报方式
bool report_type_save(report *report)
{
    APP_PRINTF("report_type_save[rpt:%d] [order:%d]\n", report->rpt, report->order);
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_PANEL_REPORT_TYPE, (uint32_t *)report, sizeof(report), true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

bool report_type_read(report *report)
{
    fmc_state_enum ret;

    ret = app_flash_read(FLASH_PANEL_REPORT_TYPE, (uint32_t *)report, sizeof(report));
    if (ret != FMC_READY) {
        APP_PRINTF("switch_adapter_kj_mode_table_read error\n");
        return false;
    }
    if (report->rpt == 0xFF)
        report->rpt = 0x01;
    if (report->order == 0xFF)
        report->order = 0x00;

    return true;
}

// 控制继电器的状态
void switch_relay_ctrl(uint8_t id, uint8_t connect)
{
    APP_SET_GPIO(get_panel_pins()->relay_pin[id], connect);
}

// 控制特定LED的状态
void switch_led_ctrl(uint8_t id, uint8_t onoff)
{
    const plcp_panel_pin_t *p_pins = get_panel_pins();

    APP_SET_GPIO(p_pins->led_y_pin[id], onoff);
}

// 控制特定可调背光LED的状态
void switch_led_b_ctrl(uint8_t id, uint8_t lum, uint8_t duration_ms)
{
    const plcp_panel_pin_t *p_pins = get_panel_pins();

    app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[id], lum * 10, duration_ms * 100);
}
