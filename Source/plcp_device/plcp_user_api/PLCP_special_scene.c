#include "PLCP_special_scene.h"
#include "../../Source/base/base.h"
#include "../../Source/flash/flash.h"
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/plcp_device/MseProcess.h"
#include "../../Source/plcp_device/plcp_linght_ct/light_ct_attr_table.h"
#include "../../Source/plcp_device/plcp_panel/attr_table.h"
#include "../../Source/timer/timer.h"
#include "../../device/device_manager.h"

bool curtain_exe = false; // 是否已开启窗帘定时器

// 函数声明
static void night_delay(void *arg);

static uint16_t delay_scene_id = 0;
static uint16_t delay_scene_time = 0;
static uint8_t delay_scene_active_timer = 0xff;
static void (*delay_scene_active_handler_fun)(void) = NULL;

static DelayScene_t my_DelayScene = {0};
static NightScene_t my_NightScene = {0};
static uint8_t night_scene_current; // 夜灯模式当前状态(0:正常模式;1:夜灯模式;2:即将进入夜灯模式)

uint8_t special_scene_set(uint8_t *data, uint8_t len)
{
#if 0
    uint8_t enable = data[3];
    uint16_t ctrl_bits = (data[1] << 8) | data[2];

    uint16_t data1; // 场景号
    uint16_t data2; // 场景号/延时时间

    memcpy((uint8_t *)&data1, &data[4], 2);
    memcpy((uint8_t *)&data2, &data[6], 2);

    switch (ctrl_bits) {
    case DELAY: {
        if (data1 == 0 || data1 == 0xFFFF)
            return 0;

        uint8_t i;
        for (i = 0; i < DelaySceneIndex; i++) {
            if (my_DelayScene[i].scene_id == data1) {
                break;
            }
        }
        if (i == DELAY_SCENE_MAX)
            return 0;

        if (i == DelaySceneIndex) {
            DelaySceneIndex++;
        }
        my_DelayScene[i].enable = enable;
        my_DelayScene[i].scene_id = data1;
        my_DelayScene[i].scene_timer = data2;
        APP_SaveDelaySceneParameter();
    } break;

    case NIGHT: {
        if (data1 == 0 || data1 == 0xFFFF)
            return 0;
        APP_PRINTF("NIGHT\n");
        APP_PRINTF("enable:%d data1:%04X data2:%04X\n", enable, data1, data2);
        my_NightScene.night_enable = enable;
        my_NightScene.open_night = data1;
        my_NightScene.close_night = data2;
        if (APP_SaveNightSceneParameter() != FMC_READY) {
            APP_PRINTF("APP_SaveNightSceneParameter error\n");
        }
    } break;
    default:
        return 0;
    }
    return 1;
#endif
    uint16_t ctrl_bits = (data[1] << 8) | data[2];

    if ((ctrl_bits >> 15) & 0x01) { // 延时场景
        uint16_t delay_id = 0;

        memcpy((uint8_t *)&delay_id, &data[4], 2);
        if (delay_id == 0 || delay_id == 0xFFFF)
            return 0;

        my_DelayScene.enable = data[3];
        memcpy((uint8_t *)&my_DelayScene.scene_id, &data[4], 2);
        memcpy((uint8_t *)&my_DelayScene.scene_timer, &data[6], 2);

        APP_PRINTF("enable:%d my_DelayScene[i].scene_id:%04X my_DelayScene[i].scene_timer:%04X\n", my_DelayScene.enable, my_DelayScene.scene_id, my_DelayScene.scene_timer);

        if (APP_SaveDelaySceneParameter() != FMC_READY) {
            APP_PRINTF("APP_SaveDelaySceneParameter error\n");
        }
    }
    if ((ctrl_bits >> 14) & 0x01) { // 夜灯场景

        my_NightScene.night_enable = data[8];
        memcpy((uint8_t *)&my_NightScene.open_night, &data[9], 2);
        memcpy((uint8_t *)&my_NightScene.close_night, &data[11], 2);

        APP_PRINTF("enable:%d my_NightScene.open_night:%04X my_NightScene.close_nightL:%04X\n",
                   my_NightScene.night_enable, my_NightScene.open_night, my_NightScene.close_night);

        if (APP_SaveNightSceneParameter() != FMC_READY) {
            APP_PRINTF("APP_SaveNightSceneParameter error\n");
        }
    }
    return 1;
}

// 返回夜灯结构体
NightScene_t *special_night_scene_get(void)
{
    return &my_NightScene;
}

DelayScene_t *special_delay_scene_get(void)
{
    return &my_DelayScene;
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

    APP_PRINTF("my_DelayScene.scene_id:[%04X].enable[%d].timer[%04X]\n", my_DelayScene.scene_id, my_DelayScene.enable, my_DelayScene.scene_timer);

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

void delay_scene_stop(void)
{
    app_timer_stop("night_delay");
    curtain_exe = false;
    night_scene_current = 0;
    APP_PRINTF("night_scene_current:%d\n", night_scene_current);
}

/*************************************************************************/

const NightScene_t *night_scene_info_get(void)
{
    return &my_NightScene;
}

const uint8_t night_scene_state_get(void)
{
    return night_scene_current;
}

void night_scene_open(void)
{
    night_scene_current = 2; // 即将进入夜灯模式
#if defined PLCP_PANEL
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        attr_led_table_set(i, 0); // 立即关闭指示灯
        attr_key_state_table_set(i, 0);
        attr_led_b_table_set(i, 100);

        app_timer_stop("night_delay");
        app_timer_stop("curtain_hold");

        app_timer_start(10000, night_delay, false, NULL, "night_delay");
    }
#elif defined PLCP_LIGHT_CT
    attr_light_ct_table_set(false);
#endif
}

static void night_delay(void *arg)
{
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        attr_led_b_table_set(i, 0);
    }
    night_scene_current = 1; // 进入夜灯模式
    APP_PRINTF("night_scene_current_enternight:%d\n", night_scene_current);
}

void night_scene_close(void)
{
    APP_PRINTF("night_scene_close\n");
    night_scene_current = 0;
    for (uint8_t i = 0; i < KEY_NUMBER; i++) {
        attr_led_b_table_set(i, 100);
    }
}

// 关闭夜灯模式
void night_scene_off_send(void)
{
    char rsl_str[64];
    uint16_t scene;
    if (night_scene_current == 1 && my_NightScene.open_night != 0xffff && my_NightScene.open_night != 0) {
        scene = my_NightScene.open_night;
        scene = (scene >> 8) | (scene << 8); // 交换端序号
        snprintf(rsl_str, sizeof(rsl_str), "%04X@SE202.FFFFFFFFFFFF/_on", scene);
        APP_SendRSL(rsl_str, 0, NULL, 0);
    }
    night_scene_current = 0;
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
    ret = app_flash_read(FLASH_PANEL_NIGHT_SCENE, (uint32_t *)&my_NightScene, sizeof(my_NightScene));

    APP_PRINTF("my_NightScene.enable:%d open_night:%04X close_night:%04X\n", my_NightScene.night_enable, my_NightScene.open_night, my_NightScene.close_night);

    return ret;
}

void APP_SpecialSceneClr(void)
{
    memset(&my_DelayScene, 0xFF, sizeof(my_DelayScene));
    memset(&my_NightScene, 0xFF, sizeof(my_NightScene));
    APP_SaveNightSceneParameter();
    APP_SaveDelaySceneParameter();
}
