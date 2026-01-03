#include "../../Source/plcp_device/plcp_user_api/PLCP_bind.h"
#include "../../Source/base/debug.h"
#include "../../Source/eventbus/eventbus.h"
#include "../../Source/plcp_device/APP_PublicAttribute.h"
#include "../../Source/plcp_device/MseProcess.h"
#include "../../Source/plcp_device/plcp_user_api/plcp_sdk_api.h"
#include "../../cJSON/cJSON.h"
#include "systick.h"

#define MAX_PAYLOAD_LEN 400
#define REPORT_TARGET_RES "/_e"

#define MAX_BIND_ITEMS 10 // 最大绑定项数
#define AEI_MAX_LEN 12    // 事件编号最大长度
typedef struct {
    char eventID[1 + AEI_MAX_LEN]; // 事件ID
    char rsl[32];                  // 响应命令(或绑定的操作)
    char data[32];                 // 绑定数据(如果有)
} BindItem;

#pragma pack(4)
typedef struct {
    uint8_t se;
    char aei[AEI_MAX_LEN + 1];
    char id[AEI_MAX_LEN + 1];
    char msg[MAX_BIND_MSG_LEN];
    uint16_t padding;
} bindDataType;
#pragma pack()

#if (MAX_BIND_TABLE_SIZE > 0)
// static void plcp_panel_event(event_type_e event, void *params);
static bindDataType gBindDataBase[MAX_BIND_TABLE_SIZE];

// void APP_Bindinit(void)
// {
//     app_eventbus_subscribe(plcp_panel_event);
// }

// 保存绑定信息到flash
static fmc_state_enum APP_SaveBindParameter(void)
{
    fmc_state_enum ret;

    ret = app_flash_program(FLASH_BIND_INFO_H_START_ADD, (uint32_t *)gBindDataBase, MAX_BIND_TABLE_SIZE * sizeof(bindDataType), true);
    return ret;
}

// 从flash里读取绑定信息
fmc_state_enum APP_ReadBindParameter(void)
{
    fmc_state_enum ret;

    ret = app_flash_read(FLASH_BIND_INFO_H_START_ADD, (uint32_t *)gBindDataBase, MAX_BIND_TABLE_SIZE * sizeof(bindDataType));
    if (ret != FMC_READY) {
        return ret;
    }
    for (uint8_t i = 0; i < MAX_BIND_TABLE_SIZE; i++) {
        if (gBindDataBase[i].se == 0) {
            gBindDataBase[i].se = 0xff;
        } else if (gBindDataBase[i].se != 0xFF) {
            APP_PRINTF("gBindDataBase[%d].se[%02X]\n", i, gBindDataBase[i].se);
            APP_PRINTF(".ae[%s].id[%s].msg[%s]\n", gBindDataBase[i].aei, gBindDataBase[i].id, gBindDataBase[i].msg);
        }
    }
    APP_PRINTF("\n");
    return ret;
}

// 写入绑定信息
uint8_t PLCP_bindTableWrite(uint8_t se, char *aei, char *id, char *msg)
{
    APP_PRINTF("PLCP_bindTableWrite  se:%d aei:%s id:%s msg:%s\n", se, aei, id, msg);
    uint8_t i;
    for (i = 0; i < MAX_BIND_TABLE_SIZE; i++) { // 第一轮循环:查找是否已经存在相同的绑定记录
        if (gBindDataBase[i].se == se && strcmp(gBindDataBase[i].aei, aei) == 0 && strcmp(gBindDataBase[i].id, id) == 0) {
            strcpy(gBindDataBase[i].msg, msg);
            APP_PRINTF("save1\n");
            if (APP_SaveBindParameter() != FMC_READY) {
                APP_PRINTF("APP_SaveBindParameter error\n");
            }
            return 1;
        }
    }
    for (i = 0; i < MAX_BIND_TABLE_SIZE; i++) { // 第二轮循环:如果没有找到,则找一个空位置写入新记录
        if (gBindDataBase[i].se == 0xff) {
            APP_PRINTF("save[%d]\n", i);
            gBindDataBase[i].se = se;
            strcpy(gBindDataBase[i].aei, aei);
            strcpy(gBindDataBase[i].id, id);
            strcpy(gBindDataBase[i].msg, msg);
            APP_PRINTF("save2\n");
            if (APP_SaveBindParameter() != FMC_READY) {
                APP_PRINTF("APP_SaveBindParameter error\n");
            }
            return 1;
        }
    }
    return 0;
}

// 读取绑定的消息
char *PLCP_bindTableRead(uint8_t se, char *aei, char *id)
{
    for (uint8_t i = 0; i < MAX_BIND_TABLE_SIZE; i++) {
#if 0
        APP_PRINTF("gBindDataBase[%d].se:%d  se:%d\n", i, gBindDataBase[i].se, se);
        APP_PRINTF("gBindDataBase[%d].aei:%.*s  aei:%.*s\n", i, 12, gBindDataBase[i].aei, 12, aei);
        APP_PRINTF("gBindDataBase[%d].id,:%.*s  id:%.*s\n", i, 12, gBindDataBase[i].id, 12, id);
#endif
        if (gBindDataBase[i].se == se && strcmp(gBindDataBase[i].aei, aei) == 0 && strcmp(gBindDataBase[i].id, id) == 0) {
            return (gBindDataBase[i].msg);
        }
    }
    return NULL;
}

