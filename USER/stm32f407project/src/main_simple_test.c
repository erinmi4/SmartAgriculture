/**
 * @file    main_simple_test.c
 * @brief   最简单的光敏电阻测试程序
 * @details 只使用基本的ADC读取，排查ADC数值异常问题
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include <stdio.h>

// 简单的ADC3初始化函数
void Simple_ADC3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
    
    // 使能时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);
    
    // 配置GPIO PF7为模拟输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    // 复位ADC3
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3, ENABLE);
    RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3, DISABLE);
    
    // 初始化ADC公共配置
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
    ADC_CommonInit(&ADC_CommonInitStructure);
    
    // 初始化ADC3
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_Init(ADC3, &ADC_InitStructure);
    
    // 使能ADC3
    ADC_Cmd(ADC3, ENABLE);
}

// 简单的ADC读取函数
uint16_t Simple_Read_ADC3_Channel5(void)
{
    // 配置通道5，排序1，采样时间480周期
    ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 1, ADC_SampleTime_480Cycles);
    
    // 启动软件转换
    ADC_SoftwareStartConv(ADC3);
    
    // 等待转换完成
    while(!ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC));
    
    // 读取并返回转换结果
    return ADC_GetConversionValue(ADC3);
}

int main(void)
{
    uint16_t adc_value = 0;
    uint8_t light_percent = 0;
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    
    // 测试LCD显示
    lcd_clear();
    lcd_print_str(0, 0, "Simple ADC Test");
    lcd_print_str(1, 0, "Initializing...");
    Mdelay_Lib(2000);
    
    // 初始化ADC
    Simple_ADC3_Init();
    Mdelay_Lib(100);
    
    // 显示启动信息
    lcd_clear();
    lcd_print_str(0, 0, "ADC3 Ready");
    lcd_print_str(1, 0, "Reading CH5...");
    Mdelay_Lib(2000);
    
    while(1)
    {
        // 读取ADC值
        adc_value = Simple_Read_ADC3_Channel5();
        
        // 转换为0-100百分比 (参考原始代码逻辑)
        if(adc_value > 4000) adc_value = 4000;
        light_percent = 100 - (adc_value / 40);
        
        // 在LCD上显示
        char str[32] = {0};
        
        // 第一行显示原始ADC值
        sprintf(str, "ADC: %4d", adc_value);
        lcd_print_str(0, 0, str);
        
        // 第二行显示百分比值
        sprintf(str, "Light: %3d%%", light_percent);
        lcd_print_str(1, 0, str);
        
        Mdelay_Lib(500); // 0.5秒更新一次
    }
}
