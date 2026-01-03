#include "../../Source/device/device_manager.h"
#if defined PLCP_DEVICE
#include "../../Source/cJSON/cJSON.h"
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "../base/debug.h"
#include "../timer/timer.h"
#include "../usart/usart.h"
#include "APP_PublicAttribute.h"
#include "MseProcess.h"
#include "gd32e23x.h"
#include "systick.h"
#include <stdint.h>

#define FORMAT_VERSION "1.0"
#define NODE_FORMAT_VERSION "1.0"
#define PRODUCT_FORMAT_VERSION "1.0"
#define VER_FORMAT_VERSION "1.0"

#define SOFTWARE_VERSION "1.4/2023.08.30"
#define APS_VERSION "1.0/2023.08.25"
#define NWK_VERSION "1.1/2023.08.18"

#define CONTYPE "PLBUS"
#define PHASE_TYPE "1"
#define CHIP_TYPE "lme4015b"

const uint32_t crc32tab[256] =
    {
        0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
        0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
        0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
        0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
        0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
        0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
        0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
        0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
        0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
        0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
        0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
        0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
        0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
        0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
        0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
        0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
        0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
        0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
        0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
        0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
        0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
        0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
        0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
        0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
        0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
        0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
        0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
        0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
        0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
        0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
        0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
        0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
        0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
        0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
        0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
        0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
        0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
        0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
        0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
        0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
        0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
        0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
        0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
        0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
        0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
        0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
        0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
        0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
        0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
        0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
        0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
        0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
        0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
        0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
        0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
        0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
        0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
        0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
        0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
        0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
        0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
        0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
        0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
        0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL};

/*-------------------------------------------------------------
函数名称：APP_CalculateCRC32
函数功能：计算CRC32函数
输入参数：buf -计算的数据内容
                           bufLen - 计算的数据长度
返回值：     CRC32的校验值
备 注：
---------------------------------------------------------------*/
U32 APP_CalculateCRC32(uint8_t *buf, u16 bufLen)

{
    U32 i = 0;
    U32 crc = 0xFFFFFFFF;

    for (i = 0; i < bufLen; i++)
        crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);

    return crc ^ 0xFFFFFFFF;
}

/*-------------------------------------------------------------
函数名称：Add_device_information_Description
函数功能：添加信息描述符
输入参数：description -待发送缓存，type-描述符特征，datelen-数据长度， Str-设备信息数据
返回值：
备 注：
---------------------------------------------------------------*/
#if 1
void Add_device_information_Description(uint8_t *description, uint8_t type, uint8_t *date, u16 datelen)
{
    //	03 06 00 89 00 00 00 00 00 00 00 89 00 00 00 12 34 00 00
    U32 CRC32 = 0;

    description[0] = 0x03;                      // 默认03类型
    description[1] = type;                      // 0x04-AE数据,0x06 - product产品信息
    description[2] = 0x00;                      // 帧属性，0x00起始帧，0x01中间帧，0x02结束帧
    description[3] = (uint8_t)(datelen & 0xff); // 文件大小,4byte
    description[4] = (uint8_t)(datelen >> 8);
    description[5] = 0x00;
    description[6] = 0x00;
    description[7] = 0x00; // 数据偏移位置 4byte
    description[8] = 0x00;

    description[9] = 0x00;
    description[10] = 0x00;
    description[11] = (uint8_t)(datelen & 0xff); // 每帧数据大小 2byte
    description[12] = (uint8_t)(datelen >> 8);
    CRC32 = APP_CalculateCRC32(date, datelen);
    memcpy(&description[13], &CRC32, 4); // 段数据校验 4byte
    description[17] = 0x00;              // 设备文件对应的AEI号或port号 2byte
    description[18] = 0x00;
}

