/********************************************************************************
 * (c) Copyright 2017-2020, LME, All Rights Reserved��
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF LME, INC��
 * The copyright notice above does not evidence any actual or intended
 * publication of such source code��
 *
 * FileName    : Common.h
 * Author      :
 * Date        : 2023-09-13
 * Version     : 1.0
 * Description :
 *             :
 * Others      :
 * ModifyRecord:
 *
 ********************************************************************************/
#ifndef __MSEPROCESS_H_
#define __MSEPROCESS_H_
#include "../../Source/plcp_common/Inc/Uapps.h"
/**********************************������� ��ʼ***********************************/
#define MSE_DEVICE_ON               "/_on"  // ��
#define MSE_DEVICE_OFF              "/_off" // �ر�

#define MSE_DEVICE_OPEN             "/_open"
#define MSE_DEVICE_CLOSE            "/_close"
#define MSE_DEVICE_STOP             "/_stop"

#define MSE_DEVICE_SET_CONFIG       "/_config" // ��������

#define MSE_DEVICE_GROUP_JOIN       "/g/_join" // �豸�����������
#define MSE_DEVICE_GROUP_QUIT       "/g/_quit" // �豸�˳��������
#define MSE_DEVICE_GROUP_LIST       "/g/_list" // �豸���Ѽ�������б�������

#define MSE_DEVICE_SCENE_JOIN       "/s/_join" // �豸���볡��������
#define MSE_DEVICE_SCENE_QUIT       "/s/_quit" // �豸�˳�����������
#define MSE_DEVICE_SCENE_LIST       "/s/_list" // �豸���Ѽ��볡���б�������
#define MSE_DEVICE_SCENE_COPY       "/s/_copy" // �豸��������
#define MSE_DEVICE_SCENE_DELE       "/s/_del"

#define MSE_DEVICE_PLCP_FACTORY     "/_factory" // �豸�ָ��������÷�����
#define MSE_DEVICE_PLCP_RESET       "/_rst"     // �豸�ָ��������÷�����
#define MSE_DEVICE_PLCP_STATE       "/_state"   // ��ѯ״̬������

#define MSE_DEVICE_PLCP_STATE_ONOFF "/_state?_onoff" // ��ѯ����״̬������
#define MSE_DEVICE_PLCP_MCUVER      "/_mcuVer"       // ��ѯMCU�汾������

#define MSE_DEVICE_PLCP_BIND        "/_bind"

#define MSE_DEVICE_PLCP_MSET        "/_mSET" // 组网成功应答

/**********************************������� ����***********************************/
/**********************************ö�ٶ��� ��ʼ***********************************/
typedef enum {
    DEVICE_STATE_ON_E = 0,
    DEVICE_STATE_OFF_E,

    DEVICE_STATE_OPEN_E,
    DEVICE_STATE_CLOSE_E,
    DEVICE_STATE_STOP_E,

    DEVICE_STATE_SET_CONFIG_E,

    DEVICE_GROUP_JOIN_e,
    DEVICE_GROUP_QUIT_e,
    DEVICE_GROUP_LIST_e,

    DEVICE_SCENE_JOIN_e,
    DEVICE_SCENE_QUIT_e,
    DEVICE_SCENE_DELE_e,
    DEVICE_SCENE_LIST_e,
    DEVICE_SCENE_COPY_e,

    DEVICE_FACTORY_e,
    DEVICE_RESET_e,
    DEVICE_STATE_e,
    DEVICE_STATE_ONOFF_e,
    DEVICE_MCUVER_e,
    DEVICE_BIND_e,
    DEVICE_MSET_e,

} MSE_RESOURCE_SERVICE_e;

/**********************************ö�ٶ��� ��ʼ***********************************/
/**********************************�ṹ������ ��ʼ***********************************/
struct Function_Config {
    uint8_t (*Control_Open)(void);
    uint8_t (*Control_Close)(void);
    uint8_t (*Set_Config)(uint8_t *buf, uint8_t buf_len);
    void (*Device_Facory)(void);
    void (*Device_Reset)(uint8_t *buf, uint8_t len);

    uint8_t (*Device_Getstate)(uint8_t *buf, uint8_t type, uint16_t bits);
    void (*Device_Putstate)(uint8_t *buf, uint8_t buf_len);
    uint8_t (*Get_Config)(uint8_t *buf, uint8_t type, uint16_t bits);
    uint8_t (*Device_GetMCUver)(uint8_t *buf);
};

void APP_SendACK(UappsMessage *reqMsg, uint16_t payloadFlag, uapps_rw_buffer_t *scratch, uint8_t type, uint8_t repondCode);

void APP_UappsMsgProcessing(uint8_t *inputDataBuff, uint16_t inputDataBuffLen);

uint16_t Aps_UartMessage(UappsMessage *pMsg, uint8_t *sendBuff, uint16_t sendBuffMaxSize);

void CmdTest_MSE_POST_Status(uint8_t *GatewaymacAddr, uint8_t *LocalmacAddr, uint8_t *buf, uint8_t buflen);

void CmdTest_MSE_Load_Product(char *CompanyName, char *model, char *PID);

void CmdTest_MSE_Load_AEInfo(const char *ae_num, uint8_t quantity, char *BrandName, char *id, char *url);
uint8_t APP_SendRSL(char *rslStr, char *from, uint8_t *playload, uint8_t playloadLen);
void mseprecess_init(void);
void CmdTest_MSE_Factory(void);
void CmdTest_MSE_Apply_net(uint8_t type, uint8_t def);
/***********************************�������� ����************************************/

#endif
