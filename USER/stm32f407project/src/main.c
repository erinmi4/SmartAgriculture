/**
 * @file    main.c
 * @brief   智能农业系统 - 蓝牙阈值控制版本
 * @details 集成MQ-2烟雾传感器和蓝牙远程阈值控制功能
 * @note    通过蓝牙发送命令：SET_THR:300 设置阈值为300ppm
 */

#include "stm32f4xx.h"
#include "uart.h"
#include "sys.h"
#include "delay.h"
#include "mq2.h"
#include "lcd.h"
#include "bluetooth.h"
#include <stdio.h>

int main(void)
{
    unsigned int smoke_value = 0;
    uint16_t threshold = 0;
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    
    // 测试LCD显示
    lcd_clear();
    lcd_print_str(0, 0, "LCD Test");
    lcd_print_str(1, 0, "Working?");
    Mdelay_Lib(2000);
    
    // 初始化MQ-2传感器模块 (使用USART3)
    lcd_clear();
    lcd_print_str(0, 0, "Init MQ-2...");
    MQ2_Init();
    Mdelay_Lib(1000);
    
    // 初始化蓝牙模块 (使用USART2)
    lcd_clear();
    lcd_print_str(0, 0, "Init Bluetooth");
    UART2_Init(UART_BAUD_9600);
    Bluetooth_Init();
    
    // 显示启动信息
    lcd_clear();
    lcd_print_str(0, 0, "Ready! BT:");
    lcd_print_str(1, 0, "1-5=THR 0=? 9=S");
    Mdelay_Lib(2000);
    
    while(1)
    {
        // 处理蓝牙命令
        Bluetooth_ProcessCommand();
        
        // 获取当前阈值
        threshold = Bluetooth_GetThreshold();
        
        // 清除数据就绪标志
        MQ2_ClearFlag();
        
        // 发送命令到MQ-2传感器
        MQ2_SendCommand();

        // 等待传感器数据就绪
        while(!MQ2_IsDataReady())
        {
            Mdelay_Lib(10);
            // 在等待期间也处理蓝牙命令
            Bluetooth_ProcessCommand();
        }

        // 获取烟雾浓度值
        smoke_value = MQ2_GetValue();

        // 在LCD上显示烟雾浓度和阈值
        char str[32] = {0};
        sprintf(str, "S:%4d T:%4d", smoke_value, threshold);
        lcd_print_str(0, 0, str);
        
        // 根据烟雾浓度和动态阈值显示状态
        if(smoke_value < threshold * 0.5)
        {
            lcd_print_str(1, 0, "Status: Normal  ");
        }
        else if(smoke_value < threshold)
        {
            lcd_print_str(1, 0, "Status: Caution ");
        }
        else
        {
            lcd_print_str(1, 0, "Status: ALARM!! ");
        }
        
        // 通过蓝牙发送状态更新
        Bluetooth_SendStatus(smoke_value);
        
        Mdelay_Lib(2000); // 2秒后进行下一次检测
    }
}

