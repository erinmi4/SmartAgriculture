/**
 * @file    main_debug.c
 * @brief   蓝牙阈值控制系统 - 调试版本
 * @details 包含更多调试信息，帮助诊断LCD显示问题
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
    uint8_t step = 0;  // 调试步骤计数器
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 逐步初始化，每步都有LCD显示确认
    step = 1;
    
    // 初始化LCD1602 (第一步)
    lcd_init();
    Mdelay_Lib(100);
    
    // 测试LCD是否工作
    lcd_clear();
    lcd_print_str(0, 0, "LCD Test OK");
    lcd_print_str(1, 0, "Step 1");
    Mdelay_Lib(2000);
    
    // 初始化MQ-2传感器模块 (第二步)
    step = 2;
    lcd_clear();
    lcd_print_str(0, 0, "Init MQ-2...");
    lcd_print_str(1, 0, "Step 2");
    MQ2_Init();
    Mdelay_Lib(1000);
    
    // 初始化蓝牙模块 (第三步)
    step = 3;
    lcd_clear();
    lcd_print_str(0, 0, "Init Bluetooth");
    lcd_print_str(1, 0, "Step 3");
    UART2_Init(UART_BAUD_9600);
    Bluetooth_Init();
    Mdelay_Lib(1000);
    
    // 显示启动完成信息
    step = 4;
    lcd_clear();
    lcd_print_str(0, 0, "System Ready!");
    lcd_print_str(1, 0, "BT: 1-5,0,9");
    Mdelay_Lib(2000);
    
    uint32_t loop_count = 0;
    
    while(1)
    {
        loop_count++;
        
        // 处理蓝牙命令
        Bluetooth_ProcessCommand();
        
        // 获取当前阈值
        threshold = Bluetooth_GetThreshold();
        
        // 显示当前循环次数和阈值 (用于确认程序在运行)
        char debug_str[32];
        sprintf(debug_str, "Loop:%lu T:%d", loop_count % 10000, threshold);
        lcd_print_str(0, 0, debug_str);
        
        // 尝试获取MQ-2数据
        MQ2_ClearFlag();
        MQ2_SendCommand();

        // 等待传感器数据就绪 (带超时和显示)
        uint16_t timeout = 0;
        lcd_print_str(1, 0, "Waiting MQ-2...");
        
        while(!MQ2_IsDataReady() && timeout < 500)
        {
            Mdelay_Lib(10);
            timeout++;
            // 在等待期间也处理蓝牙命令
            Bluetooth_ProcessCommand();
            
            // 每100ms更新一次等待显示
            if(timeout % 10 == 0)
            {
                char wait_str[16];
                sprintf(wait_str, "Wait:%d/500", timeout);
                lcd_print_str(1, 0, wait_str);
            }
        }

        if(timeout >= 500)
        {
            // MQ-2超时
            lcd_print_str(1, 0, "MQ-2 Timeout!");
            smoke_value = 0;  // 设置默认值
        }
        else
        {
            // 获取烟雾浓度值
            smoke_value = MQ2_GetValue();
            
            // 显示获取到的值
            char smoke_str[16];
            sprintf(smoke_str, "S:%d", smoke_value);
            lcd_print_str(1, 0, smoke_str);
        }
        
        Mdelay_Lib(1000);  // 显示1秒
        
        // 根据烟雾浓度和动态阈值显示状态
        char status_str[16];
        if(smoke_value < threshold / 2)
        {
            sprintf(status_str, "Normal");
        }
        else if(smoke_value < threshold)
        {
            sprintf(status_str, "Caution");
        }
        else
        {
            sprintf(status_str, "ALARM!!");
        }
        
        // 显示完整状态
        char full_status[32];
        sprintf(full_status, "S:%d T:%d", smoke_value, threshold);
        lcd_print_str(0, 0, full_status);
        lcd_print_str(1, 0, status_str);
        
        // 通过蓝牙发送状态更新
        Bluetooth_SendStatus(smoke_value);
        
        Mdelay_Lib(2000); // 2秒后进行下一次检测
    }
}
