#ifndef __BREATHING_LED_H
#define __BREATHING_LED_H

#include "stm32f4xx.h"

// 初始化定时器和GPIO
void TIM1_PWM_Init(void);

// 呼吸灯效果函数
void Breathing_LED_Effect(void);

#endif

