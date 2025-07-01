

#include "stm32f4xx.h"
#include "led.h"
#include "lcd.h"
#include "delay.h"
#include "DHT11.h"  // DHT11头文件
#include <stdio.h>  // sprintf函数

int main(void)
{
    // 系统初始化
    SystemInit();
    
    // 初始化LED用于调试
    Led_Init();
    
    // 使用简单延时让系统稳定
    Simple_Delay_Ms(100);

    for(int i = 0; i < 3; i++) {
        Led_On(LED1);
        Mdelay_Lib(200);  // 测试修复后的SysTick延时
        Led_Off(LED1);
        Mdelay_Lib(200);
    }

    // 初始化LCD
    lcd_init();
    
    // 延时确保LCD初始化完成
    Mdelay_Lib(100);
    
    // 初始化DHT11
    DHT11_Init();
    
    // 延时确保DHT11稳定
    Mdelay_Lib(100);
    
    // 清屏
    lcd_clear();
    
    // 显示初始信息
    lcd_print_str(0, 0, "Temp&Humi Sensor");
    lcd_print_str(1, 0, "Initializing...");
    
    // LED快速闪烁表示初始化完成
    for(int i = 0; i < 5; i++) {
        Led_Toggle(LED0);
        Simple_Delay_Ms(100);
    }
    
    // 延时2秒显示初始化信息
    Simple_Delay_Ms(2000);

    while (1)
    {
        unsigned char temperature, humidity;
        char temp_str[17], humi_str[17];
        
        // 读取DHT11温湿度数据
        if(DHT11_ReadData(&temperature, &humidity) == 0)  // 读取成功
        {
            // 清屏
            lcd_clear();
            
            // 格式化温度字符串
            sprintf(temp_str, "Temp: %d C", temperature);
            // 格式化湿度字符串  
            sprintf(humi_str, "Humi: %d%%", humidity);
            
            // 在LCD上显示温湿度
            lcd_print_str(0, 0, temp_str);
            lcd_print_str(1, 0, humi_str);
            
            // LED正常闪烁表示读取成功
            Led_Toggle(LED0);
        }
        else  // 读取失败
        {
            // 清屏
            lcd_clear();
            
            // 显示错误信息
            lcd_print_str(0, 0, "DHT11 Error!");
            lcd_print_str(1, 0, "Check Connect");
            
            // LED快速闪烁表示错误
            for(int i = 0; i < 3; i++) {
                Led_Toggle(LED1);
                Simple_Delay_Ms(100);
            }
        }
        
        // 每2秒更新一次数据
        Simple_Delay_Ms(2000);
    }
}

