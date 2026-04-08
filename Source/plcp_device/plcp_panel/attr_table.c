#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/base/base.h"
#include "../../Source/base/debug.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_info.h"
#include "../../Source/plcp_device/plcp_user_api/plcp_sdk_api.h"
#include "../../Source/timer/timer.h"

// 继电器当前状态表
#define relay_table_default_val 0
static uint8_t relay_table[4];

// LED 当前状态表
#define led_table_default_val 0
static uint8_t led_status_table[8];

// LED 控制模式表
#define led_state_table_default_val 1 // 1:正常开关,2:反转开关
static uint8_t led_mode_table[8];

// 可调背光 LED 控制表
#define ad_led_b_table_default_val 0x64
static uint8_t led_b_table[8];

// 按键状态表
#define button_state_default_val 0
static uint8_t key_status_table[8];

// 按键类型
#define kj_mode_table_default_val 1 // 1:场景模式;4:继电器模式;6:窗帘模式
static uint8_t kj_mode_table[8];

// 继电器上电状态
#define relay_powe_up_table_default_val 0 // 0:上电断开,1:上电闭合,2:记忆
static uint8_t relay_powe_up_table[relay_powe_up_table_max];

static report my_report; // 上报方式

/* ****************************************************************************************************** */
// 保存继电器状态
void attr_relay_table_save(void)
{
    switch_adapter_relay_table_save(relay_table, sizeof(relay_table));
}

// 恢复继电器状态
void attr_relay_table_reset(void)
{
    memset(relay_table, relay_table_default_val, sizeof(relay_table));
    attr_relay_table_save();
}

// 读取继电器状态
void attr_relay_table_recover(void)
{
    if (switch_adapter_relay_table_read(relay_table, 4) == false) {
        attr_relay_table_reset();
    }
}

// 设置继电器状态
uint8_t attr_relay_table_set(uint8_t index, uint8_t onoff)
{
    if (index >= 4 || onoff > 1) {
        return 0;
    }
    relay_table[index] = onoff;
    attr_relay_table_save(); // 保存继电器状态

    switch_relay_ctrl(index, onoff); // 控制继电器

    if (attr_kj_mode_table_get(index) == SCENE_e) { // 是场景按键

        led_ctrl_with_state(index, attr_relay_table_get(index));
    }
    return 1;
}

// 获取继电器状态
uint8_t attr_relay_table_get(uint8_t index)
{
    if (index >= 4) {
        return 0xff;
    }
    return relay_table[index];
}

void relay_table_init(void)
{
    for (uint8_t i = 0; i < 4; i++) {
        attr_relay_table_set(i, relay_table[i]);
    }
}

/* ****************************************************************************************************** */
// 保存LED状态
void attr_led_table_save(void)
{
    switch_adapter_led_table_save(led_status_table, KEY_NUMBER);
}

// 恢复LED状态
void attr_led_table_reset(void)
{
    memset(led_status_table, led_table_default_val, sizeof(led_status_table));
    attr_led_table_save();
}

// 读取LED状态
void attr_led_table_recover(void)
{
    if (switch_adapter_led_table_read(led_status_table, KEY_NUMBER) == false) {
        attr_led_table_reset();
        APP_PRINTF("attr_led_table_recover error!\n");
    }
    memcpy(&key_status_table, &led_status_table, sizeof(led_status_table));
}

// 设置LED状态
uint8_t attr_led_table_set(uint8_t index, uint8_t onoff)
{
    if (index >= KEY_NUMBER || onoff > 1) {
        return 0;
    }
    led_status_table[index] = onoff;
    attr_led_table_save(); // 保存led状态
    switch_led_ctrl(index, onoff);
    return 1;
}

// 获取LED状态
uint8_t attr_led_table_get(uint8_t index)
{
    return led_status_table[index];
}

/* ****************************************************************************************************** */
// LED 控制模式

void attr_led_state_table_save(void)
{
    switch_adapter_led_state_table_save(led_mode_table, sizeof(led_mode_table));
}

void attr_led_state_table_reset(void)
{
    memset(led_mode_table, led_state_table_default_val, sizeof(led_mode_table));
    attr_led_state_table_save();
}

void attr_led_state_table_recover(void)
{
    if (switch_adapter_led_state_table_read(led_mode_table, KEY_NUMBER) == false) {
        attr_led_state_table_reset();
    }
    APP_PRINTF_BUF("led_mode_table", led_mode_table, KEY_NUMBER);
    APP_PRINTF("\n");
}

