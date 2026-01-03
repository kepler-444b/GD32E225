
#include "../../Source/device/device_manager.h"
#if defined PLCP_DEVICE
#include "../../Source/base/base.h"
#include "../../Source/base/debug.h"
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/MseProcess.h"
#include "../../Source/plcp_device/plcp_panel/plcp_panel_info.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_bind.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_scene.h"
#include "../../Source/plcp_device/plcp_user_api/PLCP_special_scene.h"
#include "../../Source/pwm/pwm_hw.h"
#include "systick.h"

static uint8_t groupIndex;                      // 默认群组数
static uint16_t DeviceGroup[MAX_GROUP_NUMBERS]; // 默认群组

static sDevice_Scene DeviceScene[MAX_SCENE_NUMBERS]; // 默认场景数据结构
static uint8_t sceneIndex;                           // 默认场景数

static uint8_t ch_sceneIndex[CH_SCENE_NUM];   // 继电器场景列表索引
static uint8_t led_sceneIndex[LED_SCENE_NUM]; // 指示灯场景列表索引
static uint8_t ad_led_sceneIndex[AD_LED_SCENE_NUM];

static ch_scene my_ch_scene[CH_SCENE_NUM];    // 继电器场景列表
static led_scene my_led_scene[LED_SCENE_NUM]; // 指示灯场景列表
// static ad_led_scene my_ad_led_scene[AD_LED_SCENE_NUM];

static uint8_t ch_groupIndex[CH_GROUP_NUM];   // 继电器群组列表索引
static uint8_t led_groupIndex[LED_GROUP_NUM]; // 指示灯群组列表索引

static ch_group my_ch_group[CH_GROUP_NUM];    // 继电器群组列表
static led_group my_led_group[LED_GROUP_NUM]; // 指示灯群组列表

// 函数声明
static fmc_state_enum APP_SaveChSceneParameter(void);
static fmc_state_enum APP_ReadLedSceneParameter(void);

static fmc_state_enum APP_SaveLedSceneParameter(void);
static fmc_state_enum APP_ReadLedSceneParameter(void);

static fmc_state_enum APP_ReadAdLedSceneParameter(void);
static fmc_state_enum APP_SaveAdLedSceneParameter(void);

static fmc_state_enum APP_ReadSceneParameter(void);
static fmc_state_enum APP_SaveSceneParameter(void);

static fmc_state_enum APP_SaveGroupParameter(void);
static fmc_state_enum APP_ReadGroupParameter(void);
static void APP_SceneClr(void);

void APP_GroupClr(void)
{
    memset(my_ch_group, 0, sizeof(my_ch_group));
    memset(ch_groupIndex, 0, sizeof(ch_groupIndex));
    APP_SaveGroupParameter();
}

static void APP_SceneClr(void)
{
    sceneIndex = 0;
    memset(ch_sceneIndex, 0, sizeof(ch_sceneIndex));
    memset(led_sceneIndex, 0, sizeof(led_sceneIndex));
    memset(ad_led_sceneIndex, 0, sizeof(ad_led_sceneIndex));

    memset(DeviceScene, 0, sizeof(DeviceScene));
    memset(my_ch_scene, 0, sizeof(my_ch_scene));
    memset(my_led_scene, 0, sizeof(my_led_scene));
    // memset(my_ad_led_scene, 0, sizeof(my_ad_led_scene));

    APP_SaveChSceneParameter();
    APP_SaveLedSceneParameter();
    APP_SaveSceneParameter();
    // APP_SaveAdLedSceneParameter();
}

void APP_SceneGroupClr(void)
{
    APP_GroupClr();
    APP_SceneClr();
}

//  保存继电器类的场景参数到flash中
static fmc_state_enum APP_SaveChSceneParameter(void)
{
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_CH_SCENE_INFO_START_ADD, (uint32_t *)my_ch_scene, sizeof(my_ch_scene), true);
    if (ret != FMC_READY) {
        APP_PRINTF("APP_SaveChSceneParameter error\n");
        return ret;
    }
    return ret;
}

// 从flsh中读取继电器类的场景参数
static fmc_state_enum APP_ReadChSceneParameter(void)
{
    fmc_state_enum ret;

    ret = app_flash_read(FLASH_CH_SCENE_INFO_START_ADD, (uint32_t *)my_ch_scene, sizeof(my_ch_scene));
    if (ret != FMC_READY)
        return ret;

    for (uint8_t i = 0; i < CH_SCENE_NUM; i++) {          // 遍历结构体数量
        for (uint8_t j = 0; j < MAX_SCENE_NUMBERS; j++) { // 遍历场景数量
            if (my_ch_scene[i].scenes[j].scene_id == 0 || my_ch_scene[i].scenes[j].scene_id == 0xFFFF) {
                ch_sceneIndex[i] = j;
                APP_PRINTF("ch_sceneIndex[%d] = %d\n", i, j);
                break;
            }
#if 0
            APP_PRINTF("my_ch_scene[%d].scenes[%d].scene_id[%04X] \n", i, j, my_ch_scene[i].scenes[j].scene_id);
            APP_PRINTF_BUF(".open_scene ", my_ch_scene[i].scenes[j].open_scene, 2);
            APP_PRINTF_BUF(".close_scene", my_ch_scene[i].scenes[j].close_scene, 2);
#endif
        }
    }

    return ret;
}

//  保存 led 类的场景参数到flash中
static fmc_state_enum APP_SaveLedSceneParameter(void)
{
    fmc_state_enum ret;
    ret = app_flash_program(FLASH_LED_SCENE_INFO_START_ADD, (uint32_t *)my_led_scene, sizeof(my_led_scene), true);
    if (ret != FMC_READY) {
        APP_PRINTF("APP_SaveLedSceneParameter error\n");
        return ret;
    }

    return ret;
}

