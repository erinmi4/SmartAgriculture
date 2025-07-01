#include "stm32f4xx.h"
#include "led.h"
#include "lcd.h"
#include "delay.h"
#include "DHT11.h"
#include <stdio.h>

int main(void)
{
    // 系统初始化
    SystemInit();
    
    // 初始化LED用于调试
    Led_Init();
    
    // 系统稳定延时
    Simple_Delay_Ms(100);
    
    // LED闪烁表示系统启动
    for(int i = 0; i < 3; i++) {
        Led_On(LED1);
        Mdelay_Lib(200);
        Led_Off(LED1);
        Mdelay_Lib(200);
    }
    
    // 初始化LCD
    lcd_init();
    Mdelay_Lib(100);
    
    // 初始化DHT11
    DHT11_Init();
    Simple_Delay_Ms(200);  // DHT11需要较长的稳定时间
    
    // 显示启动信息
    lcd_clear();
    lcd_print_str(0, 0, "Smart Agri v1.0");
    lcd_print_str(1, 0, "Starting...");
    Simple_Delay_Ms(2000);
    
    // LED快速闪烁表示初始化完成
    for(int i = 0; i < 5; i++) {
        Led_Toggle(LED0);
        Simple_Delay_Ms(100);
    }
    
    // 主循环变量
    unsigned char temperature, humidity;
    char display_line1[17], display_line2[17];
    int read_count = 0;
    int error_count = 0;
    
    while (1)
    {
        // 读取DHT11温湿度数据
        if(DHT11_ReadData(&temperature, &humidity) == 0)  // 读取成功
        {
            read_count++;
            error_count = 0;  // 重置错误计数
            
            // 清屏
            lcd_clear();
            
            // 格式化显示字符串
            sprintf(display_line1, "T:%dC  H:%d%%", temperature, humidity);
            sprintf(display_line2, "Read: %d times", read_count);
            
            // 在LCD上显示温湿度和读取次数
            lcd_print_str(0, 0, display_line1);
            lcd_print_str(1, 0, display_line2);
            
            // LED缓慢闪烁表示正常工作
            Led_On(LED0);
            Simple_Delay_Ms(100);
            Led_Off(LED0);
            
            // 数据更新间隔（DHT11建议至少2秒间隔）
            Simple_Delay_Ms(1900);
        }
        else  // 读取失败
        {
            error_count++;
            
            // 清屏
            lcd_clear();
            
            // 显示错误信息
            sprintf(display_line1, "DHT11 Error!");
            sprintf(display_line2, "Retry: %d", error_count);
            
            lcd_print_str(0, 0, display_line1);
            lcd_print_str(1, 0, display_line2);
            
            // LED快速闪烁表示错误
            for(int i = 0; i < 3; i++) {
                Led_Toggle(LED1);
                Simple_Delay_Ms(200);
            }
            
            // 如果连续错误太多，重新初始化DHT11
            if(error_count >= 5) {
                lcd_clear();
                lcd_print_str(0, 0, "Reinit DHT11...");
                DHT11_Init();
                Simple_Delay_Ms(1000);
                error_count = 0;
            }
            
            // 错误后稍短的延时重试
            Simple_Delay_Ms(1000);
        }
    }
}