char *PLCP_bindTableReadMsgByIndex(uint8_t index)
{
    return gBindDataBase[index].msg;
}

char *PLCP_bindTableReadIdByIndex(uint8_t index)
{
    return gBindDataBase[index].id;
}

// 根据 AEI 和 SE 读取绑定消息索引
uint8_t PLCP_bindTableReadByAEI(uint8_t se, char *aei, uint8_t startIndex)
{
    for (uint8_t i = startIndex; i < MAX_BIND_TABLE_SIZE; i++) {
        if (gBindDataBase[i].se == se && strcmp(gBindDataBase[i].aei, aei) == 0) {
            return i;
        }
    }
    return 0xFF;
}

// 退出绑定
uint8_t PLCP_bindTableDelet(uint8_t se, char *aei, char *id)
{
    uint8_t i;
    for (i = 0; i < MAX_BIND_TABLE_SIZE; i++) {
        if (gBindDataBase[i].se == se && strcmp(gBindDataBase[i].aei, aei) == 0 && strcmp(gBindDataBase[i].id, id) == 0) {
            gBindDataBase[i].se = 0xff;
        }
    }
    APP_SaveBindParameter();
    return 1;
}

// 清空绑定
void PLCP_bindTableClr(void)
{
    // for (uint8_t i = 0; i < MAX_BIND_TABLE_SIZE; i++) {
    //     gBindDataBase[i].se = 0xFF;
    // }
    app_flash_erase_page(FLASH_BIND_INFO_H_START_ADD);
    // APP_SaveBindParameter();
}

// 统计写入绑定命令的参数中有多少条绑定数据
uint16_t PLCP_CountBindDataInAddBindParam(uint8_t *bindParam, uint8_t bindParamLen)
{
    /************************************
    {
        "kj":"k1@se1",
        "ae":"0500@SE0.FFFFFFFFFFFF",
        "bind":[
            {
                "id":"_press",
                "rsi":"/0/_on",
                "data":""
            },
            {
                "id":"_release",
                "rsi":"/0/_on",
                "data":""
            }
        ]
    }
    {
        "kj": "s1@se1",
        "bind": [
            {
                "id": "_present",
                "rsl": "0700@SE0.FFFFFFFFFFFF/0/_on",	//注意 rsl 与 rsi 的区别，如果是 rsl 则没有 ae 字段
                "data": ""
            },
            {
                "id": "_disappear",
                "rsl": "0600@SE0.FFFFFFFFFFFF/0/_on",
                "data": ""
            }
        ]
    }
    -->
    bind -> 2
    ******************************/
    U16 ret = 0;
    U16 error = 0;
    U8 ae_include = 0;
    U8 rsl_include = 0;
    U8 rsi_include = 0;

    if (bindParam == 0 || bindParamLen == 0) {
        return 0;
    }

    cJSON *root = cJSON_Parse((char *)bindParam);
    if (root == NULL) {
        printf("bindParam is error json\n");
        return 0;
    }

    cJSON *kj = cJSON_GetObjectItem(root, "kj");
    if (kj != NULL && cJSON_IsString(kj) && strlen(kj->valuestring) > 0 && strlen(kj->valuestring) <= 32) {
        printf("kj: %s\n", kj->valuestring);
    } else {
        error++;
    }

    cJSON *ae = cJSON_GetObjectItem(root, "ae");
    if (ae != NULL && cJSON_IsString(ae) && strlen(ae->valuestring) > 0) {
        printf("ae: %s\n", ae->valuestring);
        ae_include = 1;
    } else {
        ae_include = 0;
    }

    cJSON *bind = cJSON_GetObjectItem(root, "bind");
    if (bind != NULL && cJSON_IsArray(bind)) {
        int array_size = cJSON_GetArraySize(bind);
        for (int i = 0; i < array_size; i++) {
            cJSON *item = cJSON_GetArrayItem(bind, i);
            if (item != NULL) {
                cJSON *id = cJSON_GetObjectItem(item, "id");
                if (id != NULL && cJSON_IsString(id) && strlen(id->valuestring) > 0 && strlen(id->valuestring) < AEI_MAX_LEN) {
                    // printf("id: %s\n", id->valuestring);
                } else {
                    error++;
                }

                cJSON *rsl = cJSON_GetObjectItem(item, "rsl");
                if (rsl != NULL && cJSON_IsString(rsl) && strlen(rsl->valuestring) > 0) {
                    // printf("rsl: %s\n", rsl->valuestring);
                    rsl_include = 1;
                }
                cJSON *rsi = cJSON_GetObjectItem(item, "rsi");
                if (rsi != NULL && cJSON_IsString(rsi) && strlen(rsi->valuestring) > 0) {
                    // printf("rsi: %s\n", rsi->valuestring);
                    rsi_include = 1;
                }
                if (rsl_include == rsi_include) {
                    error++;
                }
                if (rsl_include == ae_include) {
                    error++;
                }
                if (rsi_include != ae_include) {
                    error++;
                }

                cJSON *data = cJSON_GetObjectItem(item, "data");
                if (data != NULL && cJSON_IsString(data) && strlen(data->valuestring) > 0) {
                    // printf("data: %s\n", data->valuestring);
                } else {
                    // 可以没有data字段
                }

                cJSON *fun = cJSON_GetObjectItem(item, "fun");
                if (fun != NULL && cJSON_IsString(fun) && strlen(fun->valuestring) > 0) {
                    // printf("fun: %s\n", fun->valuestring);
                } else {
                    // 可以没有fun字段
                }
            } else {
                error++;
            }
        }
        ret = array_size;
    } else {
        error++;
    }

    if (error != 0) {
        ret = 0;
    }

    cJSON_Delete(root);
    // printf("find BindParam: %d in param\n", ret);
    return ret;
}

