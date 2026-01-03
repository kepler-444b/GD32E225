#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/base/base.h"
#include "../../Source/base/debug.h"
#include "../../Source/device/device_manager.h"
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/plcp_device/plcp_panel/panel_adapter.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_info.h"
#include "../../Source/plcp_device/plcp_user_api/plcp_sdk_api.h"
#include "../../Source/timer/timer.h"

// 继电器当前状态表
#define relay_table_default_val 0

static uint8_t relay_table[relay_table_max];

// LED 当前状态表
#define led_table_default_val 0

static uint8_t led_table[KEY_NUMBER];

// LED 关联表
#define led_relay_table_default_val 0 // 0:不与继电器关联,1:与继电器关联

static uint8_t led_enable_table[led_relay_table_max];

// LED 控制模式表
#define led_state_table_default_val 1 // 1:正常开关,2:反转开关
static uint8_t led_state_table[KEY_NUMBER];

// 延时控制表
#define delay_table_default_val 0
static uint16_t delay_table[delay_table_max];

// LED 闪烁控制表
static uint8_t bk_table[KEY_NUMBER];
static uint8_t bk_led_blink_timer;

// 背光 LED 控制表
#define led_b_table_default_val 0x64
static uint8_t led_b_table[KEY_NUMBER];
// 可调背光 LED 控制表
#define ad_led_b_table_default_val 0x64
static uint8_t ad_led_b_table[KEY_NUMBER];
// 按键状态表
#define button_state_default_val 0
static uint8_t button_state_table[KEY_NUMBER];
// 按键类型
#define kj_mode_table_default_val 4 // 1:场景模式;4:继电器模式;6:窗帘模式
static uint8_t kj_mode_table[KEY_NUMBER];

// 继电器上电状态
#define relay_powe_up_table_default_val 0 // 0:上电断开,1:上电闭合,2:记忆
static uint8_t relay_powe_up_table[relay_powe_up_table_max];

/* ****************************************************************************************************** */
// 继电器状态控制
void attr_relay_table_save(void)
{
    switch_adapter_relay_table_save(relay_table, sizeof(relay_table));
}

void attr_relay_table_reset(void)
{
    memset(relay_table, relay_table_default_val, sizeof(relay_table));
    attr_relay_table_save();
}

void attr_relay_table_recover(void)
{
    if (switch_adapter_relay_table_read(relay_table, relay_table_max) == false) {
        attr_relay_table_reset();
    }
    APP_PRINTF_BUF("relay_table", relay_table, relay_table_max);
}

uint8_t attr_relay_table_set(uint8_t index, uint8_t onoff)
{
    if (index >= relay_table_max || onoff > 1) {
        return 0;
    }
    relay_table[index] = onoff;
    attr_relay_table_save(); // 保存继电器状态

    switch_adapter_relay_ctrl(index, onoff); // 控制继电器

    if (attr_kj_mode_table_get(index) == SCENE_e) { // 是场景按键

        if (led_enable_table[index] == 1) { // 指示灯与继电器关联
            led_ctrl_with_state(index, attr_relay_table_get(index));
        }
    } else { // 继电器按键(关联)
        led_ctrl_with_state(index, attr_relay_table_get(index));
    }
    return 1;
}

uint8_t attr_relay_table_get(uint8_t index)
{
    if (index >= relay_table_max) {
        return 0xff;
    }
    return relay_table[index];
}

void relay_table_init(void)
{
    for (uint8_t i = 0; i < relay_table_max; i++) {
        attr_relay_table_set(i, relay_table[i]);
    }
}

/* ****************************************************************************************************** */
// 延时控制表
void attr_delay_table_save(void)
{
    // switch_adapter_delay_table_save((uint8_t*)delay_table, sizeof(delay_table));
}

void attr_delay_table_reset(void)
{
    memset(delay_table, delay_table_default_val, sizeof(delay_table));
    attr_delay_table_save();
}

