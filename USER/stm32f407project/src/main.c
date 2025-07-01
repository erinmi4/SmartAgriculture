

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
    
    // 清屏
    lcd_clear();
    
    // 显示"mika"
    lcd_print_str(0, 0, "mika");
    
    // LED快速闪烁表示LCD初始化完成
    for(int i = 0; i < 5; i++) {
        Led_Toggle(LED0);
        Simple_Delay_Ms(100);
    }

    while (1)
    {
        // 主循环中继续闪烁LED表示系统运行
        Led_Toggle(LED0);
        Simple_Delay_Ms(1000);
    }
}

