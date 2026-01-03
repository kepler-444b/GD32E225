#ifndef _BASE_H_
#define _BASE_H_
#include "../gpio/gpio.h"
#include <stdio.h>
/*
    2025.5.16 舒东升
    本模块用于各种通用接口
*/

#define BIT0(flag)  ((bool)((flag) & 0x0001))
#define BIT1(flag)  ((bool)((flag) & 0x0002))
#define BIT2(flag)  ((bool)((flag) & 0x0004))
#define BIT3(flag)  ((bool)((flag) & 0x0008))
#define BIT4(flag)  ((bool)((flag) & 0x0010))
#define BIT5(flag)  ((bool)((flag) & 0x0020))
#define BIT6(flag)  ((bool)((flag) & 0x0040))
#define BIT7(flag)  ((bool)((flag) & 0x0080))
#define BIT8(flag)  ((bool)((flag) & 0x0100))
#define BIT9(flag)  ((bool)((flag) & 0x0200))
#define BIT10(flag) ((bool)((flag) & 0x0400))
#define BIT11(flag) ((bool)((flag) & 0x0800))
#define BIT12(flag) ((bool)((flag) & 0x1000))
#define BIT13(flag) ((bool)((flag) & 0x2000))
#define BIT14(flag) ((bool)((flag) & 0x4000))
#define BIT15(flag) ((bool)((flag) & 0x8000))

#define L_BIT(byte) ((uint8_t)((byte) & 0x0F))        // 低4位
#define H_BIT(byte) ((uint8_t)(((byte) >> 4) & 0x0F)) // 高4位

// 返回一个 8 位数中 1 的个数
#define COUNT_ONES(bits)                     \
    ({                                       \
        int _cnt = 0;                        \
        for (int8_t _i = 7; _i >= 0; _i--) { \
            if ((bits) & (1 << _i)) _cnt++;  \
        }                                    \
        _cnt;                                \
    })

// 返回一个 16 位数中 1 的个数
#define COUNT_ONES_16(bits)                   \
    ({                                        \
        int _cnt = 0;                         \
        for (int8_t _i = 15; _i >= 0; _i--) { \
            if ((bits) & (1 << _i)) _cnt++;   \
        }                                     \
        _cnt;                                 \
    })

// 检查字节的某一位是否为 1(高位优先)
#define BIT_HIGH_TO_LOW(byte, pos) ((byte) & (1 << (15 - (pos))))

// 检查16位的某一位是否为 1(高位优先)
#define BIT16_HIGH_TO_LOW(word, pos) ((word) & (1 << (15 - (pos))))

// 一个简单的非阻塞延时,在与plc模块通信时使用
#if 1
#define APP_DELAY                                        \
    do {                                                 \
        for (volatile uint32_t i = 0; i < (1000); i++) { \
            __NOP(); /* 插入空指令防止编译器优化 */      \
        }                                                \
    } while (0)
#endif

// 将连续的2个十六进制字符转为1个字节的 uint8_t 值(在协议解析时用到)
#if defined PLC_HI
static const uint8_t hex_lut[256] = {['0'] = 0, ['1'] = 1, ['2'] = 2, ['3'] = 3, ['4'] = 4, ['5'] = 5, ['6'] = 6, ['7'] = 7, ['8'] = 8, ['9'] = 9, ['A'] = 10, ['B'] = 11, ['C'] = 12, ['D'] = 13, ['E'] = 14, ['F'] = 15, ['a'] = 10, ['b'] = 11, ['c'] = 12, ['d'] = 13, ['e'] = 14, ['f'] = 15};
#define HEX_TO_BYTE(ptr) ((hex_lut[*(ptr)] << 4) | hex_lut[*(ptr + 1)])
#endif

#if defined PLC_LHW
//  将十六进制转为小写的asci
#define HEX_TO_ASCII(mac, ascii)                                         \
    do {                                                                 \
        for (int i = 0; i < 6; i++) {                                    \
            ascii[2 * i]     = "0123456789abcdef"[(mac[i] >> 4) & 0x0F]; \
            ascii[2 * i + 1] = "0123456789abcdef"[mac[i] & 0x0F];        \
        }                                                                \
    } while (0)

// 查找第二个 0xFF 力合微模组通信用到
#define FIND_SECOND_FF(buffer, length, index_var)   \
    do {                                            \
        uint8_t _found_ff_count = 0;                \
        (index_var)             = 0;                \
        for (uint8_t _i = 0; _i < (length); _i++) { \
            if ((buffer)[_i] == 0xFF) {             \
                _found_ff_count++;                  \
                if (_found_ff_count == 2) {         \
                    (index_var) = _i + 1;           \
                    break;                          \
                }                                   \
            }                                       \
        }                                           \
    } while (0)
#endif
uint16_t app_calculate_average(const uint16_t *buffer, uint16_t count);

// 将 uint8_t 数组打包为 uint32_t 数组(小端序)
bool app_uint8_to_uint32(const uint8_t *input, size_t input_count, uint32_t *output, size_t output_count);

// 将 uint32_t 数组解包为 uint8_t 数组(小端序)
bool app_uint32_to_uint8(const uint32_t *input, size_t input_count, uint8_t *output, size_t output_count);

#endif