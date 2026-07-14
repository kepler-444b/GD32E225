#include "PLCP_special_scene.h"
#include "../../Source/base/base.h"
#include "../../Source/flash/flash.h"
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/plcp_device/MseProcess.h"
#include "../../Source/plcp_device/plcp_linght_ct/light_ct_attr_table.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/timer/timer.h"
#include "../../device/device_manager.h"

// 函数声明
static void night_delay(void *arg);

static uint16_t delay_scene_id = 0;
static uint16_t delay_scene_time = 0;
static uint8_t delay_scene_active_timer = 0xff;
static void (*delay_scene_active_handler_fun)(void) = NULL;

static DelayScene_t my_DelayScene[NIGHT_SCENE_MAX] = {0};
static NightScene_t my_NightScene[5] = {0};

uint8_t special_scene_set(uint8_t *data, uint8_t len)
{
    uint16_t ctrl_bits = (data[1] << 8) | data[2];
    uint8_t delay_insert = data[2]; // 夜灯数组插入位置

    if ((ctrl_bits >> 15) & 0x01) { // 延时场景
        uint16_t delay_id = 0;

        memcpy((uint8_t *)&delay_id, &data[4], 2);
        if (delay_id == 0 || delay_id == 0xFFFF)
            return 0;

        my_DelayScene[0].enable = data[3];
        memcpy((uint8_t *)&my_DelayScene[0].scene_id, &data[4], 2);
        memcpy((uint8_t *)&my_DelayScene[0].scene_timer, &data[6], 2);

        APP_PRINTF("enable:%d my_DelayScene[i].scene_id:%04X my_DelayScene[i].scene_timer:%04X\n", my_DelayScene[0].enable, my_DelayScene[0].scene_id, my_DelayScene[0].scene_timer);

        if (APP_SaveDelaySceneParameter() != FMC_READY) {
            APP_PRINTF("APP_SaveDelaySceneParameter error\n");
        }
    }
    if ((ctrl_bits >> 14) & 0x01) { // 夜灯场景
        my_NightScene[delay_insert].night_enable = data[8];
        my_NightScene[delay_insert].night_scene_current = 0;
        memcpy((uint8_t *)&my_NightScene[delay_insert].open_night, &data[9], 2);
        memcpy((uint8_t *)&my_NightScene[delay_insert].close_night, &data[11], 2);

        if (APP_SaveNightSceneParameter() != FMC_READY) {
            APP_PRINTF("APP_SaveNightSceneParameter error\n");
        }
    }
    return 1;
}

// 返回夜灯结构体
NightScene_t *special_night_scene_get(void)
{
    return my_NightScene;
}

DelayScene_t *special_delay_scene_get(void)
{
    return my_DelayScene;
}

// 保存延时场景参数到flash中
fmc_state_enum APP_SaveDelaySceneParameter(void)
{
    return app_flash_program(FLASH_PANEL_DELAY_TABLE, (uint32_t *)&my_DelayScene, sizeof(my_DelayScene), true);
}

// 从flsh中读取延时场景
fmc_state_enum APP_ReadDelaySceneParameter(void)
{
    fmc_state_enum ret;

    ret = app_flash_read(FLASH_PANEL_DELAY_TABLE, (uint32_t *)&my_DelayScene, sizeof(my_DelayScene));

    if (ret != FMC_READY) {
        return ret;
    }
    if (my_DelayScene[0].enable == 0xFF) {
        my_DelayScene[0].enable = 1;
        my_DelayScene[0].scene_id = 0x0206;
        my_DelayScene[0].scene_timer = 0x0A;
        APP_PRINTF("Use default delay scene\n");
    }
    // APP_PRINTF("my_DelayScene.scene_id:[%04X].enable[%d].timer[%04X]\n", my_DelayScene.scene_id, my_DelayScene.enable, my_DelayScene.scene_timer);
    return ret;
}

uint8_t delay_scene_set(uint16_t scene_id, uint16_t time)
{
    if (scene_id != 0xffff && scene_id != 0) {
        delay_scene_id = scene_id;
        delay_scene_time = time;
        // printf( "delay_scene_set %04x, %d\n", delay_scene_id, delay_scene_time);
        return 1;
    } else {
        return 0;
    }
}

uint16_t delay_scene_id_get(void)
{
    return delay_scene_id;
}

uint16_t delay_scene_time_get(void)
{
    return delay_scene_time;
}

static void delay_scene_active_timer_handler(void)
{
    // APP_StopGenTimer(delay_scene_active_timer);
    // if(delay_scene_active_handler_fun != NULL){
    // 	delay_scene_active_handler_fun();
    // }
}

#if 0
uint8_t delay_scene_active(uint16_t scene_id, void (*delay_scene_active_handler)(void))
{
    // printf( "delay_scene_active %04x\n", scene_id);

    if (scene_id == 0xffff || scene_id == 0 || scene_id != delay_scene_id) {
        return 0;
    }

    // if(delay_scene_active_timer == 0xff){
    // 	delay_scene_active_timer = APP_NewGenTimer(1, delay_scene_active_timer_handler);
    // }
    // delay_scene_active_handler_fun = delay_scene_active_handler;
    // APP_StopGenTimer(delay_scene_active_timer);
    // APP_SetGenTimer(delay_scene_active_timer, delay_scene_time*1000);
    // APP_StartGenTimer(delay_scene_active_timer);
    // return 1;
}
#endif

