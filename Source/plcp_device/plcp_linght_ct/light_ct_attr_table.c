#include "light_ct_attr_table.h"
#include "../../Source/base/base.h"
#include "light_ct_adapter.h"
#include "plcp_light_ct_info.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// 控制灯驱开关
uint8_t attr_light_ct_table_set(uint8_t onoff)
{
    APP_PRINTF("attr_light_ct_table_set\n");
    const light_ct_t *obj = get_light_info();
    if (onoff) {
        light_adapter_ctrl(obj);
    } else {
        light_adapter_close(obj->grad_time);
    }
    return 1;
}

void light_ct_ctrl(uint8_t *buf, uint8_t len)
{
    APP_PRINTF_BUF("buf", buf, len);
    uint8_t cmd_type = buf[0];
    if (cmd_type != 1)
        return; // 只解析状态控制命令

    // 控制字 16bit, MSB先
    uint16_t ctrl_word = ((uint16_t)buf[1] << 8) | buf[2];
    // 解析控制字标志
    uint8_t P_flag = (ctrl_word >> 14) & 0x03; // 渐变标志 P (2bit)
    bool L_flag = BIT13(ctrl_word);            // 亮度 L
    bool T_flag = BIT12(ctrl_word);            // 色温 T
    bool M_flag = BIT6(ctrl_word);             // 断电记忆 M
    bool K_flag = BIT5(ctrl_word);             // 维持时间 K
    bool Ti_flag = BIT4(ctrl_word);            // 定时 Ti
    bool brightness_type;                      // 百分比亮度

    // 指向参数区
    uint8_t *p = buf + 3; // 指向数据
    uint8_t remain = len - 3;

    // P 渐变时间 P=0 不渐变  P=1 使用默认渐变时间 P=2 后面跟1字节渐变时间(单位100ms) P=3 随机跳变
    uint8_t grad_time = 0;
    if (P_flag == 2 && remain >= 1) {
        grad_time = *p++; // 单字节，单位100ms
        remain--;
    }
    // L 亮度 百分比格式(bit7=1 1字节 范围0~100),绝对值格式(bit7=0 2字节(MSB First) 范围0~10000)
    uint16_t brightness = 0;
    if (L_flag) {
        if (remain < 1)
            return;
        if (*p & 0x80) { // 百分比 1字节
            brightness_type = true;
            brightness = *p & 0x7F;
            p++;
            remain--;
        } else { // 绝对值 2字节
            brightness_type = false;
            if (remain < 2)
                return;
            brightness = ((uint16_t)p[0] << 8) | p[1];
            p += 2;
            remain -= 2;
        }
    }
    // T 色温 bit7=1 -> 百分比 bit7=0 -> 绝对值 绝对值范围: 2700K~6500K
    uint16_t color_temp = 0;
    if (T_flag) {
        if (remain < 2)
            return;
        if (p[0] & 0x80) { // 百分比
            color_temp = p[0] & 0x7F;
            p += 1;
            remain -= 1;
        } else { // 绝对值 2字节
            color_temp = ((uint16_t)p[0] << 8) | p[1];
            p += 2;
            remain -= 2;
        }
    }
    // 优先级顺序 Ti > K > M
    uint32_t timer = 0;
    if (Ti_flag && remain >= 3) {
        timer = ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2];
        p += 3;
        remain -= 3;
    }
    uint16_t keep_time = 0;
    if (!Ti_flag && K_flag && remain >= 2) {
        keep_time = ((uint16_t)p[0] << 8) | p[1];
        p += 2;
        remain -= 2;
    }
    uint8_t memory = 0;
    if (!Ti_flag && !K_flag && M_flag && remain >= 1) {
        memory = *p++;
        remain--;
    }
    // 调试打印
    // APP_PRINTF("cmd_type=%d, P=%d, grad_time=%d, L_flag=%d, brightness=%d, T_flag=%d, color_temp=%d, memory=%d, keep_time=%d, timer=%d\n",
    //            cmd_type, P_flag, grad_time, L_flag, brightness, T_flag, color_temp, memory, keep_time, timer);

    static light_ct_t temp;
    memset(&temp, 0, sizeof(temp));
    temp.brightness = brightness;
    temp.color_temp = color_temp;
    temp.P_flag = P_flag;
    temp.grad_time = grad_time;
    temp.timer = timer;
    temp.keep_time = keep_time;
    temp.memory = memory;
    if (brightness_type) {
        temp.brightness_type = true;
    } else {
        temp.brightness_type = false;
    }
    light_adapter_ctrl(&temp);
}