// 从flsh中读取 led 类的场景参数
static fmc_state_enum APP_ReadLedSceneParameter(void)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_LED_SCENE_INFO_START_ADD, (uint32_t *)my_led_scene, sizeof(my_led_scene));
    if (ret != FMC_READY)
        return ret;

    for (uint8_t i = 0; i < 4; i++) {                     // 遍历结构体数量
        for (uint8_t j = 0; j < MAX_SCENE_NUMBERS; j++) { // 遍历场景数量
            if (my_led_scene[i].scenes[j].scene_id == 0 || my_led_scene[i].scenes[j].scene_id == 0xFFFF) {
                led_sceneIndex[i] = j;
                APP_PRINTF("led_sceneIndex[%d] = %d\n", i, j);
                break;
            }
#if 0
            APP_PRINTF("my_led_scene[%d].scenes[%d].scene_id[%04X] \n", i, j, my_led_scene[i].scenes[j].scene_id);
            APP_PRINTF_BUF(".open_scene ", my_led_scene[i].scenes[j].open_scene, 2);
            APP_PRINTF_BUF(".close_scene", my_led_scene[i].scenes[j].close_scene, 2);
#endif
        }
    }

    return ret;
}

//  保存 可调led 类的场景参数到flash中
// static fmc_state_enum APP_SaveAdLedSceneParameter(void)
// {
//     fmc_state_enum ret;
//     ret = app_flash_program(FLASH_AD_LED_SCENE_INFO_START_ADD, (uint32_t *)my_ad_led_scene, sizeof(my_ad_led_scene), true);
//     if (ret != FMC_READY) {
//         APP_PRINTF("APP_SaveLedSceneParameter error\n");
//         return ret;
//     }

//     return ret;
// }

// 从flsh中读取 可调led 类的场景参数
#if 0
static fmc_state_enum APP_ReadAdLedSceneParameter(void)
{
    fmc_state_enum ret;

    ret = app_flash_read(FLASH_AD_LED_SCENE_INFO_START_ADD, (uint32_t *)my_ad_led_scene, sizeof(my_ad_led_scene));
    if (ret != FMC_READY)
        return ret;

#if 0
    for (uint8_t i = 0; i < 4; i++) {                     // 遍历结构体数量
        for (uint8_t j = 0; j < MAX_SCENE_NUMBERS; j++) { // 遍历场景数量
            if (my_ad_led_scene[i].scenes[j].scene_id == 0 || my_ad_led_scene[i].scenes[j].scene_id == 0xFFFF) {
                ad_led_sceneIndex[i] = j;
                APP_PRINTF("ad_led_sceneIndex[%d] = %d\n", i, j);
                break;
            }
            APP_PRINTF("my_ad_led_scene[%d].scenes[%d].scene_id[%04X] \n", i, j, my_ad_led_scene[i].scenes[j].scene_id);
            APP_PRINTF_BUF(".open_scene ", my_ad_led_scene[i].scenes[j].open_scene, 2);
            APP_PRINTF_BUF(".close_scene", my_ad_led_scene[i].scenes[j].close_scene, 2);
        }
    }
#endif
    return ret;
}
#endif
// 保存默认场景参数到flash中
static fmc_state_enum APP_SaveSceneParameter(void)
{
    fmc_state_enum ret;

    ret = app_flash_program(FLASH_SCENE_INFO_START_ADD1, (uint32_t *)DeviceScene, sizeof(sDevice_Scene) * SCENE1_NUM, true);
    if (ret != FMC_READY)
        return ret;

    ret = app_flash_program(FLASH_SCENE_INFO_START_ADD2, (uint32_t *)(DeviceScene + SCENE1_NUM), sizeof(sDevice_Scene) * SCENE2_NUM, true);
    if (ret != FMC_READY)
        return ret;

    return ret;
}

// 从flsh中读取默认场景参数
static fmc_state_enum APP_ReadSceneParameter(void)
{
    fmc_state_enum ret;

    ret = app_flash_read(FLASH_SCENE_INFO_START_ADD1, (uint32_t *)DeviceScene, sizeof(sDevice_Scene) * SCENE1_NUM);
    if (ret != FMC_READY)
        return ret;

    ret = app_flash_read(FLASH_SCENE_INFO_START_ADD2, (uint32_t *)(DeviceScene + SCENE1_NUM), sizeof(sDevice_Scene) * SCENE2_NUM);
    if (ret != FMC_READY)
        return ret;

    for (uint8_t i = 0; i < MAX_SCENE_NUMBERS; i++) {
        if (DeviceScene[i].sceneId == 0 || DeviceScene[i].sceneId == 0xFFFF) {
            sceneIndex = i;
            APP_PRINTF("sceneIndex = %d\n", sceneIndex);
            break;
        }
#if 0
        APP_PRINTF("DeviceScene[%d].sceneId[%04X]\n", i, DeviceScene[i].sceneId);
        APP_PRINTF_BUF(".scenePower    ", DeviceScene[i].scenePower, DeviceScene[i].scenePower[0] + 1);
        APP_PRINTF_BUF(".quitscenePower", DeviceScene[i].quitscenePower, DeviceScene[i].quitscenePower[0] + 1);
#endif
    }
    APP_PRINTF("\n");

    return ret;
}

bool APP_ReadAllSceneInfo(void)
{
    fmc_state_enum ret = FMC_READY;
#if defined PLCP_PANEL
    if (APP_ReadChSceneParameter() != FMC_READY) {
        ret++;
    }
    if (APP_ReadLedSceneParameter() != FMC_READY) {
        ret++;
    }
#endif
    if (APP_ReadSceneParameter() != FMC_READY) {
        ret++;
    }
    if (ret == FMC_READY) {
        return true;
    }
    APP_PRINTF("APP_ReadAllSceneInfo error!\n");
    return false;
}