void attr_delay_table_recover(void)
{
    if (switch_adapter_delay_table_read((uint8_t *)delay_table) == false) {
        attr_delay_table_reset();
    }
}

uint8_t attr_delay_table_set(uint8_t index, uint16_t time)
{
    if (index >= delay_table_max || time == 0xffff) {
        return 0;
    }
    delay_table[index] = time;
    attr_delay_table_save();
    return 1;
}

uint16_t attr_delay_table_get(uint8_t index)
{
    if (index >= delay_table_max) {
        return 0xffff;
    }
    return delay_table[index];
}

static uint16_t delay_timer_counter[delay_table_max];
static uint16_t delay_state[delay_table_max];
static uint8_t delay_timer = 0xff;

static void delay_timer_handler(void *arg)
{
    for (uint8_t i = 0; i < delay_table_max; i++) {
        if (delay_timer_counter[i] > 0 && delay_timer_counter[i] != 0xffff) {
            delay_timer_counter[i]--;
            if (delay_timer_counter[i] == 0) {
                delay_timer_counter[i] = 0xffff;
                attr_relay_table_set(i, delay_state[i]);
            }
        }
    }
}

void delay_table_init(void)
{
    uint8_t i;
    // 初始化所有计数器
    for (i = 0; i < delay_table_max; i++) {
        delay_timer_counter[i] = 0xffff; // 表示定时器未激活
        delay_state[i] = 0;              // 清除所有状态
    }
    app_timer_start(1000, delay_timer_handler, true, NULL, "delay_timer");
}

// 带延时控制的继电器状态管理系统
uint8_t delay_timer_active(uint8_t id, uint8_t state)
{
    if (id >= delay_table_max || state > 1) {
        return 0;
    }
    if (delay_table[id] == 0) {
        attr_relay_table_set(id, state);
        return 1;
    }
    delay_timer_counter[id] = delay_table[id];

    delay_state[id] = state;

    return 1;
}

/* ****************************************************************************************************** */
// LED闪烁控制

// 定义定时器回调函数，用于控制LED闪烁
static void exe_led_bilink(void *arg)
{
    static uint16_t bk_blink_count = 0;        // 闪烁次数计数器
    static uint8_t bk_led_state = 1;           // 当前LED的状态(1为开启，0为关闭)
    static uint8_t bk_led_stop_blink_flag = 0; // 标志位，控制是否停止闪烁

    if (0 != plcp_sdk_api_get_did() || bk_blink_count >= 900) {
        if (bk_led_stop_blink_flag == 0) {
            bk_led_stop_blink_flag = 1;
            bk_led_state = attr_bk_table_get(0);            // 获取配置中的LED状态
            switch_adapter_led_bk_ctrl(0xff, bk_led_state); // 更新LED的状态(所有相关LED)
        }
        return;
    }

    // 更新LED的状态(所有相关LED)
    bk_led_stop_blink_flag = 0; // 清除停止标志
    bk_blink_count++;           // 增加闪烁计数器

    bk_led_state = !bk_led_state;                   // 反转当前LED状态
    switch_adapter_led_bk_ctrl(0xff, bk_led_state); // 更新所有相关LED的状态
}

void bk_table_init(void)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        switch_adapter_led_bk_ctrl(i, bk_table[i]);
    }
    // 启动定时器，每隔1000ms执行一次exe_led_bilink函数
    app_timer_start(1000, exe_led_bilink, true, NULL, "exe_led_bilink");
}

void attr_bk_table_save(void)
{
    // switch_adapter_bk_table_save(bk_table, sizeof(bk_table));
}

void attr_bk_table_reset(void)
{
    // memset(bk_table, bk_table_default_val, sizeof(bk_table));
    attr_bk_table_save();
}

void attr_bk_table_recover(void)
{
    if (0 == switch_adapter_bk_table_read(bk_table)) {
        attr_bk_table_reset();
    }
}

