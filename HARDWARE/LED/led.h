#ifndef __LED_H
#define __LED_H

#include "stm32f4xx.h"

//有四个LED

//LED0 PF9
#define LED0 0
#define LED0_PIN GPIO_Pin_9
#define LED0_GPIO GPIOF
#define LED0_RCC RCC_AHB1Periph_GPIOF

//LED1 PF10
#define LED1 1
#define LED1_PIN GPIO_Pin_10
#define LED1_GPIO GPIOF
#define LED1_RCC RCC_AHB1Periph_GPIOF

//LED2 PE13
#define LED2 2
#define LED2_PIN GPIO_Pin_13
#define LED2_GPIO GPIOE
#define LED2_RCC RCC_AHB1Periph_GPIOE

//LED3 PE14
#define LED3 3
#define LED3_PIN GPIO_Pin_14
#define LED3_GPIO GPIOE
#define LED3_RCC RCC_AHB1Periph_GPIOE

void Led_Init(void);
void Led_On(uint16_t Led_Num);
void Led_Off(uint16_t Led_Num);
void Led_Toggle(uint16_t Led_Num);

#endif /* __LED_H */