bool APP_ReadAllGroupInfo(void)
{
    if (APP_ReadGroupParameter() != FMC_READY) {
        return false;
    }
    return true;
}

// 保存群组参数到flash中
static fmc_state_enum APP_SaveGroupParameter(void)
{
    fmc_state_enum ret;

    ret = app_flash_program(FLASH_GROUP_INFO_START_ADD, (uint32_t *)my_ch_group, sizeof(my_ch_group), true);
    if (ret != FMC_READY)
        return ret;

    return ret;
}

// 从flsh中读取群组参数
static fmc_state_enum APP_ReadGroupParameter(void)
{
    fmc_state_enum ret;
    ret = app_flash_read(FLASH_GROUP_INFO_START_ADD, (uint32_t *)my_ch_group, sizeof(my_ch_group));
    if (ret != FMC_READY)
        return ret;
#if 1
    for (uint8_t i = 0; i < CH_GROUP_NUM; i++) {
        for (uint8_t j = 0; j < MAX_GROUP_NUMBERS; j++) {
            if (my_ch_group[i].groups[j].group == 0 || my_ch_group[i].groups[j].group == 0xFFFF) {
                ch_groupIndex[i] = j;
                APP_PRINTF("ch_groupIndex[%d] = %d\n", i, ch_groupIndex[i]);
                break;
            }
            APP_PRINTF("my_ch_group[%d].groups[%d].group:%04X\n", i, j, my_ch_group[i].groups[j].group);
        }
    }
#endif
    return ret;
}

uint8_t APP_IsHaveSceneId_Device(uint16_t sceneNumber)
{
    uint8_t i = 0;
    if (sceneNumber == 0) {
        return Device_Err_Len;
    }
    for (i = 0; i < sceneIndex; i++) {
        if (DeviceScene[i].sceneId == sceneNumber) {
            return Device_OK;
        }
    }
    return Device_Err_NoFound;
}

/* **************************************** 场景相关 **************************************** */
uint8_t APP_SetScene(single_scene_data *temp)
{
    uint8_t *open_dst = NULL;      // open 场景目标地址
    uint8_t *close_dst = NULL;     // close 场景目标地址
    uint16_t *scene_id_ptr = NULL; // 场景号目标地址
    fmc_state_enum ret_flash;

    const char *log_prefix = NULL; // 日志前缀

    switch (temp->scene_type) {
    case LED_SCENE:
        scene_id_ptr = &my_led_scene[temp->kj_index].scenes[temp->index].scene_id;
        open_dst = my_led_scene[temp->kj_index].scenes[temp->index].open_scene;
        close_dst = my_led_scene[temp->kj_index].scenes[temp->index].close_scene;
        log_prefix = "my_led_scene";
        break;

    case CH_SCENE:
        scene_id_ptr = &my_ch_scene[temp->kj_index].scenes[temp->index].scene_id;
        open_dst = my_ch_scene[temp->kj_index].scenes[temp->index].open_scene;
        close_dst = my_ch_scene[temp->kj_index].scenes[temp->index].close_scene;
        log_prefix = "my_ch_scene";
        break;

    case SCENE:
        scene_id_ptr = &DeviceScene[temp->index].sceneId;
        open_dst = DeviceScene[temp->index].scenePower;
        close_dst = DeviceScene[temp->index].quitscenePower;
        log_prefix = "DeviceScene";
        break;

    default:
        return Device_Err_NoFound;
    }

    // 设置 scene_id
    *scene_id_ptr = temp->scene_id;

    // 拷贝 open_scene
    if (open_dst) {
        memset(open_dst, 0, temp->open_scene_len);
        memcpy(open_dst, temp->open_scene, temp->open_scene_len);
    }

    // 拷贝 close_scene
    if (close_dst && temp->close_scene) {
        memset(close_dst, 0, temp->close_scene_len);
        memcpy(close_dst, temp->close_scene, temp->close_scene_len);
    }

    // 打印日志
    APP_PRINTF("%s[%d].scenes[%d].scene_id:%04X\n", log_prefix, temp->kj_index, temp->index, temp->scene_id);
    APP_PRINTF_BUF(".open_scene", open_dst, temp->open_scene_len);
    if (close_dst)
        APP_PRINTF_BUF(".close_scene", close_dst, temp->close_scene_len);

    // 保存参数
    switch (temp->scene_type) {
    case LED_SCENE:
        ret_flash = APP_SaveLedSceneParameter();
        break;
    case CH_SCENE:
        ret_flash = APP_SaveChSceneParameter();
        break;
    case SCENE:
        ret_flash = APP_SaveSceneParameter();
        break;
    default:
        return Device_Err_NoFound;
    }

    return (ret_flash == FMC_READY) ? Device_OK : Device_Err_NoFound;
}