static uint8_t kjToEventSEAndAEI(char *kjStr, uint8_t *eventSE, char *eventAEI)
{
    char strTemp[32] = {0};
    if (strlen(kjStr) > 32) {
        return 0;
    }
    strcpy(strTemp, kjStr);
    char *p = strchr(strTemp, '@');
    if (p == NULL) {
        return 0;
    }
    *p = '\0';
    p++;
    strcpy(eventAEI, strTemp);

    while (!isNumber(p)) {
        p++;
        if (*p == '\0') {
            return 0;
        }
    }
    *eventSE = (uint16_t)atol(p);
    return 1;
}

// 从写入绑定命令的参数中读取第n条绑定数据
uint8_t PLCP_GetBindDataInAddBindParam(uint8_t *bindParam, uint8_t bindParamLen, uint8_t index, uint8_t *eventSE, char *eventAEI, char *eventID, char *msg)
{
    /************************************
    {
        "kj":"k1@se1",
        "ae":"0500@SE0.FFFFFFFFFFFF",
        "bind":[
            {
                "id":"_press",
                "rsi":"/0/_on",
                "data":""
            },
            {
                "id":"_release",
                "rsi":"/0/_on",
                "data":""
            }
        ]
    }
    -->
    event_se: 1; event_aei: k1; evet_id: _press;
    msg: {"rsl":"0500@SE0.FFFFFFFFFFFF/0/_on","data":""}

    event_se: 1; event_aei: k1; evet_id: _release;
    msg: {"rsl":"0500@SE0.FFFFFFFFFFFF/0/_on","data":""}
    ------------------------------------------------------
    {
        "kj": "s1@se1",
        "bind": [
            {
                "id": "_present",
                "rsl": "0700@SE0.FFFFFFFFFFFF/0/_on",	//注意 rsl 与 rsi 的区别，如果是 rsl 则没有 ae 字段
                "data": ""
            },
            {
                "id": "_disappear",
                "rsl": "0600@SE0.FFFFFFFFFFFF/0/_on",
                "data": ""
            }
        ]
    }
    -->
    event_se: 1; event_aei: s1; evet_id: _present;
    msg: {"rsl":"0700@SE0.FFFFFFFFFFFF/0/_on","data":""}
    event_se: 1; event_aei: s1; evet_id: _disappear;
    msg: {"rsl":"0600@SE0.FFFFFFFFFFFF/0/_on","data":""}
    ******************************/

#if 0
    cJSON *root = cJSON_Parse((char *)bindParam);
    cJSON *kj   = cJSON_GetObjectItem(root, "kj");
    cJSON *ae   = cJSON_GetObjectItem(root, "ae");
    cJSON *bind = cJSON_GetObjectItem(root, "bind");
    cJSON *item = cJSON_GetArrayItem(bind, index);
    cJSON *id   = cJSON_GetObjectItem(item, "id");
    cJSON *rsi  = cJSON_GetObjectItem(item, "rsi");
    cJSON *rsl  = cJSON_GetObjectItem(item, "rsl");
    cJSON *data = cJSON_GetObjectItem(item, "data");

    if (1 != kjToEventSEAndAEI(kj->valuestring, eventSE, eventAEI)) {
        cJSON_Delete(root);
        return 0;
    }
    strcpy(eventID, id->valuestring);
    printf("get event %s @ SE%d %s in param\n", eventAEI, *eventSE, eventID);

    cJSON *msgJson = cJSON_CreateObject();
    if (rsl == NULL) {
        char strTemp[UAPPS_MAX_OPTLEN + 1];
        memset(strTemp, 0, sizeof(strTemp));
        snprintf(strTemp, sizeof(strTemp), "%s%s", ae->valuestring, rsi->valuestring);
        cJSON_AddStringToObject(msgJson, "rsl", strTemp);
    } else {
        cJSON_AddStringToObject(msgJson, "rsl", rsl->valuestring);
    }
    if (data == NULL) {
        cJSON_AddStringToObject(msgJson, "data", "");
    } else {
        cJSON_AddStringToObject(msgJson, "data", data->valuestring);
    }
    char *msgStr = cJSON_PrintUnformatted(msgJson);

    if (strlen(msgStr) >= MAX_BIND_MSG_LEN) {
        cJSON_free(msgStr);
        cJSON_Delete(msgJson);
        cJSON_Delete(root);
        return 0;
    }

    strcpy(msg, msgStr);
    cJSON_free(msgStr);
    cJSON_Delete(msgJson);
    cJSON_Delete(root);

    printf("get msg %s in param\n", msg);
    return 1;
#else

    char *p = (char *)bindParam;
    char *kjStr = NULL;
    char *aeStr = NULL;

    // 解析 kj
    kjStr = strstr(p, "\"kj\"");
    if (!kjStr)
        return 0;
    kjStr = strchr(kjStr, ':');
    if (!kjStr)
        return 0;
    kjStr++;
    while (*kjStr == ' ' || *kjStr == '"')
        kjStr++;
    char kjBuf[UAPPS_MAX_OPTLEN] = {0};
    char *end = strpbrk(kjStr, "\"}");
    if (!end)
        return 0;
    strncpy(kjBuf, kjStr, end - kjStr);

    if (1 != kjToEventSEAndAEI(kjBuf, eventSE, eventAEI))
        return 0;

    // 解析 ae()可选)
    aeStr = strstr(p, "\"ae\"");

    char aeBuf[UAPPS_MAX_OPTLEN] = {0};
    if (aeStr) {
        aeStr = strchr(aeStr, ':');
        if (!aeStr)
            return 0;
        aeStr++;
        while (*aeStr == ' ' || *aeStr == '"')
            aeStr++;
        end = strpbrk(aeStr, "\"}");
        if (!end)
            return 0;
        strncpy(aeBuf, aeStr, end - aeStr);
    }

    // 找 bind 数组
    char *bindArr = strstr(p, "\"bind\"");
    if (!bindArr)
        return 0;
    bindArr = strchr(bindArr, '[');
    if (!bindArr)
        return 0;

    // 找到第 index 个对象
    char *item = bindArr + 1;
    for (uint8_t i = 0; i < index; i++) {
        item = strchr(item, '}');
        if (!item)
            return 0;
        item++;
        item = strchr(item, '{');
        if (!item)
            return 0;
    }

    // 解析 id
    char *idStr = strstr(item, "\"id\"");
    if (!idStr)
        return 0;
    idStr = strchr(idStr, ':');
    if (!idStr)
        return 0;
    idStr++;
    while (*idStr == ' ' || *idStr == '"')
        idStr++;
    end = strpbrk(idStr, "\"}");
    if (!end)
        return 0;
    strncpy(eventID, idStr, end - idStr);

    // 解析 rsi 或 rsl
    char rsiBuf[UAPPS_MAX_OPTLEN] = {0};
    char rslBuf[UAPPS_MAX_OPTLEN] = {0};
    char *rsiStr = strstr(item, "\"rsi\"");
    char *rslStr = strstr(item, "\"rsl\"");
    if (rslStr) {
        rslStr = strchr(rslStr, ':');
        if (!rslStr)
            return 0;
        rslStr++;
        while (*rslStr == ' ' || *rslStr == '"')
            rslStr++;
        end = strpbrk(rslStr, "\"}");
        if (!end)
            return 0;
        strncpy(rslBuf, rslStr, end - rslStr);
    } else if (rsiStr) {
        rsiStr = strchr(rsiStr, ':');
        if (!rsiStr)
            return 0;
        rsiStr++;
        while (*rsiStr == ' ' || *rsiStr == '"')
            rsiStr++;
        end = strpbrk(rsiStr, "\"}");
        if (!end)
            return 0;
        strncpy(rsiBuf, rsiStr, end - rsiStr);
    }

    // 解析 data
    char dataBuf[UAPPS_MAX_OPTLEN] = {0};
    char *dataStr = strstr(item, "\"data\"");
    if (dataStr) {
        dataStr = strchr(dataStr, ':');
        if (!dataStr)
            return 0;
        dataStr++;
        while (*dataStr == ' ' || *dataStr == '"')
            dataStr++;
        end = strpbrk(dataStr, "\"}");
        if (!end)
            return 0;
        strncpy(dataBuf, dataStr, end - dataStr);
    }

    // 生成 msg
    if (rslBuf[0]) {
        snprintf(msg, MAX_BIND_MSG_LEN, "{\"rsl\":\"%s\",\"data\":\"%s\"}", rslBuf, dataBuf);
    } else {
        snprintf(msg, MAX_BIND_MSG_LEN, "{\"rsl\":\"%s%s\",\"data\":\"%s\"}", aeBuf, rsiBuf, dataBuf);
    }

    // printf("get event %s @ SE%d %s in param\n", eventAEI, *eventSE, eventID);
    // printf("get msg %s in param\n", msg);

    return 1;
#endif
}

