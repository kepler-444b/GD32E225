#include <stdio.h>
#include "../../Source/plcp_common/Inc/lmexxx_conf.h"
#include "MseProcess.h"
#include <stdarg.h>
#include "APP_PublicAttribute.h"
#if 0

int main()
{
    //****************************************main函数 示例*************************************
    const sPost_wflash *sPost_flag = NULL;
    sPost_flag                     = APP_Postflag_GetPointer();

    APP_PLCSDK_Init();

    printf("run start\r\n");
    while (1) {
        if (systemTimer.flag50ms) {
            systemTimer.flag50ms = 0;
            APP_Queue_ListenAndHandleMessage();
        }
        if (systemTimer.flag1000ms) {
            systemTimer.flag1000ms = 0;
            if (sPost_flag->wproductsecflag == 1 && sPost_flag->wAEsecflag != 1) /*产品信息写成功后判断是否需要写ae信息*/
            {
                MCU_Load_AEInfo();
            }
            if (sPost_flag->wAEsecflag == 1 && sPost_flag->wWidgetsecflag != 1) /*产品信息写成功后判断是否需要写ae信息*/
            {
                MCU_Load_Widgets();
            }
            if (sPost_flag->wWidgetsecflag == 1 && sPost_flag->wtranferflag != 1) /*ae信息写成功后判断是否需要使能透传功能*/
            {
                MCU_Enable_Transmission();
            }
        }
        if (systemTimer.flag5000ms) /*每次上电5秒后判断是否需要写产品信息*/
        {
            systemTimer.flag5000ms = 0;
            if (sPost_flag->wproductsecflag == 0) {
                MCU_Load_Product();
            }
            if (get_net_state() == 1 && sPost_flag->wtranferflag == 1) {
                CmdTest_MSE_GET_DID(); // 入网后每隔5s检查配网状态
            }
        }
    }
    return 0;
}

#endif