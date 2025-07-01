#ifndef _ADC3__H_
#define _ADC3__H_

#include "stm32f4xx.h"

/* 类型定义 */
typedef uint8_t u8;
typedef uint16_t u16;

/* 函数声明 */
void Adc3_Init(void);
u16 Get_Adc3(u8 ch);

#endif

















