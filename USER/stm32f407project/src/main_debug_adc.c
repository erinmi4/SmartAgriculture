/**
 * @file    main_debug_adc.c
 * @brief   ADC调试程序 - 查看实际ADC数值
 * @details 显示原始ADC值，帮助调试光照强度计算问题
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "ADC3.h"
#include <stdio.h>

int main(void)
{
    uint16_t adc_raw = 0;
    uint32_t adc_avg = 0;
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    
    // 测试LCD显示
    lcd_clear();
    lcd_print_str(0, 0, "ADC Debug");
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
    
    lcd_clear();
    lcd_print_str(0, 0, "ADC3 Ready");
    lcd_print_str(1, 0, "Debugging...");
    Mdelay_Lib(1000);
    
    while(1)
    {
        // 读取10次求平均
        adc_avg = 0;
        for(int i = 0; i < 10; i++)
        {
            adc_raw = Get_Adc3(ADC_Channel_5);
            adc_avg += adc_raw;
            Mdelay_Lib(5);
        }
        adc_avg = adc_avg / 10;
        
        // 显示原始值和平均值
        char str[32] = {0};
        
        sprintf(str, "Raw:%4d Avg:%4d", adc_raw, (uint16_t)adc_avg);
        lcd_print_str(0, 0, str);
        
        // 显示当前计算结果
        uint32_t temp = adc_avg;
        if(temp > 4000) temp = 4000;
        uint8_t result = 100 - (temp / 40);
        
        sprintf(str, "Calc:%3d T:%4d", result, (uint16_t)temp);
        lcd_print_str(1, 0, str);
        
        Mdelay_Lib(1000);
    }
}