void Add_device_information_Description2(uint8_t *description, uint8_t frameType, u16 offset, u16 fileSize, uint8_t fileType, uint8_t *date, u16 datelen)
{
    //	03 06 00 89 00 00 00 00 00 00 00 89 00 00 00 12 34 00 00
    static uint32_t CRC32;
    CRC32 = 0;

    description[0] = 0x03;                       // 默认03类型
    description[1] = fileType;                   // 0x04-AE数据,0x06 - product产品信息
    description[2] = frameType;                  // 帧属性，0x00起始帧，0x01中间帧，0x02结束帧
    description[3] = (uint8_t)(fileSize & 0xff); // 文件大小,4byte
    description[4] = (uint8_t)(fileSize >> 8);
    description[5] = 0x00;
    description[6] = 0x00;
    description[7] = (uint8_t)(offset & 0xff); // 文件大小,4byte
    description[8] = (uint8_t)(offset >> 8);

    description[9] = 0x00;
    description[10] = 0x00;
    description[11] = (uint8_t)(datelen & 0xff); // 每帧数据大小 2byte
    description[12] = (uint8_t)(datelen >> 8);
    CRC32 = APP_CalculateCRC32(date, datelen);
    memcpy(&description[13], &CRC32, 4); // 段数据校验 4byte
    description[17] = 0x00;              // 设备文件对应的AEI号或port号 2byte
    description[18] = 0x00;
}
#endif
/*--------------------------------------------------------------
函数名称：CmdTest_MSE_Load_Product
函数功能：构造可写入plc模组产品信息的接口函数
输入参数：CompanyName-公司名；model-设备类型；PID-pid
返回值：   无
备 注：

{
  "product": {
    "ver": "产品格式版本",
    "name": "CompanyName",
    "model": "model",
    "pid": "PID",
    "license": "1234",
    "vendor": {
      "name": "Name"
    }
  }
}
---------------------------------------------------------------*/
static UappsMessage testCoap;
static char rsi[50];
static uint8_t uartSendBuff[256];
static u16 uartSendBuffLen = 0;
static uint8_t productInit[256];
void PLCP_Load_Product(char *product)
{
    memset(rsi, 0, sizeof(rsi));
    memset(uartSendBuff, 0, sizeof(uartSendBuff));
    memset(&testCoap, 0, sizeof(UappsMessage));
    memset(productInit, 0, sizeof(productInit));

    printf("enter Load_Product\n");

    sprintf(rsi, "@SE1./_load");

    char *pJsonStr = product;

    Add_device_information_Description2(productInit, 0, 0, strlen(pJsonStr), 0x06, (uint8_t *)pJsonStr, strlen(pJsonStr));
    memcpy(&productInit[19], (uint8_t *)pJsonStr, strlen(pJsonStr));

    UappsPutData(&testCoap, productInit, strlen(pJsonStr) + 19, UAPPS_FMT_OCTETS, 0); // 加入载荷

    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);
    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_Load_Product;

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, sizeof(uartSendBuff));
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}

void CmdTest_MSE_Load_Product(char *CompanyName, char *model, char *PID)
{
#if 1

    memset(rsi, 0, sizeof(rsi));
    memset(uartSendBuff, 0, sizeof(uartSendBuff));
    memset(&testCoap, 0, sizeof(UappsMessage));
    memset(productInit, 0, sizeof(productInit));

    printf("enter Load_Product\n");

    sprintf(rsi, "@SE1./_load");
    cJSON *product = cJSON_CreateObject();
    cJSON *root = cJSON_CreateObject();
    cJSON *vendor = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "ver", PRODUCT_FORMAT_VERSION);
    cJSON_AddStringToObject(root, "name", CompanyName);
    cJSON_AddStringToObject(root, "model", model);
    cJSON_AddStringToObject(root, "pid", PID);
    cJSON_AddStringToObject(root, "license", "1234");
    cJSON_AddStringToObject(vendor, "name", "Name");
    cJSON_AddItemToObject(product, "product", root);
    cJSON_AddItemToObject(root, "vendor", vendor);

    char *pJsonStr = cJSON_PrintUnformatted(product);

    Add_device_information_Description(productInit, 0x06, (uint8_t *)pJsonStr, strlen(pJsonStr));
    memcpy(&productInit[19], (uint8_t *)pJsonStr, strlen(pJsonStr));

    UappsPutData(&testCoap, productInit, strlen(pJsonStr) + 19, UAPPS_FMT_OCTETS, 0); // 加入载荷

    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);
    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_Load_Product;

    cJSON_Delete(product);
    cJSON_free(pJsonStr);

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, sizeof(uartSendBuff));
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
    // app_usart_tx_buf(uartSendBuff, uartSendBuffLen, USART0);
