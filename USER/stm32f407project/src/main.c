/**
 * @file    main.c
 * @brief   MQ-2烟雾传感器主程序
 * @details 实时获取环境烟雾数据并显示在LCD1602上
 * @note    本文件只调用模块接口函数，不包含底层实现细节
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "mq2.h"
#include "lcd.h"
#include <stdio.h>

int main(void)
{
    unsigned int smoke_value = 0;
    
    // 系统初始化
    SystemInit();
    
    // 初始化MQ-2传感器模块
    MQ2_Init();
    
    // 初始化LCD1602
    lcd_init();
    
    // 显示启动信息
    lcd_clear();
    lcd_print_str(0, 0, "MQ-2 Starting...");
    lcd_print_str(1, 0, "Please wait...");
    Mdelay_Lib(2000);
    
    while(1)
    {
        // 清除数据就绪标志
        MQ2_ClearFlag();
        
        // 发送命令到MQ-2传感器
        MQ2_SendCommand();

        // 等待传感器数据就绪
        while(!MQ2_IsDataReady())
        {
            Mdelay_Lib(10); // 添加小延时避免死循环
        }

        // 获取烟雾浓度值
        smoke_value = MQ2_GetValue();

        // 在LCD上显示烟雾浓度
        char str[32] = {0};
        sprintf(str, "Smoke:%4d ppm", smoke_value);
        lcd_print_str(0, 0, str);
        
        // 根据烟雾浓度显示状态
        if(smoke_value < 100)
        {
            lcd_print_str(1, 0, "Status: Normal  ");
        }
        else if(smoke_value < 300)
        {
            lcd_print_str(1, 0, "Status: Low     ");
        }
        else if(smoke_value < 500)
        {
            lcd_print_str(1, 0, "Status: Medium  ");
        }
        else
        {
            lcd_print_str(1, 0, "Status: HIGH!!! ");
        }
        
        Mdelay_Lib(2000); // 2秒后进行下一次检测
    }
}