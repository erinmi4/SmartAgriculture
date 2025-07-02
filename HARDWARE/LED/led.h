#ifndef __LED_H
#define __LED_H

#include "stm32f4xx.h"

//有四个LED
//LED0 PF9  - 烟雾指示
//LED1 PF10 - 温度指示
//LED2 PE13 - 湿度指示 (支持呼吸灯)
//LED3 PE14 - 光照指示 (支持呼吸灯)

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

// 基本LED功能
void Led_Init(void);
void Led_On(uint16_t Led_Num);
void Led_Off(uint16_t Led_Num);
void Led_Toggle(uint16_t Led_Num);

// 呼吸灯功能 (仅LED2和LED3支持)
void Led_BreathingInit(void);                    // 初始化呼吸灯PWM
void Led_SetBreathing(uint16_t led_num, uint16_t brightness); // 设置呼吸灯亮度 (0-999)
void Led_BreathingEffect(uint16_t led_num);      // 呼吸灯效果
void Led_StopBreathing(uint16_t led_num);        // 停止呼吸灯，恢复普通模式

#endif /* __LED_H */



