/**
 * @file    main_light_test.c
 * @brief   光敏电阻测试程序
 * @details 专门用于测试光敏电阻功能的简化版本
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "light.h"
#include <stdio.h>

int main(void)
{
    uint16_t light_raw = 0;
    uint8_t light_percent = 0;
    LightLevel_t light_level;
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    
    // 初始化光敏电阻
    lcd_clear();
    lcd_print_str(0, 0, "Init Light...");
    Light_Init();
    Mdelay_Lib(1000);
    
    lcd_clear();
    
    while(1)
    {
        // 获取光照数据
        light_raw = Light_GetRawValue();        // 原始ADC值 (0-4095)
        light_percent = Light_GetValue();       // 0-100范围的光照强度百分比
        light_level = Light_GetLevel();         // 光照等级
        
        // 在LCD上显示光照数据
        char str[32] = {0};
        
        // --- 已修改: 更新显示逻辑 ---
        // 第一行显示光照百分比和原始ADC值
        // 使用 %u 来打印无符号整数
        sprintf(str, "L:%3u%% ADC:%4u", light_percent, light_raw);
        lcd_print_str(0, 0, str);
        
        // 第二行显示光照等级的文字描述
        sprintf(str, "Level: %s", Light_GetLevelString(light_level));
        lcd_print_str(1, 0, str);
        
        Mdelay_Lib(500); // 0.5秒更新一次
    }
}
