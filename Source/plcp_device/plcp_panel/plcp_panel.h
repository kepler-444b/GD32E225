#ifndef _PLCP_PANEL_H_
#define _PLCP_PANEL_H_

#define ADC_TO_VOL(adc_val) ((adc_val) * 330 / 4096) // adc值转电压


#define VOL_BUF_SIZE 10
#define LONG_PRESS   1000 // 长按时间
#define MIN_VOL      329  // 无按键按下时的最小电压值
#define MAX_VOL      330  // 无按键按下时的最大电压值

void plcp_panel_init(void);
#endif