// 添加场景
uint8_t APP_Scene_Join(uint8_t *buf, uint16_t len, const char *aei)
{
#if 0
    uint16_t sceneId;
    memcpy((uint8_t *)&sceneId, buf, 2);
    if (sceneId == 0 || sceneId == 0xFFFF || len < 3)
        return Device_Err_Len;

    uint8_t scene_len = len - 2;           // 剥离场景号(前2个字节)
    uint8_t *scene_data = &buf[2];         // 指向剥离后的数据(open scene)
    uint8_t open_data_len = scene_data[0]; // open scene 数据长度

    if ((open_data_len + 1) > scene_len)
        return Device_Err_Len;

    static single_scene_data temp;
    memset(&temp, 0, sizeof(single_scene_data));

    bool only_open = ((open_data_len + 1) == scene_len);

    if (only_open) { // 只有开场景
        APP_PRINTF("only_open\n");
        uint8_t i = 0;
        temp.scene_id = sceneId;
        temp.open_scene = scene_data;
        temp.close_scene = NULL;
        temp.open_scene_len = open_data_len + 1;

        if (strncmp(aei, "ch_", 3) == 0) {
            uint8_t kj_index = (aei[3] - '0') - 1;
            for (i = 0; i < ch_sceneIndex[kj_index]; i++) { // 判断场景是否存在
                if (my_ch_scene[kj_index].scenes[i].scene_id == sceneId)
                    break;
            }
            if (i == ch_sceneIndex[kj_index]) { // 如果是新场景
                if (ch_sceneIndex[kj_index] >= MAX_SCENE_NUMBERS)
                    return Device_Err_Full;
                ch_sceneIndex[kj_index]++;
            }
            temp.index = i;
            temp.scene_type = CH_SCENE;
            temp.kj_index = kj_index;
            return APP_SetScene(&temp);

        } else if (strncmp(aei, "led_", 4) == 0) {
            uint8_t kj_index = (aei[4] - '0') - 1;

            for (i = 0; i < led_sceneIndex[kj_index]; i++) { // 判断场景是否存在
                if (my_led_scene[kj_index].scenes[i].scene_id == sceneId)
                    break;
            }
            if (i == led_sceneIndex[kj_index]) { // 如果是新场景
                if (led_sceneIndex[kj_index] >= MAX_SCENE_NUMBERS)
                    return Device_Err_Full;
                led_sceneIndex[kj_index]++;
            }

            temp.index = i;
            temp.scene_type = LED_SCENE;
            temp.kj_index = kj_index;
            return APP_SetScene(&temp);

        } else if (aei[0] == '\0') {
            for (i = 0; i < sceneIndex; i++) {
                if (DeviceScene[i].sceneId == sceneId)
                    break;
            }
            if (i == sceneIndex) { // 如果是新场景
                if (sceneIndex >= MAX_SCENE_NUMBERS)
                    return Device_Err_Full;
                sceneIndex++;
            }
            temp.index = i;
            temp.scene_type = SCENE;
            return APP_SetScene(&temp);
        }

    } else {
        uint8_t *close_data = &scene_data[open_data_len + 1];
        uint8_t close_data_len = close_data[0];
        if ((open_data_len + 1 + close_data_len + 1) > scene_len)
            return Device_Err_Len;

        APP_PRINTF("open and close\n");
        uint8_t i = 0;
        temp.scene_id = sceneId;
        temp.open_scene = scene_data;
        temp.open_scene_len = open_data_len + 1;
        temp.close_scene = close_data;
        temp.close_scene_len = close_data_len + 1;

        if (strncmp(aei, "ch_", 3) == 0) {
            uint8_t kj_index = (aei[3] - '0') - 1;
            for (i = 0; i < ch_sceneIndex[kj_index]; i++) { // 判断场景是否存在
                if (my_ch_scene[kj_index].scenes[i].scene_id == sceneId)
                    break;
            }
            if (i == ch_sceneIndex[kj_index]) { // 如果是新场景
                if (ch_sceneIndex[kj_index] >= MAX_SCENE_NUMBERS)
                    return Device_Err_Full;
                ch_sceneIndex[kj_index]++;
            }
            temp.index = i;
            temp.scene_type = CH_SCENE;
            temp.kj_index = kj_index;
            return APP_SetScene(&temp);

        } else if (strncmp(aei, "led_", 4) == 0) {
            uint8_t kj_index = (aei[4] - '0') - 1;

            for (i = 0; i < led_sceneIndex[kj_index]; i++) { // 判断场景是否存在
                if (my_led_scene[kj_index].scenes[i].scene_id == sceneId)
                    break;
            }
            if (i == led_sceneIndex[kj_index]) { // 如果是新场景
                if (led_sceneIndex[kj_index] >= MAX_SCENE_NUMBERS)
                    return Device_Err_Full;
                led_sceneIndex[kj_index]++;
            }

            temp.index = i;
            temp.scene_type = LED_SCENE;
            temp.kj_index = kj_index;
            return APP_SetScene(&temp);

        } else if (aei[0] == '\0') {
            for (i = 0; i < sceneIndex; i++) {
                if (DeviceScene[i].sceneId == sceneId)
                    break;
            }
            if (i == sceneIndex) { // 如果是新场景
                if (sceneIndex >= MAX_SCENE_NUMBERS)
                    return Device_Err_Full;
                sceneIndex++;
            }
            temp.index = i;
            temp.scene_type = SCENE;
            return APP_SetScene(&temp);
        }
    }
    return Device_Err_Full;
#endif
    if (len < 3)
        return Device_Err_Len;

    uint16_t sceneId = buf[0] | (buf[1] << 8);
    if (sceneId == 0 || sceneId == 0xFFFF)
        return Device_Err_Len;

    uint8_t *scene_data = &buf[2];    // 剥离前2个字节的场景号
    uint8_t scene_len = len - 2;      // 数据长度
    uint8_t open_len = scene_data[0]; // open 场景的长度

    if ((open_len + 1) > scene_len) // 若 open 场景的长度 > 数据长度
        return Device_Err_Len;

    uint8_t *close_data = NULL;
    uint8_t close_len = 0;
    bool only_open = (open_len + 1 == scene_len); // 是否只有 open 场景(open 场景长度 = 数据长度)

    if (!only_open) {
        close_data = &scene_data[open_len + 1]; // 指向了 close 场景数据(+1:包含本身)
        close_len = close_data[0];
        if ((open_len + 1 + close_len + 1) > scene_len)
            return Device_Err_Len;
    }

    static single_scene_data temp;
    memset(&temp, 0, sizeof(temp));
    temp.scene_id = sceneId;
    temp.open_scene = scene_data;
    temp.open_scene_len = open_len + 1;
    temp.close_scene = close_data;
    temp.close_scene_len = close_len + 1;

    uint8_t i = 0;
    uint8_t kj_index = 0;

    if (strncmp(aei, "ch_", 3) == 0) { // 继电器场景列表
        temp.scene_type = CH_SCENE;
        kj_index = aei[3] - '0' - 1;
        temp.kj_index = kj_index;

        for (i = 0; i < ch_sceneIndex[kj_index]; i++) { // 查找是否已存在
            if (my_ch_scene[kj_index].scenes[i].scene_id == sceneId)
                break;
        }

        if (i == ch_sceneIndex[kj_index]) { // 如果是新场景
            if (ch_sceneIndex[kj_index] >= MAX_SCENE_NUMBERS)
                return Device_Err_Full;
            ch_sceneIndex[kj_index]++;
        }
        temp.index = i;
        return APP_SetScene(&temp);

    } else if (strncmp(aei, "led_", 4) == 0) { // LED 场景列表
        temp.scene_type = LED_SCENE;
        kj_index = aei[4] - '0' - 1;
        temp.kj_index = kj_index;

        for (i = 0; i < led_sceneIndex[kj_index]; i++) {
            if (my_led_scene[kj_index].scenes[i].scene_id == sceneId)
                break;
        }
        if (i == led_sceneIndex[kj_index]) {
            if (led_sceneIndex[kj_index] >= MAX_SCENE_NUMBERS)
                return Device_Err_Full;
            led_sceneIndex[kj_index]++;
        }
        temp.index = i;
        return APP_SetScene(&temp);

    } else if (aei[0] == '\0') { // 默认场景列表
        temp.scene_type = SCENE;

        for (i = 0; i < sceneIndex; i++) {
            if (DeviceScene[i].sceneId == sceneId)
                break;
        }
        if (i == sceneIndex) {
            if (sceneIndex >= MAX_SCENE_NUMBERS)
                return Device_Err_Full;
            sceneIndex++;
        }
        temp.index = i;
        return APP_SetScene(&temp);
    }

    return Device_Err_Full;
}