static uint8_t bindMsgToBindJsonItem(cJSON *bindItem, char *msg, char *id)
{
    if (bindItem == NULL) {
        return 0;
    }

    cJSON *msgJson = cJSON_Parse(msg);
    if (msgJson == NULL) {
        cJSON_AddStringToObject(bindItem, "rsl", "");
        cJSON_AddStringToObject(bindItem, "data", "");
    } else {
        cJSON *rsl = cJSON_GetObjectItem(msgJson, "rsl");
        cJSON *data = cJSON_GetObjectItem(msgJson, "data");
        cJSON_AddStringToObject(bindItem, "rsl", rsl->valuestring);
        cJSON_AddStringToObject(bindItem, "data", data->valuestring);
    }

    cJSON_AddStringToObject(bindItem, "id", id);

    cJSON_Delete(msgJson);
    return 1;
}

uint16_t PLCP_GetBindData(uint8_t *bindParam, uint8_t bindParamLen, uint8_t *bindData)
{
    /*********************************************
    {
      "kj":"k1@se1",//要查询的控件名称
      "id":"_press"//要查询的事件名称
    }
    -->
    event_se: 1; event_aei: k1; evet_id: _press;
    -->
    msg: {"rsl":"0500@SE0.FFFFFFFFFFFF/0/_on","data":""}
    -->
    {
        "kj":"k1@se1",
        "bind":[
            {
                "id":"_press",
                "rsl":"0500@SE0.FFFFFFFFFFFF/0/_on",
                "data":""
            }
        ]
    }

    {
      "kj":"k1@se1",//要查询的控件名称
    }
    -->
    event_se: 1; event_aei: k1; evet_id: ALL;
    -->
    msg: {"rsl":"0500@SE0.FFFFFFFFFFFF/0/_on","data":""}
    ...
    -->
    {
        "kj":"k1@se1",
        "bind":[
            {
                "id":"_press",
                "rsl":"0500@SE0.FFFFFFFFFFFF/0/_on",
                "data":""
            },
            {
                "id":"_press2",
                "rsl":"0500@SE0.FFFFFFFFFFFF/0/_on",
                "data":""
            }...
        ]
    }
    *********************************************/
#if 0
    APP_PRINTF("PLCP_GetBindData\n");
    cJSON *paramJson    = NULL;
    cJSON *bindDataJson = NULL;
    cJSON *bind         = NULL;
    cJSON *bindItem     = NULL;
    cJSON *kj           = NULL;
    cJSON *id           = NULL;
    char *tempStr       = NULL;

    uint8_t eventSE;
    char eventAEI[AEI_MAX_LEN];
    char *msg;
    uint16_t len = 0;

    if (bindParam == 0 || bindParamLen == 0 || bindData == 0) {
        goto getBindDataEnd;
    }

    paramJson = cJSON_Parse((char *)bindParam);
    kj        = cJSON_GetObjectItem(paramJson, "kj");
    id        = cJSON_GetObjectItem(paramJson, "id");
    memset(eventAEI, 0, AEI_MAX_LEN);
    if (1 != kjToEventSEAndAEI(kj->valuestring, &eventSE, eventAEI)) {
        goto getBindDataEnd;
    }
    // uint8_t index;
    char eventID[AEI_MAX_LEN];
    bind = cJSON_CreateArray();
    if (id != NULL) {
        memset(eventID, 0, AEI_MAX_LEN);
        strcpy(eventID, id->valuestring);
        bindItem = cJSON_CreateObject();
        msg      = PLCP_bindTableRead(eventSE, eventAEI, eventID);
        bindMsgToBindJsonItem(bindItem, msg, eventID);
        cJSON_AddItemToArray(bind, cJSON_Duplicate(bindItem, TRUE));
        cJSON_Delete(bindItem);
    }
    // else
    // {
    // 	index = 0;
    // 	while (1)
    // 	{
    // 		memset(eventID, 0, AEI_MAX_LEN);
    // 		index = PLCP_eventTableGetEventIdWithStartIndex(eventSE, eventAEI, eventID, index);
    // 		if(index == 0xff)
    // 		{
    // 			break;
    // 		}

    // 		bindItem = cJSON_CreateObject();
    // 		msg = PLCP_bindTableRead(index);
    // 		bindMsgToBindJsonItem(bindItem, msg, eventID);
    // 		cJSON_AddItemToArray(bind, cJSON_Duplicate(bindItem, TRUE));
    // 		cJSON_Delete(bindItem);
    // 		index++;
    // 	}
    // }

    bindDataJson = cJSON_CreateObject();
    cJSON_AddStringToObject(bindDataJson, "kj", kj->valuestring);
    cJSON_AddItemToObject(bindDataJson, "bind", bind);
    tempStr = cJSON_PrintUnformatted(bindDataJson);
    len     = strlen(tempStr);
    if (len > MAX_PAYLOAD_LEN) {
        len = 0;
        goto getBindDataEnd;
    }

    strcpy((char *)bindData, tempStr);
    APP_PRINTF("bindData:%s\n", bindData);
getBindDataEnd:
    cJSON_free(tempStr);
    cJSON_Delete(paramJson);
    cJSON_Delete(bindDataJson);
    cJSON_Delete(bind);
    return len;
#else
    uint8_t eventSE = 0;
    char eventAEI[AEI_MAX_LEN] = {0};
    char eventID[AEI_MAX_LEN] = {0};
    uint16_t len = 0;
    int hasId = 0;

    if (bindParam == NULL || bindParamLen == 0 || bindData == NULL) {
        APP_PRINTF("Invalid input param\n");
        return 0;
    }

    char *paramStr = (char *)bindParam;
    APP_PRINTF("Raw bindParam: %s\n", paramStr);

    char *kjStr = strstr(paramStr, "\"kj\":\"");
    char *idStr = strstr(paramStr, "\"id\":\"");

    if (kjStr == NULL) {
        APP_PRINTF("Missing 'kj' field\n");
        return 0;
    }

    // 提取 kj
    kjStr += 6;
    char *kjEnd = strchr(kjStr, '"');
    if (kjEnd == NULL)
        return 0;
    size_t kjLen = kjEnd - kjStr;
    if (kjLen >= AEI_MAX_LEN)
        kjLen = AEI_MAX_LEN - 1;
    char kj[AEI_MAX_LEN] = {0};
    strncpy(kj, kjStr, kjLen);

    // 提取 id (如果有)
    if (idStr != NULL) {
        idStr += 6;
        char *idEnd = strchr(idStr, '"');
        if (idEnd != NULL) {
            size_t idLen = idEnd - idStr;
            if (idLen >= AEI_MAX_LEN)
                idLen = AEI_MAX_LEN - 1;
            strncpy(eventID, idStr, idLen);
            hasId = 1;
        }
    }

    // 解析 kj -> eventSE + eventAEI
    if (kjToEventSEAndAEI(kj, &eventSE, eventAEI) != 1) {
        APP_PRINTF("kjToEventSEAndAEI() failed for %s\n", kj);
        return 0;
    }
    char *p = (char *)bindData;
    int remain = MAX_PAYLOAD_LEN;

    if (hasId) { // 有 id (查询指定事件的绑定信息)
        char *msg = PLCP_bindTableRead(eventSE, eventAEI, eventID);
        APP_PRINTF("msg:%s\n", msg);
        if (msg) {
            char *p = (char *)bindData;
            remain = MAX_PAYLOAD_LEN;

            int n = snprintf(p, remain, "{\"kj\":\"%s\",\"bind\":[", kj);
            p += n;
            remain -= n;

            size_t msgLen = strlen(msg);
            if (msg[0] == '{' && msg[msgLen - 1] == '}') {
                msg[msgLen - 1] = '\0';
                n = snprintf(p, remain, "%s,\"id\":\"%s\"}]}", msg, eventID);
            } else {
                n = snprintf(p, remain, "{\"rsl\":%s,\"id\":\"%s\"}]}", msg, eventID);
            }
            p += n;
            remain -= n;

            len = p - (char *)bindData;
        } else {
            len = snprintf((char *)bindData, MAX_PAYLOAD_LEN, "{\"kj\":\"%s\",\"bind\":[]}", kj);
        }
    } else { // 无 id (查询多个绑定信息)
        uint8_t index = 0;

        // JSON 开头
        int n = snprintf(p, remain, "{\"kj\":\"%s\",\"bind\":[", kj);
        if (n < 0 || n >= remain)
            return 0;
        p += n;
        remain -= n;

        int first = 1;
        while (1) {
            index = PLCP_bindTableReadByAEI(eventSE, eventAEI, index);
            if (index == 0xFF)
                break;

            char *msg = PLCP_bindTableReadMsgByIndex(index); // {"rsl":"...","data":""}
            char *id = PLCP_bindTableReadIdByIndex(index);

            if (!msg || !id) {
                index++;
                continue;
            }

            if (!first) {
                if (remain <= 1)
                    break;
                *p++ = ',';
                remain--;
            }
            first = 0;

            // 拼接单条记录：把 msg 尾部 } 去掉，再加 ,"id":"xxx"}
            size_t msgLen = strlen(msg);
            if (msg[0] == '{' && msg[msgLen - 1] == '}') {
                msg[msgLen - 1] = '\0';
            }

            n = snprintf(p, remain, "%s,\"id\":\"%s\"}", msg, id);
            if (n < 0 || n >= remain)
                break;
            p += n;
            remain -= n;

            index++;
        }
        // JSON 结束
        if (remain >= 2) {
            *p++ = ']';
            *p++ = '}';
        }

        len = p - (char *)bindData;
    }
    return len;
#endif
}

