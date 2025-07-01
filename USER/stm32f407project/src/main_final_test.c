/**
 * @file    main_final_test.c
 * @brief   最终光敏传感器测试程序
 * @details 使用现有的ADC3.c和Get_Adc3函数进行测试
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
    uint8_t light_percent = 0;
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    
    // 测试LCD显示
    lcd_clear();
    lcd_print_str(0, 0, "Light Test Final");
    lcd_print_str(1, 0, "Starting...");
    Mdelay_Lib(2000);
    
    // 初始化GPIO PF7为模拟输入
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
    lcd_print_str(0, 0, "ADC3 Ready");
    lcd_print_str(1, 0, "Testing PF7...");
    Mdelay_Lib(2000);
    
    while(1)
    {
        // 直接读取ADC3通道5的值
        adc_raw = Get_Adc3(ADC_Channel_5);
        
        // 转换为光照强度百分比
        uint32_t temp = adc_raw;
        if(temp > 4000) temp = 4000;
        light_percent = 100 - (temp / 40);
        
        // 在LCD上显示
        char str[32] = {0};
        
        // 第一行显示原始ADC值
        sprintf(str, "ADC: %4d", adc_raw);
        lcd_print_str(0, 0, str);
        
        // 第二行显示光照强度和状态
        if(adc_raw > 4095)
        {
            sprintf(str, "ERROR: %d", adc_raw);
        }
        else if(light_percent > 80)
        {
            sprintf(str, "L:%3d%% Bright", light_percent);
        }
        else if(light_percent > 50)
        {
            sprintf(str, "L:%3d%% Normal", light_percent);
        }
        else
        {
            sprintf(str, "L:%3d%% Dark", light_percent);
        }
        lcd_print_str(1, 0, str);
        
        Mdelay_Lib(500); // 0.5秒更新一次
    }
}