#else
    UappsMessage testCoap;
    char rsi[50];
    uint8_t uartSendBuff[256];
    u16 uartSendBuffLen = 0;
    uint8_t productInit[250];
    memset(rsi, 0, sizeof(rsi));
    memset(uartSendBuff, 0, sizeof(uartSendBuff));
    memset(&testCoap, 0, sizeof(UappsMessage));
    memset(productInit, 0, sizeof(productInit));

    printf("enter Load_Product\n");

    sprintf(rsi, "@SE1./_load");
    cJSON *product = cJSON_CreateObject();
    cJSON *root = cJSON_CreateObject();
    cJSON *vendor = cJSON_CreateObject();

    cJSON_AddStringToObject(root, "ver", PRODUCT_FORMAT_VERSION);
    cJSON_AddStringToObject(root, "name", CompanyName);
    cJSON_AddStringToObject(root, "model", model);
    cJSON_AddStringToObject(root, "pid", PID);
    cJSON_AddStringToObject(root, "license", "1234");

    cJSON_AddStringToObject(vendor, "name", "Name");
    cJSON_AddItemToObject(root, "vendor", vendor);
    cJSON_AddItemToObject(product, "product", root);

    char *pJsonStr = cJSON_PrintUnformatted(product);

    size_t json_len = strlen(pJsonStr);
    if (json_len + 19 >= sizeof(productInit)) {
        printf("[Error] JSON too large! length=%d\n", (int)json_len);
        free(pJsonStr);        // 先释放 JSON 字符串
        cJSON_Delete(product); // 再释放 JSON 对象
        return;
    }
    Add_device_information_Description(productInit, 0x06, (uint8_t *)pJsonStr, json_len);
    memcpy(&productInit[19], (uint8_t *)pJsonStr, json_len);

    UappsPutData(&testCoap, productInit, json_len + 19, UAPPS_FMT_OCTETS, 0); // 加入载荷
    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_Load_Product;

    free(pJsonStr);
    cJSON_Delete(product);

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, sizeof(uartSendBuff));

    app_usart_tx_buf(uartSendBuff, uartSendBuffLen, USART0);
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
#endif
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_Load_AEInfo
函数功能：构造可写入plc模组AE信息的接口函数
输入参数：ae_num-ae列表的元素值；quantity- 元素个数；BrandName-品牌名, id-Panel类型，url-Panel地址
返回值：   无
备 注：
---------------------------------------------------------------*/
#if 0
void CmdTest_MSE_Load_AEInfoMulti(const AE_Info_t *ae_infos, uint8_t ae_count)
{
    if (!ae_infos || ae_count == 0) {
        printf("[Error] AE info is NULL or count is 0!\n");
        return;
    }

    memset(rsi, 0, sizeof(rsi));
    memset(uartSendBuff, 0, sizeof(uartSendBuff));
    memset(&testCoap, 0, sizeof(UappsMessage));
    memset(productInit, 0, sizeof(productInit));

    printf("enter Load_AEInfoOneByOneStable\n");
    sprintf(rsi, "@SE1./_load");

    for (uint8_t i = 0; i < ae_count; i++) {
        const AE_Info_t *info = &ae_infos[i];

        // 打印调试
        printf("[Debug] AE index %d: name=%s, se=%s, panel_id=%s, panel_url=%s\n",
               i,
               info->name ? info->name : "NULL",
               info->se ? info->se : "NULL",
               info->panel_id ? info->panel_id : "NULL",
               info->panel_url ? info->panel_url : "NULL");

        // 检查 ae_num 每个元素是否为 NULL
        for (uint8_t j = 0; j < info->quantity; j++) {
            if (!info->ae_num[j]) {
                printf("[Warning] ae_num[%d] is NULL, replaced with empty string\n", j);
                ((const char **)info->ae_num)[j] = "";
            }
        }

        // 创建 root 和 AE_List
        cJSON *root = cJSON_CreateObject();
        if (!root) {
            printf("[Error] Failed to create root JSON for AE index %d\n", i);
            continue;
        }

        cJSON *AE_List = cJSON_CreateArray();
        if (!AE_List) {
            printf("[Error] Failed to create AE list for AE index %d\n", i);
            cJSON_Delete(root);
            continue;
        }

        cJSON_AddItemToObject(root, "AE", AE_List);

        // 创建 AE 对象
        cJSON *AE_Info   = cJSON_CreateObject();
        cJSON *Panel     = cJSON_CreateObject();
        cJSON *AE_Number = cJSON_CreateStringArray(info->ae_num, info->quantity);

        if (!AE_Info || !Panel || !AE_Number) {
            printf("[Error] Failed to create AE objects for index %d\n", i);
            cJSON_Delete(root);
            continue;
        }

        // 自动处理 NULL 或空格
        cJSON_AddItemToObject(AE_Info, "ae", AE_Number);
        cJSON_AddStringToObject(AE_Info, "name", info->name ? info->name : "");
        cJSON_AddStringToObject(AE_Info, "se", info->se ? info->se : "0");
        cJSON_AddItemToObject(AE_Info, "panel", Panel);
        cJSON_AddStringToObject(Panel, "id", info->panel_id ? info->panel_id : "");

        // 去掉 panel_url 尾部空格
        char panel_url_clean[128] = {0};
        if (info->panel_url) {
            size_t len = strlen(info->panel_url);
            while (len > 0 && (info->panel_url[len - 1] == ' ' || info->panel_url[len - 1] == '\t')) len--;
            strncpy(panel_url_clean, info->panel_url, len);
            panel_url_clean[len] = '\0';
        }
        cJSON_AddStringToObject(Panel, "url", panel_url_clean);

        cJSON_AddItemToArray(AE_List, AE_Info);

        // 打印调试 JSON
        char *pJsonStr = cJSON_PrintUnformatted(root);
        if (!pJsonStr) {
            printf("[Error] Failed to print JSON for AE index %d\n", i);
            cJSON_Delete(root);
            continue;
        }

        printf("[Debug] AE index %d JSON: %s\n", i, pJsonStr);

        // 发送
        Add_device_information_Description(productInit, 0x04, (uint8_t *)pJsonStr, strlen(pJsonStr));
        memcpy(&productInit[19], pJsonStr, strlen(pJsonStr));

        UappsPutData(&testCoap, productInit, strlen(pJsonStr) + 19, UAPPS_FMT_OCTETS, 0);
        UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);
        testCoap.token[0] = 0x52;
        testCoap.token[1] = MSE_Load_AE;

        uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, sizeof(uartSendBuff));
        if (uartSendBuffLen <= 0) {
            printf("[Error] Failed to create UART message for AE index %d\n", i);
        } else {
            MCU_Send_date(uartSendBuff, uartSendBuffLen);
            printf("[Info] JSON sent successfully for AE index %d, length=%d\n", i, uartSendBuffLen);
        }

        // 释放 JSON 内存
        cJSON_Delete(root);
        cJSON_free(pJsonStr);
    }
}
#endif
void PLCP_Load_AEInfo(char *root)
{
    memset(rsi, 0, sizeof(rsi));
    memset(uartSendBuff, 0, sizeof(uartSendBuff));
    memset(&testCoap, 0, sizeof(UappsMessage));
    memset(productInit, 0, sizeof(productInit));
    APP_PRINTF("enter Load_AEInfo\n");
    sprintf(rsi, "@SE1./_load");

    char *pJsonStr = root;
    u16 totalLen = strlen(root);
    u16 offset = 0;
    u16 datalen = 0;
    uint8_t frameType = 0;

    while (offset < totalLen) {
        // 计算当前帧类型
        if (offset == 0) {
            frameType = 0; // 起始帧
        } else if (totalLen - offset <= 200) {
            frameType = 2; // 结束帧
        } else {
            frameType = 1; // 中间帧
        }
        datalen = (totalLen - offset > 200) ? 200 : (totalLen - offset);
        APP_PRINTF("frameType %d, offset %d, datalen %d\n", frameType, offset, datalen);
        memset(productInit, 0, sizeof(productInit));
        Add_device_information_Description2(productInit, frameType, offset, totalLen, 0x04, (uint8_t *)pJsonStr, datalen);

        if (datalen + 19 <= sizeof(productInit)) {
            memcpy(&productInit[19], pJsonStr, datalen);
        } else {
            APP_PRINTF("Error: buffer overflow risk!\n");
            break;
        }
        UappsPutData(&testCoap, productInit, datalen + 19, UAPPS_FMT_OCTETS, 0);
        UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);
        testCoap.token[0] = 0x52;
        testCoap.token[1] = MSE_Load_AE;

        uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, sizeof(uartSendBuff));
        if (uartSendBuffLen > 0) {
            MCU_Send_date(uartSendBuff, uartSendBuffLen);
        }
        pJsonStr += datalen;
        offset += datalen;
    }
}
#if 0
void CmdTest_MSE_Load_AEInfo(const char *ae_num, uint8_t quantity, char *BrandName, char *id, char *url)
{
    memset(rsi, 0, sizeof(rsi));
    memset(uartSendBuff, 0, sizeof(uartSendBuff));
    memset(&testCoap, 0, sizeof(UappsMessage));
    memset(productInit, 0, sizeof(productInit));
    printf("enter Load_AEInfo\n");

    sprintf(rsi, "@SE1./_load");

    cJSON *root      = cJSON_CreateObject();
    cJSON *AE_Info   = cJSON_CreateObject();
    cJSON *Panel     = cJSON_CreateObject();
    cJSON *AE_List   = cJSON_CreateArray();
    cJSON *AE_Number = cJSON_CreateStringArray(ae_num, quantity);

    cJSON_AddItemToObject(AE_Info, "ae", AE_Number);
    cJSON_AddStringToObject(AE_Info, "name", BrandName);
    cJSON_AddStringToObject(AE_Info, "se", "1");
    cJSON_AddStringToObject(Panel, "id", id);
    cJSON_AddStringToObject(Panel, "url", url);

    cJSON_AddItemToObject(AE_Info, "panel", Panel);
    cJSON_AddItemToArray(AE_List, AE_Info);
    cJSON_AddItemToObject(root, "AE", AE_List);

    char *pJsonStr = cJSON_PrintUnformatted(root);

    Add_device_information_Description(productInit, 0x04, (uint8_t *)pJsonStr, strlen(pJsonStr));
    memcpy(&productInit[19], pJsonStr, strlen(pJsonStr));

    UappsPutData(&testCoap, productInit, strlen(pJsonStr) + 19, UAPPS_FMT_OCTETS, 0); // 加入载荷
    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);
    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_Load_AE;

    cJSON_Delete(root);
    cJSON_free(pJsonStr);

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, sizeof(uartSendBuff));
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}
#endif
void PLCP_Load_Widgets(char *root)
{
    memset(rsi, 0, sizeof(rsi));
    memset(uartSendBuff, 0, sizeof(uartSendBuff));
    memset(&testCoap, 0, sizeof(UappsMessage));
    memset(productInit, 0, sizeof(productInit));

    printf("enter Load_Widgets\n");

    sprintf(rsi, "@SE1./_load");

    // 这里 root 就是 JSON 字符串
    char *pJsonStr = root;
    uint16_t jsonLen = strlen(pJsonStr);

    Add_device_information_Description(productInit, 0x05, (uint8_t *)pJsonStr, jsonLen);

    memcpy(&productInit[19], (uint8_t *)pJsonStr, jsonLen);

    UappsPutData(&testCoap, productInit, jsonLen + 19, UAPPS_FMT_OCTETS, 0);

    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);

    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_Load_Widgets;
    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, sizeof(uartSendBuff));
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
}

