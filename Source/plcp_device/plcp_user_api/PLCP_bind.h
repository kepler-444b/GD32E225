#include "gd32e23x_fmc.h"

#ifndef _PLCP_BIND_H_
#define _PLCP_BIND_H_

#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "gd32e23x_fmc.h"

#define FLASH_BIND_INFO_H_START_ADD 0x08016800UL // 90页

#define MAX_BIND_MSG_LEN            64
#define MAX_BIND_TABLE_SIZE         8
/*--------------------------------------------------------------
函数名称：PLCP_bindTableWrite
函数功能：写入绑定
输入参数：msg 待写入的事件对应的消息
返回值：  1 - 成功 0 - 失败
备 注：
---------------------------------------------------------------*/
uint8_t PLCP_bindTableWrite(uint8_t se, char *aei, char *id, char *msg);
// void APP_Bindinit(void);

/*--------------------------------------------------------------
函数名称：PLCP_bindTableRead
函数功能：读取绑定的消息
输入参数： 读出消息的索引
返回值：  	null - 失败
备 注：
---------------------------------------------------------------*/
char *PLCP_bindTableRead(uint8_t se, char *aei, char *id);

/*--------------------------------------------------------------
函数名称：PLCP_Bind_Quit
函数功能：退出绑定
输入参数：buf - 数据指针 len - 数据长度
返回值：  1 - 成功 0 - 失败
备 注：
---------------------------------------------------------------*/
uint8_t PLCP_bindTableDelet(uint8_t se, char *aei, char *id);

void PLCP_bindTableClr(void);

/*--------------------------------------------------------------
函数名称：PLCP_bindTableRecover
函数功能：绑定从easy_flash读取数据
输入参数：无
返回值：  无
备 注：
---------------------------------------------------------------*/
fmc_state_enum APP_ReadBindParameter(void);
static fmc_state_enum APP_SaveBindParameter(void);
/*--------------------------------------------------------------
函数名称：PLCP_WigetEvent
函数功能：触发事件
输入参数：需要触发事件的 SE, AEI, ID, 数值，其中 数值 如果为0xffff
    则上报事件的 data 项将使用 event table 中的 data 数据
返回值：
备 注：此函数用于在事件发生时调用，如果传入的参数与 event table 不比配
则此函数不起作用；如果匹配，则此事件将上报给 cco
---------------------------------------------------------------*/
uint8_t PLCP_WigetEvent(uint8_t eventSE, char *eventAEI, char *eventID, uint32_t eventValue);

U16 PLCP_CountBindDataInAddBindParam(uint8_t *bindParam, uint8_t bindParamLen);
uint8_t PLCP_GetBindDataInAddBindParam(uint8_t *bindParam, uint8_t bindParamLen, uint8_t index,
                                       uint8_t *eventSE, char *eventAEI, char *eventID, char *msg);
uint16_t PLCP_GetBindData(uint8_t *bindParam, uint8_t bindParamLen, uint8_t *bindData);
U16 PLCP_CountEventInDelBindParam(uint8_t *bindParam, uint8_t bindParamLen);
uint8_t PLCP_GetEventInDelBindParam(uint8_t *bindParam, uint8_t bindParamLen, uint8_t index, uint8_t *eventSE, char *eventAEI, char *eventID);
void PLCP_bindTableDeletByAEI(uint8_t eventSE, char *eventAEI);
uint8_t PLCP_CountKJInDelBindParam(uint8_t *bindParam, uint8_t bindParamLen);
uint8_t PLCP_GetKJInDelBindParam(uint8_t *bindParam, uint8_t bindParamLen, uint8_t index, uint8_t *eventSE, char *eventAEI);
uint8_t PLCP_WigetEventWithType(uint8_t eventSE, char *eventAEI, char *eventID, U32 eventValue, char *eventType);
#endif
