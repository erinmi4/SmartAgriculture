#ifndef __KEY_H
#define __KEY_H


#include "stm32f4xx.h"

//按键定义

//KEY0 PA0
#define KEY0_PIN GPIO_Pin_0
#define KEY0_GPIO GPIOA
#define KEY0_RCC RCC_AHB1Periph_GPIOA

//KEY1 PE2
#define KEY1_PIN GPIO_Pin_2
#define KEY1_GPIO GPIOE
#define KEY1_RCC RCC_AHB1Periph_GPIOE

//KEY2 PE3
#define KEY2_PIN GPIO_Pin_3
#define KEY2_GPIO GPIOE
#define KEY2_RCC RCC_AHB1Periph_GPIOE

//KEY3 PE4
#define KEY3_PIN GPIO_Pin_4
#define KEY3_GPIO GPIOE
#define KEY3_RCC RCC_AHB1Periph_GPIOE

//按键初始化
void Key_Init(void);      // 初始化所有按键
void Key0_Init(void);     // 初始化KEY0
void Key1_Init(void);     // 初始化KEY1
void Key2_Init(void);     // 初始化KEY2
void Key3_Init(void);     // 初始化KEY3

//按键防抖函数
//返回1表示按下，0表示未按下
uint8_t Key_Debounce(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

void Key_LED_Control(void); // 按键控制LED翻转函数
#endif