uint8_t APP_Scene_Quit(uint8_t *buf, uint16_t len, const char *aei)
{
    APP_PRINTF("APP_Scene_Quit\n");
    uint16_t sceneId;
    memcpy((uint8_t *)&sceneId, buf, 2);
    if (sceneId == 0 || sceneId == 0xffff) {
        return Device_Err_Len;
    }
#if defined PLCP_PANEL
    if (strncmp(aei, "ch_", 3) == 0) {
        uint8_t kj_index = (aei[3] - '0') - 1;

        uint8_t i = 0;

        for (i = 0; i < ch_sceneIndex[kj_index]; i++) {
            if (my_ch_scene[kj_index].scenes[i].scene_id == sceneId)
                break;
        }
        if (i == ch_sceneIndex[kj_index])
            return Device_Err_NoFound;

        ch_sceneIndex[kj_index]--;
        memcpy(&my_ch_scene[kj_index].scenes[i], &my_ch_scene[kj_index].scenes[i + 1], (ch_sceneIndex[kj_index] - i) * sizeof(single_ch_scene));
        memset(&my_ch_scene[kj_index].scenes[ch_sceneIndex[kj_index]], 0, sizeof(single_ch_scene));
        APP_SaveChSceneParameter();
        return Device_OK;
    }
    if (strncmp(aei, "led_", 4) == 0) {
        uint8_t kj_index = (aei[4] - '0') - 1;
        uint8_t i = 0;
        for (i = 0; i < led_sceneIndex[kj_index]; i++) {
            if (my_led_scene[kj_index].scenes[i].scene_id == sceneId)
                break;
        }
        if (i == led_sceneIndex[kj_index])
            return Device_Err_NoFound;

        led_sceneIndex[kj_index]--;
        memcpy(&my_led_scene[kj_index].scenes[i], &my_led_scene[kj_index].scenes[i + 1], (led_sceneIndex[kj_index] - i) * sizeof(single_led_scene));
        memset(&my_led_scene[kj_index].scenes[ch_sceneIndex[kj_index]], 0, sizeof(single_led_scene));
        APP_SaveLedSceneParameter();
        return Device_OK;
    }
#elif defined PLCP_LIGHT_CT
    uint8_t i = 0;
    for (i = 0; i < sceneIndex; i++) {
        if (DeviceScene[i].sceneId == sceneId) {
            break;
        }
    }
    if (i == sceneIndex) {
        return Device_Err_Full;
    }
    sceneIndex--;
    memcpy(&DeviceScene[i], &DeviceScene[i + 1], (sceneIndex - i) * sizeof(sDevice_Scene));
    memset(&DeviceScene[sceneIndex], 0, sizeof(sDevice_Scene));
    APP_SaveSceneParameter();
    return Device_OK;
#endif
    return Device_OK;
}

uint16_t APP_Scene_List(uint8_t *buf, const char *aei)
{
    uint16_t index = 0;
    if (strncmp(aei, "ch_", 3) == 0) {
        uint8_t kj_index = (aei[3] - '0') - 1;

        buf[index++] = ch_sceneIndex[kj_index];
        for (uint8_t i = 0; i < ch_sceneIndex[kj_index]; i++) {
            buf[index++] = (uint8_t)(my_ch_scene[kj_index].scenes[i].scene_id & 0xff);
            buf[index++] = (uint8_t)(my_ch_scene[kj_index].scenes[i].scene_id >> 8);
        }
        return index;
    }
    if (strncmp(aei, "led_", 4) == 0) {
        uint8_t kj_index = (aei[4] - '0') - 1;
        buf[index++] = led_sceneIndex[kj_index];
        for (uint8_t i = 0; i < led_sceneIndex[kj_index]; i++) {
            buf[index++] = (uint8_t)(my_led_scene[kj_index].scenes[i].scene_id & 0xff);
            buf[index++] = (uint8_t)(my_led_scene[kj_index].scenes[i].scene_id >> 8);
        }
        return index;
    } else if (aei[0] == '\0') {
        buf[index++] = sceneIndex;
        for (uint8_t i = 0; i < sceneIndex; i++) {
            buf[index++] = (uint8_t)(DeviceScene[i].sceneId & 0xff);
            buf[index++] = (uint8_t)(DeviceScene[i].sceneId >> 8);
        }
        return index;
    }
    return 0;
}

