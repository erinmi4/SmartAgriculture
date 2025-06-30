#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f4xx.h"

//Beep PF8
#define BEEP_PIN GPIO_Pin_8
#define BEEP_GPIO GPIOF
#define BEEP_RCC RCC_AHB1Periph_GPIOF
//蜂鸣器初始化
//BEEP -> PF8 -> GPIOF
void Beep_Init(void);

//打开蜂鸣器
void Beep_On(void);

//关闭蜂鸣器
void Beep_Off(void);

#endif
