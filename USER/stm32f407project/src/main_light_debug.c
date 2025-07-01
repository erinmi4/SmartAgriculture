/**
 * @file    main_light_debug.c
 * @brief   光敏电阻调试程序
 * @details 专门用于调试光敏电阻ADC读取问题
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "light.h"
#include "ADC3.h"
#include <stdio.h>

int main(void)
{
    uint16_t light_raw = 0;
    uint16_t direct_adc = 0;
    uint8_t light_value = 0;
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    lcd_clear();
    lcd_print_str(0, 0, "ADC Debug Mode");
    lcd_print_str(1, 0, "Starting...");
    Mdelay_Lib(2000);
    
    // 初始化光敏电阻
    Light_Init();
    lcd_clear();
    lcd_print_str(0, 0, "Light Init OK");
    Mdelay_Lib(1000);
    
    while(1)
    {
        // 直接调用ADC读取函数进行测试
        direct_adc = Get_Adc3(5);  // 直接使用通道5
        
        // 通过封装函数读取
        light_raw = Light_GetRawValue();
        light_value = Light_GetValue();
        
        char str[32] = {0};
        
        // 第一行：直接ADC读取 vs 封装函数读取
        sprintf(str, "D:%4d F:%4d", direct_adc, light_raw);
        lcd_print_str(0, 0, str);
        
        // 第二行：光照强度值
        sprintf(str, "Light: %3d", light_value);
        lcd_print_str(1, 0, str);
        
        // 检查ADC值是否异常
        if(direct_adc > 4095 || light_raw > 4095)
        {
            lcd_clear();
            lcd_print_str(0, 0, "ADC OVERFLOW!");
            sprintf(str, "D:%d F:%d", direct_adc, light_raw);
            lcd_print_str(1, 0, str);
            Mdelay_Lib(5000);
        }
        
        Mdelay_Lib(500); // 快速更新用于调试
    }
}