uint16_t APP_Scene_Copy_Get(uint8_t *buf, const char *aei)
{
#if 0
    for (uint8_t i = 0; i < sceneIndex; i++) {
    }
    for (uint8_t i = 0; i < CH_SCENE_NUM; i++) {
        for (uint8_t j = 0; j < ch_sceneIndex[i]; j++) {
        }
    }
    for (uint8_t i = 0; i < LED_SCENE_NUM; i++) {
        for (uint8_t j = 0; j < led_sceneIndex[i]; j++) {
        }
    }
#endif
    uint16_t index = 1; // buf[0] 用来存总场景数
    uint8_t scene_count = 0;

    // -------------------------
    // 1. 默认场景列表 DeviceScene
    // -------------------------
    for (uint8_t i = 0; i < sceneIndex; i++) {
        uint16_t sceneId = DeviceScene[i].sceneId;

        // 场景号（高字节在前）
        buf[index++] = (sceneId >> 8) & 0xFF;
        buf[index++] = sceneId & 0xFF;

        // AEI为空
        buf[index++] = 0x00;
        buf[index++] = 0x00;
        buf[index++] = 0x00;
        buf[index++] = 0x00;
        buf[index++] = 0x00;

        // 状态类型（1 = 只有打开场景）
        buf[index++] = 0x01;

        // 数据长度
        buf[index++] = 0x01;

        // 数据内容（这里先用0示例，可以改为 DeviceScene[i].scenePower[0]）
        buf[index++] = 0x00;

        scene_count++;
    }

    // -------------------------
    // 2. 继电器场景列表 my_ch_scene
    // -------------------------
    for (uint8_t i = 0; i < CH_SCENE_NUM; i++) {
        for (uint8_t j = 0; j < ch_sceneIndex[i]; j++) {
            uint16_t sceneId = my_ch_scene[i].scenes[j].scene_id;

            // 场景号
            buf[index++] = (sceneId >> 8) & 0xFF;
            buf[index++] = sceneId & 0xFF;

            // AEI = ch_1, ch_2 ...
            buf[index++] = 'c';
            buf[index++] = 'h';
            buf[index++] = '_';
            buf[index++] = '0' + (i + 1);
            buf[index++] = 0x00;

            // 状态类型（1 = 只有打开场景）
            buf[index++] = 0x01;

            // 数据长度
            buf[index++] = 0x01;

            // 数据内容（示例用 open_scene[0]）
            buf[index++] = my_ch_scene[i].scenes[j].open_scene[0];

            scene_count++;
        }
    }

    // -------------------------
    // 3. LED场景列表 my_led_scene
    // -------------------------
    for (uint8_t i = 0; i < LED_SCENE_NUM; i++) {
        for (uint8_t j = 0; j < led_sceneIndex[i]; j++) {
            uint16_t sceneId = my_led_scene[i].scenes[j].scene_id;

            // 场景号
            buf[index++] = (sceneId >> 8) & 0xFF;
            buf[index++] = sceneId & 0xFF;

            // AEI = led_1, led_2 ...
            buf[index++] = 'l';
            buf[index++] = 'e';
            buf[index++] = 'd';
            buf[index++] = '_';
            buf[index++] = '0' + (i + 1);

            // 状态类型
            buf[index++] = 0x01;

            // 数据长度
            buf[index++] = 0x01;

            // 数据内容（示例用 open_scene[0]）
            buf[index++] = my_led_scene[i].scenes[j].open_scene[0];

            scene_count++;
        }
    }

    buf[0] = scene_count; // 写入总场景数
    return index;         // 返回总填充长度
}

void APP_Device_SceneStart(UappsMessage *uappsMsg, const char *aei)
{
    // APP_PRINTF("APP_Device_SceneStart\n");
    uint8_t payloadFlag = 0;
    uint16_t sceneId = 0;
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uapps_rw_buffer_t scratch;
    memset(&scratch, 0, sizeof(scratch));

    APP_StrToHex((uint8_t *)aei, strlen(aei), (uint8_t *)&sceneId);
    const sPublic_attribute *pAttributeOfAPP = APP_Attribute_GetPointer();
    if (pAttributeOfAPP->did == 0) {
        if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
            respondCode = UAPPS_NOT_FOUND;
            APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
        }
        return;
    }
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        if (night_scene_info_get()->night_enable == 0x01) { // 使能夜灯模式
            if (sceneId == night_scene_info_get()->open_night) {
                night_scene_open();
            }
            respondCode = UAPPS_ACK_CHANGED;
        }

        for (uint8_t j = 0; j < CH_SCENE_NUM; j++) {
#if defined DND_MODE_PANEL // 对于清理勿扰,插卡后需要恢复原来的状态
            if (sceneId == 0x9999) {
                MCU_dnd_recover();
                respondCode = UAPPS_ACK_CHANGED;
                break;
            }
            if (sceneId == 0x6666) {
                for (uint8_t j = 0; j < KEY_NUMBER; j++) {
                    APP_SET_GPIO(get_panel_pins()->led_y_pin[j], false);
                    app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[j], 0, 100);
                }
                respondCode = UAPPS_ACK_CHANGED;
                break;
            }
