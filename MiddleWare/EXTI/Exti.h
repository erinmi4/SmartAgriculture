#ifndef __EXTI_H
#define __EXTI_H

/**
 * @file    Exti.h
 * @brief   外部中断控制模块头文件
 * @details 提供STM32F4xx系列MCU外部中断相关的函数声明和宏定义
 * @author  龚维学
 * @date    2025-06-28
 * @version 1.0
 */

#include "stm32f4xx.h"

/* 外部中断相关的引脚和GPIO宏定义 */
#define EXTI0_PIN               GPIO_Pin_0
#define EXTI0_GPIO              GPIOA
#define EXTI0_GPIO_RCC          RCC_AHB1Periph_GPIOA
#define EXTI0_PORT_SOURCE       EXTI_PortSourceGPIOA
#define EXTI0_PIN_SOURCE        EXTI_PinSource0
#define EXTI0_IRQn              EXTI0_IRQn
#define EXTI0_IRQHandler        EXTI0_IRQHandler

#define EXTI1_PIN               GPIO_Pin_1
#define EXTI1_GPIO              GPIOE
#define EXTI1_GPIO_RCC          RCC_AHB1Periph_GPIOE
#define EXTI1_PORT_SOURCE       EXTI_PortSourceGPIOE
#define EXTI1_PIN_SOURCE        EXTI_PinSource1
#define EXTI1_IRQn              EXTI1_IRQn
#define EXTI1_IRQHandler        EXTI1_IRQHandler

#define EXTI2_PIN               GPIO_Pin_2
#define EXTI2_GPIO              GPIOE
#define EXTI2_GPIO_RCC          RCC_AHB1Periph_GPIOE
#define EXTI2_PORT_SOURCE       EXTI_PortSourceGPIOE
#define EXTI2_PIN_SOURCE        EXTI_PinSource2
#define EXTI2_IRQn              EXTI2_IRQn
#define EXTI2_IRQHandler        EXTI2_IRQHandler

#define EXTI3_PIN               GPIO_Pin_3
#define EXTI3_GPIO              GPIOE
#define EXTI3_GPIO_RCC          RCC_AHB1Periph_GPIOE
#define EXTI3_PORT_SOURCE       EXTI_PortSourceGPIOE
#define EXTI3_PIN_SOURCE        EXTI_PinSource3
#define EXTI3_IRQn              EXTI3_IRQn
#define EXTI3_IRQHandler        EXTI3_IRQHandler

#define EXTI4_PIN               GPIO_Pin_4
#define EXTI4_GPIO              GPIOE
#define EXTI4_GPIO_RCC          RCC_AHB1Periph_GPIOE
#define EXTI4_PORT_SOURCE       EXTI_PortSourceGPIOE
#define EXTI4_PIN_SOURCE        EXTI_PinSource4
#define EXTI4_IRQn              EXTI4_IRQn
#define EXTI4_IRQHandler        EXTI4_IRQHandler

/**
 * @brief   中断触发方式枚举
 */
typedef enum {
    EXTI_TRIGGER_RISING,     /**< 上升沿触发 */
    EXTI_TRIGGER_FALLING,    /**< 下降沿触发 */
    EXTI_TRIGGER_BOTH        /**< 上升沿和下降沿都触发 */
} EXTI_TriggerTypeDef;

/* 函数声明 */
/**
 * @brief  初始化EXTI4外部中断
 * @param  None
 * @retval None
 * @note   此函数配置PE4为外部中断源，下降沿触发
 */
void EXTI4_Init(void);

/**
 * @brief  通用外部中断初始化函数
 * @param  GPIOx: GPIO端口，如GPIOE
 * @param  GPIO_Pin: GPIO引脚，如GPIO_Pin_4
 * @param  PortSource: 端口源，如EXTI_PortSourceGPIOE
 * @param  PinSource: 引脚源，如EXTI_PinSource4
 * @param  EXTI_Line: 中断线，如EXTI_Line4
 * @param  IRQn: 中断号，如EXTI4_IRQn
 * @param  Trigger: 触发方式，见EXTI_TriggerTypeDef枚举
 * @param  PreemptionPriority: 抢占优先级，0-15
 * @param  SubPriority: 子优先级，0-15
 * @retval None
 */
void EXTI_Config(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, 
                 uint8_t PortSource, uint8_t PinSource, 
                 uint32_t EXTI_Line, uint8_t IRQn,
                 EXTI_TriggerTypeDef Trigger,
                 uint8_t PreemptionPriority, uint8_t SubPriority);

/**
 * @brief  初始化EXTI0外部中断
 * @param  None
 * @retval None
 * @note   此函数配置PA0为外部中断源
 */
void EXTI0_Init(void);

/**
 * @brief  初始化EXTI1外部中断
 * @param  None
 * @retval None
 * @note   此函数配置PE1为外部中断源
 */
void EXTI1_Init(void);

/**
 * @brief  初始化EXTI2外部中断
 * @param  None
 * @retval None
 * @note   此函数配置PE2为外部中断源
 */
void EXTI2_Init(void);

/**
 * @brief  初始化EXTI3外部中断
 * @param  None
 * @retval None
 * @note   此函数配置PE3为外部中断源
 */
void EXTI3_Init(void);

#endif /* __EXTI_H */