// 统计删除绑定命令的参数中有多少条事件的回调函数
uint16_t PLCP_CountEventInDelBindParam(uint8_t *bindParam, uint8_t bindParamLen)
{
    /************************************
    {
        "kj": "k1@se1",
        "id": ["_press", "_release"]
    }
    -->
    id -> 2
    ******************************/
    uint16_t ret = 0;
    uint16_t error = 0;

    if (bindParam == 0 || bindParamLen == 0) {
        return 0;
    }

    cJSON *root = cJSON_Parse((char *)bindParam);
    if (root == NULL) {
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        return 0;
    }

    cJSON *kj = cJSON_GetObjectItem(root, "kj");
    if (kj != NULL && cJSON_IsString(kj) && strlen(kj->valuestring) > 0) {
        printf("kj: %s\n", kj->valuestring);
    } else {
        error++;
    }

    cJSON *id = cJSON_GetObjectItem(root, "id");
    if (id != NULL && cJSON_IsArray(id)) {
        int array_size = cJSON_GetArraySize(id);
        for (int i = 0; i < array_size; i++) {
            cJSON *item = cJSON_GetArrayItem(id, i);
            if (item != NULL) {
                printf("id %d: %s ", i, cJSON_GetStringValue(item));
            } else {
                error++;
            }
        }
        ret = array_size;
    } else {
        cJSON_Delete(root);
        return 0xff;
    }

    if (error != 0) {
        ret = 0;
    }

    cJSON_Delete(root);
    return ret;
}