uint8_t attr_bk_table_set(uint8_t index, uint8_t onoff)
{
    if (index >= KEY_NUMBER || onoff > 1) {
        return 0;
    }

    bk_table[index] = onoff;
    attr_bk_table_save();

    // switch_adapter_led_bk_ctrl(index, onoff);

    return 1;
}

uint8_t attr_bk_table_get(uint8_t index)
{
    if (index >= KEY_NUMBER) {
        return 0xff;
    }

    return bk_table[index];
}

/* ****************************************************************************************************** */
// LED控制表

void attr_led_table_save(void)
{
    switch_adapter_led_table_save(led_table, KEY_NUMBER);
}

void attr_led_table_reset(void)
{
    memset(led_table, led_table_default_val, sizeof(led_table));

    attr_led_table_save();
}

void attr_led_table_recover(void)
{
    if (switch_adapter_led_table_read(led_table, KEY_NUMBER) == false) {
        attr_led_table_reset();
        APP_PRINTF("attr_led_table_recover error!\n");
    }
    memcpy(&button_state_table, &led_table, sizeof(led_table));
    APP_PRINTF_BUF("led_table", led_table, KEY_NUMBER);
}

uint8_t attr_led_table_set(uint8_t index, uint8_t onoff)
{
    if (index >= KEY_NUMBER || onoff > 1) {
        return 0;
    }
    led_table[index] = onoff;
    attr_led_table_save();
    switch_adapter_led_ctrl(index, onoff);

    return 1;
}

uint8_t attr_led_table_get(uint8_t index)
{
    if (index >= KEY_NUMBER) {
        return 0xff;
    }

    return led_table[index];
}

void led_table_init(void)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        switch_adapter_led_ctrl(i, led_table[i]);
    }
}

/* ****************************************************************************************************** */
// LED 使能表

void attr_led_enable_table_save(void)
{
    switch_adapter_led_relay_table_save(led_enable_table, led_relay_table_max);
}

void attr_led_enable_table_reset(void)
{
    memset(led_enable_table, led_relay_table_default_val, sizeof(led_enable_table));
    attr_led_enable_table_save();
}

void attr_led_enable_table_recover(void)
{
    if (switch_adapter_led_enable_table_read(led_enable_table, led_relay_table_max) == false) {
        attr_led_enable_table_reset();
    }
    APP_PRINTF_BUF("led_enable_table", led_enable_table, led_relay_table_max);
}

uint8_t attr_led_relay_table_set(uint8_t index, uint8_t state)
{
    if (index >= led_relay_table_max) {
        return 0;
    }

    led_enable_table[index] = state;
    attr_led_enable_table_save();

    return 1;
}

uint8_t attr_led_relay_table_get(uint8_t index)
{
    if (index >= led_relay_table_max) {
        return 0xff;
    }
    return led_enable_table[index];
}

/* ****************************************************************************************************** */
// LED 控制模式

void attr_led_state_table_save(void)
{
    switch_adapter_led_state_table_save(led_state_table, sizeof(led_state_table));
}

void attr_led_state_table_reset(void)
{
    memset(led_state_table, led_state_table_default_val, sizeof(led_state_table));
    attr_led_state_table_save();
}

void attr_led_state_table_recover(void)
{
    if (switch_adapter_led_state_table_read(led_state_table, KEY_NUMBER) == false) {
        attr_led_state_table_reset();
        APP_PRINTF("attr_led_state_table_recover\n");
    }
    APP_PRINTF_BUF("led_state_table", led_state_table, KEY_NUMBER);
}

// 根据LED的状态表更新LED的状态
void led_ctrl_with_state(uint8_t id, uint8_t onoff)
{
    APP_PRINTF("led_state_table[%d]:%d\n", id, led_state_table[id]);
    if (led_state_table[id] == 1) { //  正常开关

        attr_led_table_set(id, onoff);
        attr_ad_led_b_table_set(id, onoff ? 0 : 100); // 可调背光灯与指示灯互斥
    } else if (led_state_table[id] == 2) {            // 反转开关
        attr_led_table_set(id, !onoff);
        attr_ad_led_b_table_set(id, onoff ? 100 : 0); // 可调背光灯与指示灯互斥
    }
}

