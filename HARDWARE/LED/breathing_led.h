#ifndef __BREATHING_LED_H
#define __BREATHING_LED_H

#include "stm32f4xx.h"

// ��ʼ����ʱ����GPIO
void TIM1_PWM_Init(void);

// ������Ч������
void Breathing_LED_Effect(void);

#endif