// 从删除绑定命令的参数中读取第n条事件的回调函数
uint8_t PLCP_GetEventInDelBindParam(uint8_t *bindParam, uint8_t bindParamLen, uint8_t index, uint8_t *eventSE, char *eventAEI, char *eventID)
{
    /************************************
    {
        "kj": "k1@se1",
        "id": ["_press"]
    }
    -->
    event_se: 1; event_aei: k1; evet_id: _press;
    ******************************/
    cJSON *root = cJSON_Parse((char *)bindParam);
    cJSON *kj = cJSON_GetObjectItem(root, "kj");
    cJSON *id = cJSON_GetObjectItem(root, "id");
    cJSON *item = cJSON_GetArrayItem(id, index);

    if (1 != kjToEventSEAndAEI(kj->valuestring, eventSE, eventAEI)) {
        cJSON_Delete(root);
        return 0;
    }
    strcpy(eventID, item->valuestring);

    cJSON_Delete(root);
    return 1;
}

uint8_t PLCP_CountKJInDelBindParam(uint8_t *bindParam, uint8_t bindParamLen)
{
    /************************************
    {
        "kj": "k1@se1"
    }
    -->
    count -> 1

    {
        "kj": ["k1@se1", "k2@se1"]
    }
    -->
    count -> 2
    ******************************/
    if (bindParam == 0 || bindParamLen == 0) {
        return 0;
    }

    cJSON *root = cJSON_Parse((char *)bindParam);
    if (root == NULL) {
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        return 0;
    }

    cJSON *kj = cJSON_GetObjectItem(root, "kj");
    if (kj == NULL) {
        printf("kj info error\n");
        cJSON_Delete(root);
        return 0;
    }

    if (cJSON_IsArray(kj)) {
        int array_size = cJSON_GetArraySize(kj);
        for (int i = 0; i < array_size; i++) {
            cJSON *item = cJSON_GetArrayItem(kj, i);
            if (item != NULL) {
                printf("kj %d: %s\n", i, cJSON_GetStringValue(item));
            } else {
                printf("kj item error\n");
                cJSON_Delete(root);
                return 0;
            }
        }
        cJSON_Delete(root);
        return array_size;
    } else {
        cJSON_Delete(root);
        return 1;
    }
}

