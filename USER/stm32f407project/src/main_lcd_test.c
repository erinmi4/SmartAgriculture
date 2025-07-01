#include "stm32f4xx.h"
#include "led.h"
#include "lcd.h"
#include "delay.h"

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
        Led_On(LED0);
        Simple_Delay_Ms(200);
        Led_Off(LED0);
        Simple_Delay_Ms(200);
    }
    
    // 初始化LCD
    lcd_init();
    
    // LCD初始化延时
    Simple_Delay_Ms(100);
    
    // 清屏
    lcd_clear();
    
    // 显示"mika"在第一行
    lcd_print_str(0, 0, "mika");
    
    // 显示一些测试信息在第二行
    lcd_print_str(1, 0, "LCD OK");
    
    // LED快速闪烁5次表示LCD初始化完成
    for(int i = 0; i < 5; i++) {
        Led_Toggle(LED1);
        Simple_Delay_Ms(100);
    }
    
    // 主循环
    int counter = 0;
    while (1)
    {
        // LED慢闪表示程序正常运行
        Led_Toggle(LED0);
        Simple_Delay_Ms(1000);
        
        // 每10秒更新一次显示
        counter++;
        if (counter >= 10) {
            counter = 0;
            lcd_clear();
            lcd_print_str(0, 0, "mika");
            lcd_print_str(1, 0, "Running...");
            Simple_Delay_Ms(2000);
            
            lcd_clear();
            lcd_print_str(0, 0, "Hello World!");
            lcd_print_str(1, 0, "Test OK");
            Simple_Delay_Ms(2000);
        }
    }
}