uint8_t attr_led_state_table_set(uint8_t index, uint8_t state)
{
    if (index >= KEY_NUMBER) {
        return 0;
    }

    led_state_table[index] = state;
    attr_led_state_table_save();

    if (led_state_table[index] == 0) {
        attr_led_table_set(index, 0);
    } else if (led_state_table[index] == 1) {
        attr_led_table_set(index, attr_relay_table_get(index));
    } else if (led_state_table[index] == 2) {
        attr_led_table_set(index, !attr_relay_table_get(index));
    }

    return 1;
}

uint8_t attr_led_state_table_get(uint8_t index)
{
    if (index >= KEY_NUMBER) {
        return 0xff;
    }
    return led_state_table[index];
}

/* ****************************************************************************************************** */
// 背光LED控制表

void attr_led_b_table_save(void)
{
    switch_adapter_led_b_table_save(led_b_table, KEY_NUMBER);
}

void attr_led_b_table_reset(void)
{
    memset(led_b_table, led_b_table_default_val, sizeof(led_b_table));
    attr_led_b_table_save();
}

void attr_led_b_state_table_recover(void)
{
    if (switch_adapter_led_b_table_read(led_b_table, KEY_NUMBER) == false) {
        attr_led_b_table_reset();
    }
}

uint8_t attr_led_b_table_set(uint8_t index, uint8_t onoff)
{
    APP_PRINTF("attr_led_b_table_set\n");
    if (index >= KEY_NUMBER || onoff > 1) {
        return 0;
    }
    led_b_table[index] = onoff;
    attr_led_b_table_save(); // 保存背光灯状态
    switch_adapter_led_b_ctrl(index, onoff);

    return 1;
}

uint8_t attr_led_b_table_get(uint8_t index)
{
    if (index >= KEY_NUMBER) {
        return 0xff;
    }
    return led_b_table[index];
}

void led_b_table_init(void)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        switch_adapter_led_b_ctrl(i, led_b_table[i]);
    }
}

/* ****************************************************************************************************** */
// 可调背光LED控制表

void attr_ad_led_b_table_save(void)
{
    switch_adapter_ad_led_b_table_save(ad_led_b_table, KEY_NUMBER);
}

void attr_ad_led_b_table_reset(void)
{
    memset(ad_led_b_table, ad_led_b_table_default_val, sizeof(ad_led_b_table));
    attr_ad_led_b_table_save();
}

void attr_ad_led_b_state_table_recover(void)
{
    if (switch_adapter_ad_led_b_table_read(ad_led_b_table, KEY_NUMBER) == false) {
        attr_ad_led_b_table_reset();
    }
    APP_PRINTF_BUF("ad_led_b_table", ad_led_b_table, KEY_NUMBER);
}

uint8_t attr_ad_led_b_table_set(uint8_t index, uint8_t onoff)
{
    if (index >= KEY_NUMBER) {
        return 0;
    }
    ad_led_b_table[index] = onoff;
    attr_ad_led_b_table_save();
    switch_adapter_ad_led_b_ctrl(index, onoff, 1);
    return 1;
}

uint8_t attr_ad_led_b_table_get(uint8_t index)
{
    if (index >= KEY_NUMBER) {
        return 0xff;
    }
    return ad_led_b_table[index];
}

void ad_led_b_table_init(void)
{
    uint8_t duration_ms = 10;
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        // switch_adapter_ad_led_b_ctrl(i, ad_led_b_table[i], duration_ms);
        switch_adapter_ad_led_b_ctrl(i, 100, duration_ms); // 亚朵不需要掉点记忆,暂使用硬编码
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
            switch_adapter_relay_ctrl(i, attr_relay_table_get(i));
            if (attr_kj_mode_table_get(i) == SCENE_e) { // 是场景按键
                if (led_enable_table[i] == 1) {         // 指示灯与继电器关联
                    led_ctrl_with_state(i, attr_relay_table_get(i));
                }
            } else { // 继电器按键
                led_ctrl_with_state(i, attr_relay_table_get(i));
            }
        }
    }
}

