#include "panel_adapter.h"
#include "../../Source/base/base.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/flash/flash.h"
#include "../../Source/gpio/gpio.h"
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
            if (i < 4) {
                kj_mode_table[i] = SWITCH_e;
            } else {
                kj_mode_table[i] = SCENE_e;
            }
        }
    }
    return true;
}
bool switch_adapter_kj_mode_table_save(uint8_t *kj_mode_table, uint8_t len)
{
    APP_PRINTF("switch_adapter_kj_mode_table_save\n");
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_PANEL_KEY_TYPE_TABLE, (uint32_t *)kj_mode_table, len, true);
    if (ret != FMC_READY) {
        return false;
    }
    return true;
}

// 控制继电器的状态
void switch_adapter_relay_ctrl(uint8_t id, uint8_t connect)
{
    APP_SET_GPIO(get_panel_pins()->relay_pin[id], connect);
}

// 控制特定LED的状态
void switch_adapter_led_ctrl(uint8_t id, uint8_t onoff)
{
#if defined PANEL_3KEY
    if (id == 0 || id == 1) {
        APP_SET_GPIO(get_panel_pins()->led_y_pin[0], onoff);
        APP_SET_GPIO(get_panel_pins()->led_y_pin[1], onoff);
    } else {
        APP_SET_GPIO(get_panel_pins()->led_y_pin[id], onoff);
    }
#elif defined PANEL_1KEY
    if (id == 0 || id == 1 || id == 2 || id == 3 || id == 4 || id == 5) {
        APP_SET_GPIO(get_panel_pins()->led_y_pin[0], onoff);
        APP_SET_GPIO(get_panel_pins()->led_y_pin[1], onoff);
        APP_SET_GPIO(get_panel_pins()->led_y_pin[2], onoff);
        APP_SET_GPIO(get_panel_pins()->led_y_pin[3], onoff);
        APP_SET_GPIO(get_panel_pins()->led_y_pin[4], onoff);
        APP_SET_GPIO(get_panel_pins()->led_y_pin[5], onoff);
    } else {
        APP_SET_GPIO(get_panel_pins()->led_y_pin[id], onoff);
    }
#elif defined PANEL_2KEY
    if (id == 0 || id == 3) {
        APP_SET_GPIO(get_panel_pins()->led_y_pin[0], onoff);
        APP_SET_GPIO(get_panel_pins()->led_y_pin[3], onoff);
    } else if (id == 1 || id == 2) {
        APP_SET_GPIO(get_panel_pins()->led_y_pin[1], onoff);
        APP_SET_GPIO(get_panel_pins()->led_y_pin[2], onoff);
    } else {
        APP_SET_GPIO(get_panel_pins()->led_y_pin[id], onoff);
    }
#else
    APP_SET_GPIO(get_panel_pins()->led_y_pin[id], onoff);
#endif
}

// 控制特定背光LED的状态
void switch_adapter_led_b_ctrl(uint8_t id, uint8_t onoff)
{
    uint16_t duty = onoff > 0 ? 1000 : 0;

    app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[id], duty, 0);
}

// 控制特定可调背光LED的状态
void switch_adapter_ad_led_b_ctrl(uint8_t id, uint8_t lum, uint8_t duration_ms)
{
#if defined PANEL_3KEY
    if (id == 0 || id == 1) {
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[0], lum * 10, duration_ms * 100);
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[1], lum * 10, duration_ms * 100);
    } else {
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[id], lum * 10, duration_ms * 100);
    }
#elif defined PANEL_1KEY
    if (id == 0 || id == 1 || id == 2 || id == 3 || id == 4 || id == 5) {
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[0], lum * 10, duration_ms * 100);
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[1], lum * 10, duration_ms * 100);
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[2], lum * 10, duration_ms * 100);
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[3], lum * 10, duration_ms * 100);
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[4], lum * 10, duration_ms * 100);
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[5], lum * 10, duration_ms * 100);
    } else {
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[id], lum * 10, duration_ms * 100);
    }
#elif defined PANEL_2KEY
    if (id == 0 || id == 3) {
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[0], lum * 10, duration_ms * 100);
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[3], lum * 10, duration_ms * 100);
    } else if (id == 1 || id == 2) {
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[1], lum * 10, duration_ms * 100);
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[2], lum * 10, duration_ms * 100);
    } else {
        app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[id], lum * 10, duration_ms * 100);
    }
#else
    app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[id], lum * 10, duration_ms * 100);
#endif
}