// 根据LED的状态表更新LED的状态
void led_ctrl_with_state(uint8_t id, uint8_t onoff)
{
    // APP_PRINTF("led_mode_table[%d]:%d\n", id, led_mode_table[id]);
    if (led_mode_table[id] == 1) { //  正常开关

        attr_led_table_set(id, onoff);
        attr_led_b_table_set(id, onoff ? 0 : 100); // 可调背光灯与指示灯互斥
    } else if (led_mode_table[id] == 2) {          // 反转开关
        attr_led_table_set(id, !onoff);
        attr_led_b_table_set(id, onoff ? 100 : 0); // 可调背光灯与指示灯互斥
    }
}

uint8_t attr_led_state_table_set(uint8_t index, uint8_t state)
{
    if (index >= KEY_NUMBER) {
        return 0;
    }

    led_mode_table[index] = state;
    attr_led_state_table_save();
    if (led_mode_table[index] == 0) {
        attr_led_table_set(index, 0);
    } else if (led_mode_table[index] == 1) {
        attr_led_table_set(index, state);
    } else if (led_mode_table[index] == 2) {
        attr_led_table_set(index, !state);
    }

    return 1;
}

uint8_t attr_led_state_table_get(uint8_t index)
{
    if (index >= KEY_NUMBER) {
        return 0xff;
    }
    return led_mode_table[index];
}

/* ****************************************************************************************************** */
// 可调背光LED控制表

void attr_ad_led_b_table_save(void)
{
    switch_adapter_ad_led_b_table_save(led_b_table, KEY_NUMBER);
}

void attr_ad_led_b_table_reset(void)
{
    memset(led_b_table, ad_led_b_table_default_val, sizeof(led_b_table));
    attr_ad_led_b_table_save();
}

void attr_ad_led_b_state_table_recover(void)
{
    if (switch_adapter_ad_led_b_table_read(led_b_table, KEY_NUMBER) == false) {
        attr_ad_led_b_table_reset();
    }
    APP_PRINTF_BUF("led_b_table", led_b_table, KEY_NUMBER);
}

uint8_t attr_led_b_table_set(uint8_t index, uint8_t onoff)
{
    if (index >= KEY_NUMBER) {
        return 0;
    }
    led_b_table[index] = onoff;
    attr_ad_led_b_table_save();
    switch_led_b_ctrl(index, onoff, 1);
    return 1;
}

uint8_t attr_ad_led_b_table_get(uint8_t index)
{
    return led_b_table[index];
}

void ad_led_b_table_init(void)
{
    uint8_t duration_ms = 10;
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        // switch_led_b_ctrl(i, led_b_table[i], duration_ms);
        led_b_table[i] = 0x64;
        switch_led_b_ctrl(i, 100, duration_ms); // 亚朵不需要掉点记忆,暂使用硬编码
    }
}

/* ****************************************************************************************************** */
// 继电器上电状态

void attr_relay_power_up_table_save(void)
{
    switch_adapter_relay_powe_up_table_save(relay_powe_up_table, relay_powe_up_table_max);
}

void attr_relay_power_up_table_reset(void)
{
    memset(relay_powe_up_table, relay_powe_up_table_default_val, sizeof(relay_powe_up_table));
    attr_relay_power_up_table_save();
}

void attr_relay_power_up_table_recover(void)
{
    if (switch_adapter_relay_powe_up_table_read(relay_powe_up_table, relay_powe_up_table_max) == false) {
        attr_relay_power_up_table_reset();
    }
    APP_PRINTF_BUF("relay_powe_up_table", relay_powe_up_table, relay_powe_up_table_max);
}

uint8_t attr_relay_power_up_table_set(uint8_t index, uint8_t state)
{
    if (index >= relay_powe_up_table_max) {
        return 0;
    }

    relay_powe_up_table[index] = state;
    attr_relay_power_up_table_save();
    return 1;
}

uint8_t attr_relay_power_up_table_get(uint8_t index)
{
    if (index >= relay_powe_up_table_max) {
        return 0xff;
    }

    return relay_powe_up_table[index];
}

// 执行继电器的上电状态
void relay_powe_up_table_init(void)
{
    for (uint8_t i = 0; i < relay_powe_up_table_max; i++) {
        if (relay_powe_up_table[i] == 0) { // 关闭
            attr_relay_table_set(i, 0);
        } else if (relay_powe_up_table[i] == 1) { // 打开
            attr_relay_table_set(i, 1);
        } else if (relay_powe_up_table[i] == 2) { // 恢复
            switch_relay_ctrl(i, attr_relay_table_get(i));
            led_ctrl_with_state(i, attr_relay_table_get(i));
        }
    }
}

/* ****************************************************************************************************** */
// 保存控件类型
void attr_kj_mode_table_save(void)
{
    switch_adapter_kj_mode_table_save(kj_mode_table, sizeof(kj_mode_table));
}

