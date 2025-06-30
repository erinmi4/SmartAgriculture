#include "stm32f4xx.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "exti.h"
#include "uart.h"
#include "lcd.h"
#include "delay.h"

int main(void)
{    
    // 初始化LED灯
    Led_Init();
    // 点亮LED0
    Led_On(0);      // 点亮LED0
    
    // 初始化蜂鸣器
    Beep_Init();

    //Beep_On();  // 打开蜂鸣器
    
    // 初始化串口1，波特率115200
    //UART1_Init(UART_BAUD_115200);
    
    // // 关闭所有LED和蜂鸣器(初始状态)
    // Led_Off(0);
    // Led_Off(1);
    // Led_Off(2);
    // Led_Off(3);
    // Beep_Off();

    // 初始化LCD
    lcd_init();
    
    // 在LCD上显示信息
    lcd_clear();                    // 清屏

    lcd_gotoxy(0, 0);              // 定位到第一行第一列
    lcd_print_str("Mika");   // 显示 名字
    
    lcd_gotoxy(1, 0);              // 定位到第二行第一列
    lcd_print_str("202205570105");  // 显示学号
    
    while (1)
    {

    }
}



