/**
 * @file    main.c
 * @brief   DHT11温湿度传感器主程序
 * @details 实时获取环境温湿度数据并显示在LCD1602上
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "DHT11.h"
#include <stdio.h>

int main(void)
{
    unsigned char temp, humi;
    char str[32];

    // 初始化DHT11和LCD1602
    dht11_init();
    lcd_init();

    while (1)
    {
        if (dht11_read_dat(&temp, &humi) == 0) // 成功读取数据
        {
            // 显示温度
            lcd_gotoxy(0, 0); // 设置LCD光标位置
            sprintf(str, "Temp: %d C", temp); // 格式化字符串
            lcd_print_str(0, 0, str); // 显示温度

            // 显示湿度
            lcd_gotoxy(1, 0); // 设置LCD光标位置
            sprintf(str, "Humi: %d %%", humi); // 格式化字符串
            lcd_print_str(1, 0, str); // 显示湿度
        }
        else
        {
            // 如果读取失败，显示错误信息
            lcd_gotoxy(0, 0);
            lcd_print_str(0, 0, "Read Error");
            lcd_gotoxy(1, 0);
            lcd_print_str(1, 0, "Check DHT11");
        }

        Mdelay_Lib(2000); // 每2秒读取一次
    }
}