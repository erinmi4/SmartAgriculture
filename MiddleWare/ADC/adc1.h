#ifndef __ADC1_H
#define __ADC1_H

#include "stm32f4xx.h"

/* 类型定义，方便移植 */
typedef uint8_t u8;
typedef uint16_t u16;

/* 函数声明 */
void Adc1_Init(void);
u16 Get_Adc1(u8 ch);

#endif
