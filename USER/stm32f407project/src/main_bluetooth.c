/**
 * @file    main_bluetooth.c
 * @brief   智能农业系统 - 蓝牙阈值控制版本
 * @details 集成MQ-2烟雾传感器、DHT11温湿度传感器和蓝牙远程控制功能
 * @note    本文件展示如何通过蓝牙远程设置烟雾报警阈值
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "mq2.h"
#include "DHT11.h"
#include "lcd.h"
#include "bluetooth.h"
#include <stdio.h>

/* 私有变量 */
static uint8_t display_mode = 0;  // 0-温湿度, 1-烟雾浓度, 2-蓝牙状态

int main(void)
{
    unsigned int smoke_value = 0;
    unsigned char temp_data = 0, humi_data = 0;
    uint16_t threshold = 0;
    uint32_t last_bluetooth_check = 0;
    uint32_t last_display_update = 0;
    uint32_t current_time = 0;
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化各个模块
    MQ2_Init();           // MQ-2烟雾传感器 (使用USART3)
    dht11_init();         // DHT11温湿度传感器
    lcd_init();           // LCD1602显示器
    
    // 初始化蓝牙模块 (使用USART2)
    // 首先初始化UART2
    extern void UART2_Init(unsigned int BaudRate);
    UART2_Init(9600);
    Bluetooth_Init();
    
    // 显示启动信息
    lcd_clear();
    lcd_print_str(0, 0, "Smart Agri v2.0");
    lcd_print_str(1, 0, "BT + Sensors");
    Mdelay_Lib(2000);
    
    lcd_clear();
    lcd_print_str(0, 0, "Bluetooth Ready");
    lcd_print_str(1, 0, "Connecting...");
    Mdelay_Lib(2000);
    
    while(1)
    {
        current_time = SysTick->VAL;  // 简单的时间戳
        
        // 每100ms检查一次蓝牙命令
        if(current_time - last_bluetooth_check > 100)
        {
            Bluetooth_ProcessCommand();
            last_bluetooth_check = current_time;
        }
        
        // 每2秒更新一次显示和传感器数据
        if(current_time - last_display_update > 2000)
        {
            // 获取当前阈值
            threshold = Bluetooth_GetThreshold();
            
            switch(display_mode)
            {
                case 0: // 显示温湿度
                {
                    if(dht11_read_dat(&temp_data, &humi_data) == 0)
                    {
                        char str[32];
                        sprintf(str, "T:%dC H:%d%%", temp_data, humi_data);
                        lcd_print_str(0, 0, str);
                        lcd_print_str(1, 0, "Mode: DHT11     ");
                    }
                    else
                    {
                        lcd_print_str(0, 0, "DHT11 Error!    ");
                        lcd_print_str(1, 0, "Check sensor    ");
                    }
                    break;
                }
                
                case 1: // 显示烟雾浓度
                {
                    // 获取MQ-2数据
                    MQ2_ClearFlag();
                    MQ2_SendCommand();
                    
                    // 等待数据就绪 (带超时)
                    uint16_t timeout = 0;
                    while(!MQ2_IsDataReady() && timeout < 500)
                    {
                        Mdelay_Lib(10);
                        timeout++;
                    }
                    
                    if(timeout < 500)
                    {
                        smoke_value = MQ2_GetValue();
                        char str[32];
                        
                        // 第一行显示浓度和阈值
                        sprintf(str, "S:%d T:%d", smoke_value, threshold);
                        lcd_print_str(0, 0, str);
                        
                        // 第二行显示状态
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
                    }
                    else
                    {
                        lcd_print_str(0, 0, "MQ-2 Timeout!   ");
                        lcd_print_str(1, 0, "Check sensor    ");
                    }
                    break;
                }
                
                case 2: // 显示蓝牙状态
                {
                    char str[32];
                    sprintf(str, "BT Thr: %d ppm", threshold);
                    lcd_print_str(0, 0, str);
                    
                    BluetoothStatus_t bt_status = Bluetooth_GetStatus();
                    switch(bt_status)
                    {
                        case BT_STATUS_IDLE:
                            lcd_print_str(1, 0, "BT: Ready       ");
                            break;
                        case BT_STATUS_RECEIVING:
                            lcd_print_str(1, 0, "BT: Receiving...");
                            break;
                        case BT_STATUS_CMD_READY:
                            lcd_print_str(1, 0, "BT: Processing  ");
                            break;
                        default:
                            lcd_print_str(1, 0, "BT: Unknown     ");
                            break;
                    }
                    break;
                }
            }
            
            last_display_update = current_time;
        }
        
        // 每6秒切换显示模式
        static uint32_t last_mode_switch = 0;
        if(current_time - last_mode_switch > 6000)
        {
            display_mode = (display_mode + 1) % 3;
            last_mode_switch = current_time;
        }
        
        // 小延时，避免CPU占用过高
        Mdelay_Lib(50);
    }
}