/*--------------------------------------------------------------
函数名称：CmdTest_MSE_Load_Widgets
函数功能：构造可写入plc模组Widgets信息的接口函数
输入参数：ae_num-ae列表的元素值；quantity- 元素个数；
返回值：   无
备 注：
---------------------------------------------------------------*/
void CmdTest_MSE_Load_Widgets(const char *ae_num, uint8_t quantity)
{
    UappsMessage testCoap;
    char rsi[50];
    uint8_t uartSendBuff[256];
    u16 uartSendBuffLen = 0;
    uint8_t productInit[256];
    memset(rsi, 0, sizeof(rsi));
    memset(uartSendBuff, 0, sizeof(uartSendBuff));
    memset(&testCoap, 0, sizeof(UappsMessage));
    memset(productInit, 0, sizeof(productInit));

    printf("enter Load_Widgets\n");
    sprintf(rsi, "@SE1./_load");

    cJSON *wg_array = cJSON_CreateArray();
    cJSON *element = cJSON_CreateObject();
    cJSON_AddStringToObject(element, "name", "Switch");
    cJSON_AddStringToObject(element, "se", "1");
    cJSON_AddStringToObject(element, "type", "switch");

    // Create the events array and add event objects to it
    cJSON *events_array = cJSON_CreateArray();

    cJSON *ae = cJSON_CreateStringArray(&ae_num, quantity);
    cJSON_AddItemToObject(element, "ae", ae);

    cJSON *event1 = cJSON_CreateObject();
    cJSON_AddStringToObject(event1, "id", "_open");
    cJSON_AddStringToObject(event1, "name", "ON");
    cJSON_AddStringToObject(event1, "data", "1");
    cJSON_AddItemToArray(events_array, event1);

    cJSON *event2 = cJSON_CreateObject();
    cJSON_AddStringToObject(event2, "id", "_close");
    cJSON_AddStringToObject(event2, "name", "OFF");
    cJSON_AddStringToObject(event2, "data", "0");
    cJSON_AddItemToArray(events_array, event2);
    cJSON_AddItemToObject(element, "events", events_array);

    cJSON_AddItemToArray(wg_array, element);
    char *pJsonStr = cJSON_PrintUnformatted(element);

    // Add_device_information_Description(productInit, 0x05, (uint8_t *)pJsonStr, strlen(pJsonStr));
    memcpy(&productInit[19], pJsonStr, strlen(pJsonStr));

    UappsPutData(&testCoap, productInit, strlen(pJsonStr) + 19, UAPPS_FMT_OCTETS, 0); // 加入载荷
    UappsCreateMessage(&testCoap, UAPPS_TYPE_CON, UAPPS_REQ_PUT, rsi);
    // 如果应用层要设置token ID的话
    testCoap.token[0] = 0x52;
    testCoap.token[1] = MSE_Load_Widgets;

    cJSON_Delete(element);
    cJSON_free(pJsonStr);

    uartSendBuffLen = Aps_UartMessage(&testCoap, uartSendBuff, sizeof(uartSendBuff));
    MCU_Send_date(uartSendBuff, uartSendBuffLen);
    // app_usart_tx_buf(uartSendBuff, uartSendBuffLen, USART0);
}
#endif