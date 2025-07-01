/**
 * @file    light_channel_test.c
 * @brief   光敏电阻通道测试
 * @details 测试不同ADC通道号的读取结果
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "ADC3.h"
#include <stdio.h>

int main(void)
{
    uint16_t adc_ch5 = 0;    // 使用ADC_Channel_5
    uint16_t adc_5 = 0;      // 直接使用5
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    lcd_clear();
    lcd_print_str(0, 0, "Channel Test");
    lcd_print_str(1, 0, "Starting...");
    Mdelay_Lib(2000);
    
    // 初始化GPIO PF7
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    // 初始化ADC3
    Adc3_Init();
    Mdelay_Lib(100);
    
    while(1)
    {
        // 测试不同的通道参数
        adc_ch5 = Get_Adc3(ADC_Channel_5);  // 使用宏定义
        adc_5 = Get_Adc3(5);                // 直接使用数字5
        
        char str[32] = {0};
        
        // 第一行：显示ADC_Channel_5的结果
        sprintf(str, "CH5:%4d", adc_ch5);
        lcd_print_str(0, 0, str);
        
        // 第二行：显示直接使用5的结果
        sprintf(str, "5  :%4d", adc_5);
        lcd_print_str(1, 0, str);
        
        Mdelay_Lib(500);
    }
}