#endif
            uint8_t ch_cnt = ch_sceneIndex[j];
            for (uint8_t i = 0; i < ch_cnt; i++) {
                if (my_ch_scene[j].scenes[i].scene_id == sceneId) {
                    MCU_SceneKj_exe(my_ch_scene[j].scenes[i].open_scene, j, CH_SCENE);
                    respondCode = UAPPS_ACK_CHANGED;
                    break;
                }
            }
            uint8_t led_cnt = led_sceneIndex[j];
            for (uint8_t i = 0; i < led_cnt; i++) {
                if (my_led_scene[j].scenes[i].scene_id == sceneId) {
                    MCU_SceneKj_exe(my_led_scene[j].scenes[i].open_scene, j, LED_SCENE);
                    respondCode = UAPPS_ACK_CHANGED;
                    break;
                }
            }
        }
        // 默认场景处理
        for (uint8_t i_scene = 0; i_scene < sceneIndex; i_scene++) {
#if defined DND_MODE_PANEL // 对于清理勿扰,插卡后需要恢复原来的状态
            if (sceneId == 0x9999) {
                MCU_dnd_recover();
                respondCode = UAPPS_ACK_CHANGED;
                break;
            }
            if (sceneId == 0x6666) {
                for (uint8_t j = 0; j < KEY_NUMBER; j++) {
                    APP_SET_GPIO(get_panel_pins()->led_y_pin[j], false);
                    app_set_pwm_hw_fade(get_panel_pins()->led_w_pin[j], 0, 100);
                }
                respondCode = UAPPS_ACK_CHANGED;
                break;
            }
#endif
            if (DeviceScene[i_scene].sceneId == sceneId) {

                MCU_Scene_exe(DeviceScene[i_scene].scenePower, DeviceScene[i_scene].scenePower[0]);
                respondCode = UAPPS_ACK_CHANGED;
                break;
            }
        }
    }
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_Device_SceneClose(UappsMessage *uappsMsg, const char *aei)
{
    // printf("APP_Device_SceneClose\r\n");
    uint8_t i = 0;
    uint8_t payloadFlag = 0;
    uint16_t sceneId = 0;
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uapps_rw_buffer_t scratch;
    memset(&scratch, 0, sizeof(scratch));

    APP_StrToHex((uint8_t *)aei, strlen(aei), (uint8_t *)&sceneId);
    const sPublic_attribute *pAttributeOfAPP = APP_Attribute_GetPointer();

    if (pAttributeOfAPP->did == 0) {
        if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
            respondCode = UAPPS_NOT_FOUND;
            APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
        }
        return;
    }
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        for (uint8_t j = 0; j < CH_SCENE_NUM; j++) {
            uint8_t ch_cnt = ch_sceneIndex[j];
            for (uint8_t i = 0; i < ch_cnt; i++) {
                if (my_ch_scene[j].scenes[i].scene_id == sceneId) {
                    MCU_SceneKj_exe(my_ch_scene[j].scenes[i].close_scene, j, CH_SCENE);
                    respondCode = UAPPS_ACK_CHANGED;
                    break;
                }
            }
            uint8_t led_cnt = led_sceneIndex[j];
            for (uint8_t i = 0; i < led_cnt; i++) {
                if (my_led_scene[j].scenes[i].scene_id == sceneId) {
                    MCU_SceneKj_exe(my_led_scene[j].scenes[i].close_scene, j, LED_SCENE);
                    respondCode = UAPPS_ACK_CHANGED;
                    break;
                }
            }
        }
        // 默认场景数据
        for (uint8_t i_scene = 0; i_scene < sceneIndex; i_scene++) {
            if (DeviceScene[i_scene].sceneId == sceneId) {
                MCU_Scene_exe(DeviceScene[i_scene].quitscenePower, DeviceScene[i_scene].quitscenePower[0]);
                respondCode = UAPPS_ACK_CHANGED;
                break;
            }
        }
    }

    // 最后统一发送 ACK
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

/* **************************************** 群组相关 **************************************** */
uint8_t APP_Group_Join(uint8_t *buf, uint16_t len, const char *aei)
{
    uint8_t i = 0;
    uint16_t groupNumber;
    memcpy((uint8_t *)&groupNumber, buf, len);
    if (len != 2) {
        return Device_Err_Len;
    }
    APP_PRINTF("APP_Group_Join\n");

    bool match_aei =
#if defined PLCP_PANEL
        (strncmp(aei, "ch_", 3) == 0);
#elif defined PLCP_LIGHT_CT
        true;
#endif
    if (match_aei) {
        uint8_t kj_index =
#if defined PLCP_PANEL
            (uint8_t)(aei[3] - '0' - 1);
#elif defined PLCP_LIGHT_CT
            0;
#endif
        for (i = 0; i < ch_groupIndex[kj_index]; i++) { // 判断群组是否存在
            if (my_ch_group[kj_index].groups->group == groupNumber)
                break;
        }
        if (i == ch_groupIndex[kj_index]) { // 如果是新群组
            if (ch_groupIndex[kj_index] >= MAX_GROUP_NUMBERS)
                return Device_Err_Full;
            ch_groupIndex[kj_index]++;
        }

        my_ch_group[kj_index].groups[i].group = groupNumber;
        APP_PRINTF("add group:%04X\n", my_ch_group[kj_index].groups[i].group);
        APP_SaveGroupParameter();
    }
    return Device_OK;
}

uint8_t APP_Group_Quit(uint8_t *buf, uint16_t len, const char *aei)
{
    uint16_t groupNumber;
    printf("APP_Group_Quit\n");
    memcpy((uint8_t *)&groupNumber, buf, len);
    if (len != 2) {
        return Device_Err_Len;
    }

    bool match_aei =
#if defined PLCP_PANEL
        (strncmp(aei, "ch_", 3) == 0);
#elif defined PLCP_LIGHT_CT
        true;
#endif
    if (match_aei) {
        uint8_t kj_index =
#if defined PLCP_PANEL
            (uint8_t)(aei[3] - '0' - 1);
#elif defined PLCP_LIGHT_CT
            0;
#endif
        uint8_t i = 0;
        for (i = 0; i < ch_groupIndex[kj_index]; i++) {
            if (my_ch_group[kj_index].groups[i].group == groupNumber)
                break;
        }
        if (i == ch_groupIndex[kj_index])
            return Device_Err_NoFound;
        ch_groupIndex[kj_index]--;
        memcpy(&my_ch_group[kj_index].groups[i], &my_ch_group[kj_index].groups[i + 1], (ch_groupIndex[kj_index] - i) * sizeof(single_ch_group));
        memset(&my_ch_group[kj_index].groups[ch_groupIndex[kj_index]], 0, sizeof(single_ch_group));

        APP_SaveGroupParameter();
    }
    return Device_OK;
}