uint8_t PLCP_GetKJInDelBindParam(uint8_t *bindParam, uint8_t bindParamLen, uint8_t index, uint8_t *eventSE, char *eventAEI)
{
    cJSON *root = cJSON_Parse((char *)bindParam);
    cJSON *kj = cJSON_GetObjectItem(root, "kj");

    if (cJSON_IsArray(kj)) {
        cJSON *item = cJSON_GetArrayItem(kj, index);
        if (item == NULL) {
            cJSON_Delete(root);
            return 0;
        }

        if (1 != kjToEventSEAndAEI(item->valuestring, eventSE, eventAEI)) {
            cJSON_Delete(root);
            return 0;
        }
    } else {
        if (index != 0) {
            cJSON_Delete(root);
            return 0;
        }

        if (1 != kjToEventSEAndAEI(kj->valuestring, eventSE, eventAEI)) {
            cJSON_Delete(root);
            return 0;
        }
    }

    return 1;
}

void PLCP_bindTableDeletByAEI(uint8_t eventSE, char *eventAEI)
{
    uint8_t i;
    for (i = 0; i < MAX_BIND_TABLE_SIZE; i++) {
        if (gBindDataBase[i].se == eventSE && strcmp(gBindDataBase[i].aei, eventAEI) == 0) {
            gBindDataBase[i].se = 0xff;
        }
    }

    APP_SaveBindParameter();
}

/*****************************************************************/
/*****************************************************************/
uint8_t PLCP_sendBindMsg(uint8_t se, char *aei, char *id, uint8_t *data, uint8_t dataLen)
{
    /***********************************************************
    msg: {"rsl":"0500@SE0.FFFFFFFFFFFF/0/_on","data":""}
    ***********************************************************/
#if 0
    uint8_t playload[48] = {0}; // 此处 playloud 不会超过 48 byte
    char *msg;
    msg = PLCP_bindTableRead(se, aei, id);
    if (msg == NULL) {
        return 0;
    }
    cJSON *msgJson = cJSON_Parse(msg);
    if (msgJson == NULL) {
        return 0;
    }
    cJSON *rslJson  = cJSON_GetObjectItem(msgJson, "rsl");
    cJSON *dataJosn = cJSON_GetObjectItem(msgJson, "data");
    if (dataLen != 0) {
        APP_SendRSL(rslJson->valuestring, NULL, data, dataLen);
    } else {
        dataLen = APP_StrToHex((uint8_t *)dataJosn->valuestring, strlen(dataJosn->valuestring), playload);
        APP_SendRSL(rslJson->valuestring, NULL, playload, dataLen);
        APP_PRINTF_BUF("PLCP_sendBindMsg", playload, dataLen);
    }

    cJSON_Delete(msgJson);

    return 1;
#else
    static uint8_t playload[48] = {0};
    memset(playload, 0, sizeof(playload));
    char *msg = PLCP_bindTableRead(se, aei, id);
    if (msg == NULL) {
        APP_PRINTF("msg is NULL\n");
        return 0;
    }
    char *rslStart = strstr(msg, "\"rsl\":\"");
    if (rslStart == NULL) {
        APP_PRINTF("rslStart is NULL\n");
        return 0;
    }

    rslStart += strlen("\"rsl\":\"");
    char *rslEnd = strchr(rslStart, '"');
    if (rslEnd == NULL) {
        APP_PRINTF("rslEnd is NULL\n");
        return 0;
    }

    char rslBuf[64] = {0};
    memcpy(rslBuf, rslStart, rslEnd - rslStart);

    char *dataStart = strstr(msg, "\"data\":\"");
    if (dataStart == NULL) {
        APP_PRINTF("dataStart is NULL\n");
        return 0;
    }

    dataStart += strlen("\"data\":\"");
    char *dataEnd = strchr(dataStart, '"');
    if (dataEnd == NULL) {
        APP_PRINTF("dataEnd is NULL\n");
        return 0;
    }

    static char dataBuf[64] = {0};
    memcpy(dataBuf, dataStart, dataEnd - dataStart);

    if (dataLen != 0) {
        // 数据由外部传入
        APP_SendRSL(rslBuf, NULL, data, dataLen);
    } else {
        // data 字段是 hex 字符串，比如 "01020304"
        dataLen = APP_StrToHex((uint8_t *)dataBuf, strlen(dataBuf), playload);
        APP_SendRSL(rslBuf, NULL, playload, dataLen);
    }
    return 1;
#endif
}

