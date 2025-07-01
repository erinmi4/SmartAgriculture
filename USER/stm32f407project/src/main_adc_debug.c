/**
 * @file    main_adc_debug.c
 * @brief   ADC调试程序
 * @details 专门用于调试ADC读取问题的程序
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "ADC3.h"
#include <stdio.h>

int main(void)
{
    uint16_t adc_ch5 = 0;
    uint16_t adc_direct = 0;
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    
    // 测试LCD显示
    lcd_clear();
    lcd_print_str(0, 0, "ADC Debug");
    lcd_print_str(1, 0, "Initializing...");
    Mdelay_Lib(2000);
    
    // 手动初始化GPIO (PF7)
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    // 初始化ADC3
    Adc3_Init();
    Mdelay_Lib(100);
    
    // 显示启动信息
    lcd_clear();
    lcd_print_str(0, 0, "ADC Debug");
    lcd_print_str(1, 0, "Ready!");
    Mdelay_Lib(2000);
    
    while(1)
    {
        // 直接读取ADC3通道5
        adc_ch5 = Get_Adc3(ADC_Channel_5);
        
        // 也尝试读取其他通道作为对比
        adc_direct = ADC_GetConversionValue(ADC3);
        
        // 在LCD上显示ADC数据
        char str[32] = {0};
        
        // 第一行显示通道5的值
        sprintf(str, "CH5: %4d", adc_ch5);
        lcd_print_str(0, 0, str);
        
        // 第二行显示直接读取的值
        sprintf(str, "DIR: %4d", adc_direct);
        lcd_print_str(1, 0, str);
        
        Mdelay_Lib(500); // 0.5秒更新一次
    }
}
