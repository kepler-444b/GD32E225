
#ifndef _sp_s_H_
#define _sp_s_H_

#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../../Source/flash/flash.h"

#define FLASH_PANEL_DELAY_TABLE 0x0801E000UL // 120页
#define FLASH_PANEL_NIGHT_SCENE 0x0801E400UL // 121页

#define DELAY_SCENE_MAX         32

typedef struct {
    uint8_t enable;
    uint16_t scene_id;
    uint16_t scene_timer;
} DelayScene_t;

typedef struct
{
    uint8_t night_enable;
    uint16_t open_night;  // 触发夜灯属性场景号
    uint16_t close_night; // 退出夜灯属性场景号
    uint8_t reserved;
} NightScene_t;

uint8_t
delay_scene_set(uint16_t scene_id, uint16_t time);
uint16_t delay_scene_id_get(void);
uint16_t delay_scene_time_get(void);
uint8_t delay_scene_active(uint16_t scene_id, void (*delay_scene_active_handler)(void));
void delay_scene_stop(void);

/*************************************************************************/
const NightScene_t *night_scene_info_get(void);
const uint8_t night_scene_state_get(void);

void night_scene_open(void);
void night_scene_close(void);

void night_scene_off_send(void);

uint8_t special_scene_set(uint8_t *data, uint8_t len);
fmc_state_enum APP_ReadDelaySceneParameter(void);
fmc_state_enum APP_SaveDelaySceneParameter(void);

fmc_state_enum APP_ReadNightSceneParameter(void);
fmc_state_enum APP_SaveNightSceneParameter(void);
void APP_SpecialSceneinit(void);

#endif