// 恢复控件类型
void attr_kj_mode_table_reset(void)
{
    memset(kj_mode_table, kj_mode_table_default_val, sizeof(kj_mode_table));

    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        if (kj_mode_table[i] == 0xFF) {
            if (i < KEY_NUMBER) {
                kj_mode_table[i] = SCENE_e; // 默认设置为"场景按键"
            }
        }
    }
    attr_kj_mode_table_save();
}

// 读取控件类型
void attr_kj_mode_table_recover(void)
{
    if (switch_adapter_kj_mode_table_read(kj_mode_table, sizeof(kj_mode_table)) == false) { // 读取按钮模式
        attr_kj_mode_table_reset();
        APP_ERROR("attr_kj_mode_table_reset");
    } else {
        APP_PRINTF_BUF("kj_mode_table", kj_mode_table, sizeof(kj_mode_table));
        APP_PRINTF("\n");
    }
}

// 设置控件类型
uint8_t attr_kj_mode_table_set(uint8_t index, uint8_t state)
{
    if (index >= KEY_NUMBER) {
        return 0;
    }
    kj_mode_table[index] = state;
    attr_kj_mode_table_save();
    return 1;
}

// 获取控件类型
uint8_t attr_kj_mode_table_get(uint8_t index)
{
    if (index >= KEY_NUMBER) {
        return 0xff;
    }
    return kj_mode_table[index];
}

/* ****************************************************************************************************** */
// 按键状态
uint8_t attr_key_state_table_get(uint8_t id)
{
    return key_status_table[id];
}

// 设置按键状态
uint8_t attr_key_state_table_set(uint8_t index, uint8_t on_off)
{
    if (index >= KEY_NUMBER) {
        return 0;
    }
    on_off = !!on_off;
    key_status_table[index] = on_off; // 设置按键状态

    switch_led_ctrl(index, on_off);                // 控制 LED
    switch_led_b_ctrl(index, on_off ? 0 : 100, 1); // 控制背光灯,原逻辑:按键打开背光 0,关闭 100
    return 1;
}

// 恢复按键状态
void attr_key_state_table_recover(void)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        attr_key_state_table_set(i, 0);
    }
}

/* ****************************************************************************************************** */
// 设置上报模式和上报顺取
uint8_t attr_report_type_set(report *report)
{
    return report_type_save(&my_report);
}

report *attr_report_type_get(void)
{
    report_type_read(&my_report);
    return &my_report;
}

const report *app_report_get(void)
{
    return &my_report;
}

// 执行 state
void switch_status_by_bits(const uint8_t *data, uint8_t data_len)
{
    if (!data || data_len < 3) {
        return;
    }
    uint8_t cmd_type = data[0]; // 控制 type
    uint16_t ctrl_bits;         // 控制字

    ctrl_bits = ((uint16_t)data[1] << 8) | data[2];
    uint8_t idx = 0;

    for (uint8_t i = 0; i < 16 && (3 + idx) < data_len; i++) {
        if (!(ctrl_bits & (1 << (15 - i)))) { // 只处理需要控制的位
            continue;
        }

        uint8_t status = data[3 + idx++];

        switch (cmd_type) {
        case BUTTON_TYPE:
            attr_key_state_table_set(i, status);
            break;
        case REALY_TYPE:
            attr_relay_table_set(i, status);
            break;
        case BLCK_TYPE:
            if (i < 8) { // 前8个bit是背光灯
                attr_led_b_table_set(i, status ? 100 : 0);
            } else { // 后8个bit是指示灯
                attr_led_table_set(i - 8, !!status);
            }
            break;
        case ADJUST_BLCK_TYPE:
            attr_led_b_table_set(i, status);
            break;
        default:
            break;
        }
    }
}

// 默认场景执行
void parse_control_commands(const uint8_t *buf, uint8_t buf_len)
{
    uint8_t buf_length = buf[0]; // 第一个字节就是数据包长度
    uint8_t pos = 1;             // 从第二个字节开始解析命令

    while (pos < buf_length + 1) { // 注意：pos要小于 buf_length + 1
        uint8_t start_pos = pos;

        uint16_t control_mask = (buf[pos + 1] << 8) | buf[pos];
        pos += 2;

        // 计算被控个数(统计1的个数)
        uint8_t control_count = COUNT_ONES_16(control_mask);

        for (uint8_t i = 0; i < control_count && pos < buf_length + 1; i++) {
            pos++;
        }
        uint8_t cmd_length = pos - start_pos; // 计算当前命令的长度
        switch_status_by_bits(&buf[start_pos], cmd_length);
    }
}