uint16_t APP_Group_List(uint8_t *buf, const char *aei)
{
    uint8_t i = 0;
    uint16_t index = 0;
    printf("APP_Group_List\n");
    bool match_aei =
#if defined PLCP_PANEL
        (strncmp(aei, "ch_", 3) == 0);
#elif defined PLCP_LIGHT_CT
        true;
#endif
    if (match_aei) {
        uint8_t kj_index =
#if defined PLCP_PANEL
            (uint8_t)(aei[3] - '0' - 1);
#elif defined PLCP_LIGHT_CT
            0;
#endif
        buf[index++] = ch_groupIndex[kj_index];
        for (i = 0; i < ch_groupIndex[kj_index]; i++) {
            buf[index++] = (uint8_t)(my_ch_group[kj_index].groups[i].group & 0xff);
            buf[index++] = (uint8_t)(my_ch_group[kj_index].groups[i].group >> 8);
        }
        return index;
    }
    return index;
}

uint8_t APP_isGroupExist(uint16_t groupNum, const char *aei)
{
    uint8_t i = 0;

    if (groupNum == 0) {
        return Device_Err_Len;
    }
    if (strncmp(aei, "ch_", 3) == 0) {
        uint8_t kj_index = (aei[3] - '0') - 1;
        for (i = 0; i < ch_groupIndex[kj_index]; i++) {
            if (my_ch_group[kj_index].groups[i].group == groupNum)
                return Device_OK;
        }
    }
    return Device_Err_NoFound;
}

void APP_Device_GroupState(UappsMessage *uappsMsg, const char *aei)
{
    APP_PRINTF("APP_Device_GroupState\n");
    uint8_t i = 0;
    uint8_t payloadFlag = 0;
    uint16_t groupId = 0;
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uapps_rw_buffer_t scratch;
    memset(&scratch, 0, sizeof(scratch));

    APP_StrToHex((uint8_t *)aei, strlen(aei), (uint8_t *)&groupId);
    APP_PRINTF("groupId=%04X\n", groupId);
    const sPublic_attribute *pAttributeOfAPP = APP_Attribute_GetPointer();
    if (pAttributeOfAPP->did == 0) {
        if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
            respondCode = UAPPS_NOT_FOUND;
            APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
        }
        return;
    }
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {

        for (uint8_t j = 0; j < CH_GROUP_NUM; j++) {
            for (i = 0; i < ch_groupIndex[j]; i++) {
                if (my_ch_group[j].groups[i].group == groupId)
                    break;
            }
            if (i == ch_groupIndex[j]) {
                APP_PRINTF("my_ch_group[%d] none\n", j);
            } else {
#if defined PLCP_PANEL
                MCU_GroupKj_On(false, j, CH_SCENE);
#elif defined PLCP_LIGHT_CT
                MCU_Group_State(&uappsMsg->pl_ptr[0], uappsMsg->pl_len);
#endif
                respondCode = UAPPS_ACK_CHANGED;
            }
        }
    }
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}

void APP_Device_GroupClose(UappsMessage *uappsMsg, const char *aei)
{
    printf("APP_Device_GroupClose\r\n");
    uint8_t i = 0;
    uint8_t payloadFlag = 0;
    uint16_t groupId = 0;
    uint8_t respondCode = UAPPS_BAD_REQUEST;
    uapps_rw_buffer_t scratch;
    memset(&scratch, 0, sizeof(scratch));

    APP_StrToHex((uint8_t *)aei, strlen(aei), (uint8_t *)&groupId);
    APP_PRINTF("groupId=%04X\n", groupId);
    const sPublic_attribute *pAttributeOfAPP = APP_Attribute_GetPointer();
    if (pAttributeOfAPP->did == 0) {
        if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
            respondCode = UAPPS_NOT_FOUND;
            APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
        }
        return;
    }
    if (uappsMsg->hdr.hdrCode == UAPPS_REQ_PUT) {
        for (uint8_t j = 0; j < CH_GROUP_NUM; j++) {
            for (i = 0; i < ch_groupIndex[j]; i++) {
                if (my_ch_group[j].groups[i].group == groupId)
                    break;
            }
            if (i == ch_groupIndex[j]) {
                APP_PRINTF("my_led_scene[%d] none\n", j);
            } else {
#if defined PLCP_PANEL
                MCU_GroupKj_Off(false, j, CH_SCENE);
#elif defined PLCP_LIGHT_CT
                MCU_GroupKj_Off(false, j, SCENE);
#endif
                respondCode = UAPPS_ACK_CHANGED;
            }
        }
        // i = 0;
        // for (i = 0; i < sceneIndex; i++) {
        //     if (DeviceScene[i].sceneId == groupId) break;
        // }
        // if (i == sceneIndex) {
        //     APP_PRINTF("scene none\r\n");
        //     return;
        // }
        // MCU_Scene_exe(DeviceScene[i].quitscenePower, DeviceScene[i].quitscenePower[0]);
        // respondCode = UAPPS_ACK_CHANGED;
    }
    if (uappsMsg->hdr.type == UAPPS_TYPE_CON) {
        APP_SendACK(uappsMsg, payloadFlag, &scratch, UAPPS_TYPE_ACK, respondCode);
    }
}
#endif