void delay_scene_stop(uint8_t night_num)
{
    app_timer_stop("night_delay");
    my_NightScene[night_num].night_scene_current = 0;
    for (uint8_t i = 0; i < 5; i++) {
        APP_PRINTF("night_scene_current[%d] :%d\n", i, my_NightScene[i].night_scene_current);
    }
    APP_PRINTF("my_NightScene[%d].night_scene_current:%d\n", night_num, my_NightScene[night_num].night_scene_current);
}

/*************************************************************************/

const NightScene_t *night_scene_info_get(void)
{
    return my_NightScene;
}

const uint8_t night_scene_state_get(uint8_t night_mun)
{
    return my_NightScene[night_mun].night_scene_current;
}

void night_scene_open(uint8_t night_num)
{
    my_NightScene[night_num].night_scene_current = 2; // 即将进入夜灯模式

    // for (uint8_t i = 0; i < NIGHT_SCENE_MAX; i++) {
    //     APP_PRINTF("my_NightScene[%d].night_scene_current:%d\n", i, my_NightScene[i].night_scene_current);
    // }

    static uint8_t s_night_num = 0;
    s_night_num = night_num;

#if defined PLCP_PANEL
    attr_key_state_table_recover(); // 恢复按键状态
    app_timer_stop("night_delay");
    app_timer_start(10000, night_delay, false, &s_night_num, "night_delay");

#elif defined PLCP_LIGHT_CT
    attr_light_ct_table_set(false); // 关闭灯
    app_timer_stop("night_delay");
    app_timer_start(10000, night_delay, false, &s_night_num, "night_delay");
#endif
}

static void night_delay(void *arg)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        switch_led_b_ctrl(i, 0, 1); // 关闭背光灯
        switch_led_ctrl(i, 0);      // 关闭指示灯
    }

    uint8_t night_num = *(uint8_t *)arg;
    APP_PRINTF("night_num:%d\n", night_num);

    my_NightScene[night_num].night_scene_current = 1; // 进入夜灯模式
    for (uint8_t i = 0; i < NIGHT_SCENE_MAX; i++) {
        APP_PRINTF("my_NightScene[%d].night_scene_current:%d\n", i, my_NightScene[i].night_scene_current);
    }
    APP_SaveNightSceneParameter();
}

// 退出夜灯模式
void night_scene_close(uint8_t night_num)
{
    APP_PRINTF("[%d]night_scene_close\n", night_num);
    my_NightScene[night_num].night_scene_current = 0;
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        switch_led_b_ctrl(i, 100, 1);
    }
    APP_SaveNightSceneParameter();
}

// 关闭夜灯模式
void night_scene_off_send(uint8_t night_num)
{
    char rsl_str[64];
    uint16_t scene;
    if (my_NightScene[night_num].night_scene_current == 1 && my_NightScene[night_num].open_night != 0xffff && my_NightScene[night_num].open_night != 0) {
        scene = my_NightScene[night_num].open_night;
        scene = (scene >> 8) | (scene << 8); // 交换端序号
        snprintf(rsl_str, sizeof(rsl_str), "%04X@SE202.FFFFFFFFFFFF/_on", scene);
        APP_SendRSL(rsl_str, 0, NULL, 0);
    }
    my_NightScene[night_num].night_scene_current = 0;
    APP_SaveNightSceneParameter();
}

// 保存夜灯场景参数到flash中
fmc_state_enum APP_SaveNightSceneParameter(void)
{
    return app_flash_program(FLASH_PANEL_NIGHT_SCENE, (uint32_t *)&my_NightScene, sizeof(my_NightScene), true);
}

// 从flash中读取夜灯场景
fmc_state_enum APP_ReadNightSceneParameter(void)
{
    fmc_state_enum ret;
    bool use_default_night_scenes = false;
    ret = app_flash_read(FLASH_PANEL_NIGHT_SCENE, (uint32_t *)&my_NightScene, sizeof(my_NightScene));

    if (my_NightScene[0].night_enable == 0xFF) {
        use_default_night_scenes = true;
        my_NightScene[0].night_enable = 1;
        my_NightScene[0].open_night = 0x0206;
        my_NightScene[0].close_night = 0x0206;
        my_NightScene[0].night_scene_current = 0;
    }
    if (my_NightScene[1].night_enable == 0xFF) {
        use_default_night_scenes = true;
        my_NightScene[1].night_enable = 1;
        my_NightScene[1].open_night = 0x0300;
        my_NightScene[1].close_night = 0x0300;
        my_NightScene[1].night_scene_current = 0;
    }
    if (use_default_night_scenes == true) {
        APP_SaveNightSceneParameter();
        APP_PRINTF("use default night scenes\n");
    }

    for (uint8_t i = 0; i < NIGHT_SCENE_MAX; i++) {
        APP_PRINTF("my_NightScene[%d].enable:%d open_night:%04X close_night:%04X night_scene_current:%d\n",
                   i, my_NightScene[i].night_enable, my_NightScene[i].open_night, my_NightScene[i].close_night, my_NightScene[i].night_scene_current);
    }
    if (my_NightScene[0].night_scene_current == 1) { // 恢复之前的夜灯模式
        for (uint8_t i = 0; i < KEY_NUMBER; i++) {
            switch_led_b_ctrl(i, 0, 1); // 关闭背光灯
            switch_led_ctrl(i, 0);      // 关闭指示灯
        }
    } else {
        attr_key_state_table_recover();
    }
    return ret;
}

void APP_SpecialSceneClr(void)
{
    memset(&my_DelayScene, 0xFF, sizeof(my_DelayScene));
    memset(&my_NightScene, 0xFF, sizeof(my_NightScene));
    APP_SaveNightSceneParameter();
    APP_SaveDelaySceneParameter();
}
