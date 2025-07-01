/**
 * @file    main_both_sensors.c
 * @brief   DHT11温湿度传感器 + MQ-2烟雾传感器 组合使用示例
 * @details 同时获取温湿度和烟雾浓度数据，并显示在LCD1602上
 * @note    本文件只调用模块接口函数，不包含底层实现细节
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "mq2.h"
#include "DHT11.h"
#include "lcd.h"
#include <stdio.h>

int main(void)
{
    unsigned int smoke_value = 0;
    unsigned char temp_data = 0, humi_data = 0;
    unsigned char sensor_mode = 0; // 0-显示温湿度, 1-显示烟雾浓度
    
    // 系统初始化
    SystemInit();
    
    // 初始化传感器模块
    MQ2_Init();
    dht11_init();
    
    // 初始化LCD1602
    lcd_init();
    
    // 显示启动信息
    lcd_clear();
    lcd_print_str(0, 0, "Sensors Init...");
    lcd_print_str(1, 0, "DHT11 + MQ-2");
    Mdelay_Lib(2000);
    
    while(1)
    {
        if(sensor_mode == 0)
        {
            // DHT11温湿度采集和显示
            if(dht11_read_dat(&temp_data, &humi_data) == 0)
            {
                char str[32] = {0};
                
                // 第一行显示温度
                sprintf(str, "Temp: %d C", temp_data);
                lcd_print_str(0, 0, str);
                
                // 第二行显示湿度
                sprintf(str, "Humi: %d%%", humi_data);
                lcd_print_str(1, 0, str);
            }
            else
            {
                lcd_print_str(0, 0, "DHT11 Error!    ");
                lcd_print_str(1, 0, "Check wiring    ");
            }
        }
        else
        {
            // MQ-2烟雾浓度采集和显示
            
            // 清除数据就绪标志
            MQ2_ClearFlag();
            
            // 发送命令到MQ-2传感器
            MQ2_SendCommand();

            // 等待传感器数据就绪
            unsigned int timeout = 0;
            while(!MQ2_IsDataReady() && timeout < 500)
            {
                Mdelay_Lib(10);
                timeout++;
            }

            if(timeout < 500)
            {
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
            }
            else
            {
                lcd_print_str(0, 0, "MQ-2 Timeout!   ");
                lcd_print_str(1, 0, "Check sensor    ");
            }
        }
        
        // 延时3秒
        Mdelay_Lib(3000);
        
        // 切换显示模式
        sensor_mode = !sensor_mode;
    }
}