uint8_t PLCP_eventReport(uint8_t eventSE, char *eventAEI, char *eventID, U32 data, char *type, char *targetRES)
{
    /***********************************************************
    event_se: 1; event_aei: k1; evet_id: _close;
    -->
    {
        "name":"Switch"
        "ael":"k1@SE1",
        "type":"switch"
        "event": {
            "id": "_close",
            "name": "ON",
            "data": "1",
            "type": "swith"
        }
    }
    ***********************************************************/

    static char tempSrt[UAPPS_MAX_OPTLEN] = {0};
    static char from[UAPPS_MAX_OPTLEN] = {0};
    static uint8_t macAddr[6] = {0};
    static uint8_t playload[120] = {0}; // 上报的event长度预估不会超过 120 byte
    uint8_t playloadLen;

    memset(tempSrt, 0, sizeof(tempSrt));
    memset(from, 0, sizeof(from));
    memset(macAddr, 0, sizeof(macAddr));
    memset(playload, 0, sizeof(playload));

    if (plcp_sdk_api_get_cco_mac(macAddr) == 0 || plcp_sdk_api_get_did() == 0) {
        return 0;
    }

    cJSON *repotJson = cJSON_CreateObject();
    cJSON_AddStringToObject(repotJson, "id", eventID);

    // 根据 eventID 来构造 "name" 字段的值
    if (strcmp(eventID, "_open") == 0) {
        cJSON_AddStringToObject(repotJson, "name", "ON");
    } else if (strcmp(eventID, "_close") == 0) {
        cJSON_AddStringToObject(repotJson, "name", "OFF");
    } else if (strcmp(eventID, "_stop") == 0) {
        cJSON_AddStringToObject(repotJson, "name", "STOP");
    }

    if (data < 0xffffffff) {
        memset((uint8_t *)tempSrt, 0, UAPPS_MAX_OPTLEN);
        sprintf(tempSrt, "%d", data);
        cJSON_AddStringToObject(repotJson, "data", tempSrt);
    }
    if (type != NULL) {
        cJSON_AddStringToObject(repotJson, "type", type);
    }

    char *playloadStr = cJSON_PrintUnformatted(repotJson);
    strcpy((char *)playload, playloadStr);
    playloadLen = strlen(playloadStr);
    cJSON_free(playloadStr);
    cJSON_Delete(repotJson);

    snprintf(tempSrt, sizeof(tempSrt), "@SE200.%02x%02x%02x%02x%02x%02x%s",
             macAddr[0], macAddr[1], macAddr[2],
             macAddr[3], macAddr[4], macAddr[5],
             targetRES);

    if (plcp_sdk_api_get_self_mac(macAddr) == 0) {
        return 0;
    }
    snprintf(from, sizeof(from), "%s@SE%d.%02x%02x%02x%02x%02x%02x",
             eventAEI, eventSE,
             macAddr[0], macAddr[1], macAddr[2],
             macAddr[3], macAddr[4], macAddr[5]);
    return APP_SendRSL(tempSrt, from, playload, playloadLen);
}

/*****************************************************************/

// typedef struct
// {
//     uint8_t eventSE;
//     char *eventAEI;
//     char *eventID;
//     char *eventType;
//     uint32_t eventValue;
// } event_frame;
// static event_frame my_event_frame;

uint8_t PLCP_WigetEventWithType(uint8_t eventSE, char *eventAEI, char *eventID, U32 eventValue, char *eventType)
{
    // APP_PRINTF("eventID:%s\n", eventID);
    uint8_t ret_1, ret_2;

    // 把事件上报到网关
    // memset(&my_event_frame, 0, sizeof(event_frame));
    // my_event_frame.eventAEI = eventAEI;
    // my_event_frame.eventID = eventID;
    // my_event_frame.eventSE = eventSE;
    // my_event_frame.eventValue = eventValue;
    // my_event_frame.eventType = eventType;

    // app_eventbus_publish(PLCE_SEND_BIND_MSG, &my_event_frame);
    // app_eventbus_publish(PLCP_EVENT_REPORT, &my_event_frame);

    ret_2 = PLCP_sendBindMsg(eventSE, eventAEI, eventID, 0, 0);
    ret_1 = PLCP_eventReport(eventSE, eventAEI, eventID, eventValue, eventType, REPORT_TARGET_RES);

    return (ret_1 || ret_2);
}

// static void plcp_panel_event(event_type_e event, void *params)
// {
//     event_frame *temp = (event_frame *)params;
//     switch (event) {
//     case PLCP_EVENT_REPORT:
//         PLCP_eventReport(temp->eventSE, temp->eventAEI, temp->eventID, temp->eventValue, temp->eventType, REPORT_TARGET_RES);
//         break;
//     case PLCE_SEND_BIND_MSG:
//         PLCP_sendBindMsg(temp->eventSE, temp->eventAEI, temp->eventID, 0, 0);
//         break;
//     default:
//         break;
//     }
// }

#endif