/* ****************************************************************************************************** */
// 按键类型
void attr_kj_mode_table_save(void)
{
    switch_adapter_kj_mode_table_save(kj_mode_table, KEY_NUMBER);
}

void attr_kj_mode_table_reset(void)
{
    memset(kj_mode_table, kj_mode_table_default_val, sizeof(kj_mode_table));

    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        if (i < 4) {
            kj_mode_table[i] = 0x04;
        } else {
            kj_mode_table[i] = 0x01;
        }
    }
    attr_kj_mode_table_save();
}

void attr_kj_mode_table_recover(void)
{
    if (switch_adapter_kj_mode_table_read(kj_mode_table, KEY_NUMBER) == false) { // 读取按钮模式
        attr_kj_mode_table_reset();
    }
    APP_PRINTF_BUF("kj_mode_table", kj_mode_table, KEY_NUMBER);
}

uint8_t attr_button_mode_table_set(uint8_t index, uint8_t state)
{
    if (index >= KEY_NUMBER) {
        return 0;
    }

    kj_mode_table[index] = state;
    APP_PRINTF("key_id[%d] key_type[%d]\n", index, state);

    attr_kj_mode_table_save();

    return 1;
}

// 获取按键类型
uint8_t attr_kj_mode_table_get(uint8_t index)
{
    if (index >= KEY_NUMBER) {
        return 0xff;
    }
    return kj_mode_table[index];
}

// 控制一个或多个LED的状态
void switch_adapter_led_bk_ctrl(uint8_t id, uint8_t onoff)
{
    if (id == 0xff) {
        // Y_LED_CMD(onoff, 1);
        // Y_LED_CMD(onoff, 3);
        // Y_LED_CMD(onoff, 5);
        // Y_LED_CMD(onoff, 7);
    } else {
        // Y_LED_CMD(onoff, id*2+1);
    }
}

/* ****************************************************************************************************** */
// 按键状态

uint8_t attr_key_state_table_get(uint8_t id)
{
    return button_state_table[id];
}

uint8_t attr_key_state_table_set(uint8_t index, uint8_t on_off)
{
    if (index >= KEY_NUMBER) {
        return 0;
    }
    button_state_table[index] = on_off;
    return 1;
}
/* ****************************************************************************************************** */

void switch_status_by_bits(const uint8_t *data, uint8_t data_len)
{
    // APP_PRINTF_BUF("data", data, data_len);
    uint8_t cmd_type = data[0];
    uint16_t ctrl_bits = (data[1] << 8) | data[2];
    uint8_t idx = 0;
    for (uint8_t i = 0; i < 16; i++) {
        if (ctrl_bits & (1 << (15 - i))) {  // 判断该继电器是否需要控制
            uint8_t status = data[3 + idx]; // 获取目标状态
            switch (cmd_type) {
            case BUTTON_TYPE: {
                attr_key_state_table_set(i, status); // 按键状态
            } break;
            case GRAD_TYPE: {
            } break;
            case DIAL_TYPE: {
            } break;
            case REALY_TYPE: {
                attr_relay_table_set(i, status);
            } break;
            case BLCK_TYPE: {
                // attr_led_b_table_set(i, status);
                if (status == 1) {
                    status = 0x64;
                }
                attr_ad_led_b_table_set(i, status);
            } break;
            case COUTN_TYPE: {
            } break;
            case INPUT_CHECK: {
            } break;
            case ADJUST_BLCK_TYPE: {
                attr_ad_led_b_table_set(i, status);
            } break;
            default:
                break;
            }
            idx++; // 增加索引
        }
    }
}

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
