#include "exti.h"
#include "led.h"
#include "beep.h" // 添加蜂鸣器头文件，解决Beep_On/Off函数隐式声明的警告

/**
 * @file    Exti.c
 * @brief   外部中断控制模块源文件
 * @details 实现STM32F4xx系列MCU外部中断的配置和处理
 * @author  Your Name
 * @date    2025-06-28
 * @version 1.0
 */

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
                 uint8_t PreemptionPriority, uint8_t SubPriority)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    uint32_t RCC_AHB1Periph;
    
    /* 根据GPIO端口确定对应的RCC时钟 */
    if (GPIOx == GPIOA)
        RCC_AHB1Periph = RCC_AHB1Periph_GPIOA;
    else if (GPIOx == GPIOB)
        RCC_AHB1Periph = RCC_AHB1Periph_GPIOB;
    else if (GPIOx == GPIOC)
        RCC_AHB1Periph = RCC_AHB1Periph_GPIOC;
    else if (GPIOx == GPIOD)
        RCC_AHB1Periph = RCC_AHB1Periph_GPIOD;
    else if (GPIOx == GPIOE)
        RCC_AHB1Periph = RCC_AHB1Periph_GPIOE;
    else if (GPIOx == GPIOF)
        RCC_AHB1Periph = RCC_AHB1Periph_GPIOF;
    else if (GPIOx == GPIOG)
        RCC_AHB1Periph = RCC_AHB1Periph_GPIOG;
    else
        return; /* 不支持的GPIO端口 */
    
    /* 1. 配置GPIO */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;  /* 上拉 */
    GPIO_Init(GPIOx, &GPIO_InitStructure);
    
    /* 2. 配置SYSCFG选择器 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(PortSource, PinSource);
    
    /* 3. 配置EXTI外部中断控制器 */
    EXTI_InitStructure.EXTI_Line = EXTI_Line;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    
    /* 根据传入的触发方式设置EXTI触发类型 */
    if (Trigger == EXTI_TRIGGER_RISING)
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    else if (Trigger == EXTI_TRIGGER_FALLING)
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    else
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
    
    /* 4. 配置NVIC中断控制器 */
    NVIC_InitStructure.NVIC_IRQChannel = IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PreemptionPriority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = SubPriority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/**
 * @brief  初始化EXTI0外部中断
 * @param  None
 * @retval None
 * @note   此函数配置PA0为外部中断源，上升沿触发
 */
void EXTI0_Init(void)
{
    /* 使用通用配置函数初始化EXTI0 */
    EXTI_Config(EXTI0_GPIO, EXTI0_PIN, 
                EXTI0_PORT_SOURCE, EXTI0_PIN_SOURCE, 
                EXTI_Line0, EXTI0_IRQn,
                EXTI_TRIGGER_RISING, 1, 1);
}

/**
 * @brief  初始化EXTI1外部中断
 * @param  None
 * @retval None
 * @note   此函数配置PE1为外部中断源，下降沿触发
 */
void EXTI1_Init(void)
{
    /* 使用通用配置函数初始化EXTI1 */
    EXTI_Config(EXTI1_GPIO, EXTI1_PIN, 
                EXTI1_PORT_SOURCE, EXTI1_PIN_SOURCE, 
                EXTI_Line1, EXTI1_IRQn,
                EXTI_TRIGGER_FALLING, 1, 2);
}

/**
 * @brief  初始化EXTI2外部中断
 * @param  None
 * @retval None
 * @note   此函数配置PE2为外部中断源，下降沿触发
 */
void EXTI2_Init(void)
{
    /* 使用通用配置函数初始化EXTI2 */
    EXTI_Config(EXTI2_GPIO, EXTI2_PIN, 
                EXTI2_PORT_SOURCE, EXTI2_PIN_SOURCE, 
                EXTI_Line2, EXTI2_IRQn,
                EXTI_TRIGGER_FALLING, 1, 3);
}

/**
 * @brief  初始化EXTI3外部中断
 * @param  None
 * @retval None
 * @note   此函数配置PE3为外部中断源，下降沿触发
 */
void EXTI3_Init(void)
{
    /* 使用通用配置函数初始化EXTI3 */
    EXTI_Config(EXTI3_GPIO, EXTI3_PIN, 
                EXTI3_PORT_SOURCE, EXTI3_PIN_SOURCE, 
                EXTI_Line3, EXTI3_IRQn,
                EXTI_TRIGGER_FALLING, 1, 4);
}

/**
 * @brief  初始化EXTI4外部中断
 * @param  None
 * @retval None
 * @note   此函数配置PE4为外部中断源，下降沿触发
 */
void EXTI4_Init(void) 
{
    /* 使用通用配置函数初始化EXTI4 */
    EXTI_Config(EXTI4_GPIO, EXTI4_PIN, 
                EXTI4_PORT_SOURCE, EXTI4_PIN_SOURCE, 
                EXTI_Line4, EXTI4_IRQn,
                EXTI_TRIGGER_FALLING, 2, 2);
}

/**
 * @brief  EXTI0中断处理函数 - 处理KEY0按键
 * @param  None
 * @retval None
 * @note   此函数在EXTI0中断触发时被调用，点亮LED0
 */
void EXTI0_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line0) != RESET)
    {
        /* 点亮LED0 */
        Led_On(0);
        
        /* 清除EXTI0的中断标志 */
        EXTI_ClearITPendingBit(EXTI_Line0);
    }
}

/**
 * @brief  EXTI1中断处理函数
 * @param  None
 * @retval None
 * @note   此函数目前未被使用
 */
void EXTI1_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line1) != RESET)
    {
        /* 清除EXTI1的中断标志 */
        EXTI_ClearITPendingBit(EXTI_Line1);
    }
}

/**
 * @brief  EXTI2中断处理函数
 * @param  None
 * @retval None
 * @note   此函数在EXTI2中断触发时被调用
 */
void EXTI2_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        /* 关闭蜂鸣器 */
        Beep_Off();
        
        /* 清除EXTI2的中断标志 */
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

/**
 * @brief  EXTI3中断处理函数
 * @param  None
 * @retval None
 * @note   此函数在EXTI3中断触发时被调用
 */
void EXTI3_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        /* 熄灭LED0 */
        Led_Off(0);
        
        /* 清除EXTI3的中断标志 */
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

/**
 * @brief  EXTI4中断处理函数
 * @param  None
 * @retval None
 * @note   此函数在EXTI4中断触发时被调用
 */
void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        /* 打开蜂鸣器 */
        Beep_On();
        
        /* 清除EXTI4的中断标志 */
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}
