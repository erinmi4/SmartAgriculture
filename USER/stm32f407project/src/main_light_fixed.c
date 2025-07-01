/**
 * @file    main_light_fixed.c
 * @brief   修复后的光敏电阻测试程序
 * @details 显示ADC原始值和修复后的光照强度计算
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
    uint8_t light_value = 0;
    LightLevel_t light_level = LIGHT_LEVEL_NORMAL;
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    
    // 测试LCD显示
    lcd_clear();
    lcd_print_str(0, 0, "Light Fixed");
    lcd_print_str(1, 0, "Starting...");
    Mdelay_Lib(2000);
    
    // 初始化光敏电阻
    Light_Init();
    Mdelay_Lib(100);
    
    // 显示启动信息
    lcd_clear();
    lcd_print_str(0, 0, "Light Ready");
    lcd_print_str(1, 0, "Testing...");
    Mdelay_Lib(2000);
    
    while(1)
    {
        // 获取光照数据
        light_raw = Light_GetRawValue();        // 原始ADC值
        light_value = Light_GetValue();         // 0-100范围的光照强度
        light_level = Light_GetLevel();
        
        // 在LCD上显示光照数据
        char str[32] = {0};
        
        // 第一行显示原始ADC值和光照强度百分比
        sprintf(str, "A:%4d L:%3d%%", light_raw, light_value);
        lcd_print_str(0, 0, str);
        
        // 第二行显示光照等级
        sprintf(str, "Level: %s", Light_GetLevelString(light_level));
        lcd_print_str(1, 0, str);
        
        Mdelay_Lib(1000); // 1秒更新一次
    }
}
