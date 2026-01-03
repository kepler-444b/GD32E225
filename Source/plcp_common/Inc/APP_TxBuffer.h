#include "../../Source/device/device_manager.h"
#if defined PLCP_DEVICE
#ifndef _APP_TX_BUFFER_H_
#define _APP_TX_BUFFER_H_
#include "lmexxx_conf.h"

#define aMaxNSDUSize                        450          // NSDU数据的最大长度 aMaxNSDUSize
#define APP_TX_BUFFER_SIZE                  aMaxNSDUSize // APP层发射缓存的大小
#define APP_TX_BUFFER_DATA_PARAM_ARRAY_SIZE 5            // APP层发射缓存中发送数据参数队列的大小

#define RING_BUFFER                         // 使用环形缓冲区

typedef struct
{
    u16 nsduLength; // 数据长度
    u8 *nsdu;       // 数据内容
} APP_DataTxTask;

/*--------------------------------1-----------------------------
函数名称：APP_TxBuffer_init
函数功能：APP层发射缓存的初始化。
输入参数：无
返回值：无
备 注：
---------------------------------------------------------------*/
void APP_TxBuffer_init(void);

/*--------------------------------------------------------------
函数名称：APP_TxBuffer_Add2FirstMsg
函数功能：APP层发送缓存添加一个元素，并且排到首个元素的位置。
输入参数：inputDataPointer -- 添加数据的指针
        inputDataLen -- 添加数据的长度
返回值： 0-添加失败，1-添加成功
备 注：
---------------------------------------------------------------*/
u8 APP_TxBuffer_Add2FirstMsg(APP_DataTxTask *inputDataParam);

/*--------------------------------2-----------------------------
函数名称：APP_TxBuffer_Add
函数功能：APP层发射缓存添加一个元素。
输入参数：inputDataPointer -- 添加数据的指针
        inputDataLen -- 添加数据的长度
返回值： 0-添加失败，1-添加成功
备 注：
---------------------------------------------------------------*/
u8 APP_TxBuffer_Add(APP_DataTxTask *inputDataParam);

/*--------------------------------3-----------------------------
函数名称：APP_TxBuffer_DeleteFirstMsg
函数功能：APP层发射缓存删除第一条消息。
输入参数：deleteDataLen -- 删除数据的长度
返回值：  0-删除失败，1-删除成功
备 注：
---------------------------------------------------------------*/
u8 APP_TxBuffer_DeleteFirstMsg(void);

/*--------------------------------4-----------------------------
函数名称：APP_TxBuffer_GetFirstMsgDataParameters
函数功能：APP层发射缓存获取第一条消息的参数
输出参数：firstMsgDataParam -  发射缓存第一条消息的参数
返回值： 0-失败，1-成功
备 注：
---------------------------------------------------------------*/
u8 APP_TxBuffer_GetFirstMsgDataParameters(APP_DataTxTask *firstMsgDataParam);

/*--------------------------------5-----------------------------
函数名称：APP_TxBuffer_GetFirstMsgPointer
函数功能：APP层发射缓存获取第一条消息的消息缓存指针
输入参数：无
返回值：APP层发射缓存获取第一条消息的消息缓存指针
备 注：
---------------------------------------------------------------*/
u8 *APP_TxBuffer_GetFirstMsgPointer(void);

/*--------------------------------6-----------------------------
函数名称：APP_TxBuffer_IsFull
函数功能：APP层发射缓存是否满。
输入参数：无
返回值：0-否，1-是
备 注：
---------------------------------------------------------------*/
u8 APP_TxBuffer_IsFull(void);

/*--------------------------------7-----------------------------
函数名称：APP_TxBuffer_IsEmpty
函数功能：APP层发射缓存是否空。
输入参数：无
返回值：0-否，1-是
备 注：
---------------------------------------------------------------*/
u8 APP_TxBuffer_IsEmpty(void);

/*--------------------------------8-----------------------------
函数名称：APP_TxBuffer_CanAddNewMsg
函数功能：APP层发射缓存是否添加新消息。
输入参数：inputDataLen -- 输入数据的长度
返回值：0-否，1-是
备 注：
---------------------------------------------------------------*/
u8 APP_TxBuffer_CanAddNewMsg(void);

/*--------------------------------9-----------------------------
函数名称：APP_TxBuffer_GetTxTaskNum
函数功能：APP层获取TxBuffer
输入参数：无
返回值：当前索引值
备 注：
---------------------------------------------------------------*/
u16 APP_TxBuffer_GetTxTaskNum(void);

#endif
#endif