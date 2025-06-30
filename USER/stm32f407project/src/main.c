#include "stm32f4xx.h"
#include "led.h"
#include "key.h"
#include "beep.h"
#include "exti.h"
#include "uart.h"
#include "lcd.h"
#include "delay.h"
#include "DHT11.h"  // 添加DHT11头文件
#include <stdio.h>  // 添加stdio.h用于sprintf函数

/**
 * @brief 安全延时函数（毫秒级）
 * @param ms: 延时毫秒数
 * @retval 无
 * @note 避免使用可能冲突的延时函数
 */
static void safe_delay_ms(uint32_t ms)
{
    uint32_t i, j;
    for(i = 0; i < ms; i++)
    {
        for(j = 0; j < 168000; j++)  // 根据168MHz系统时钟调整
        {
            __NOP();
        }
    }
}

int main(void)
{    
    unsigned char temp_str[16] = {0};  // 温度显示字符串缓冲区
    unsigned char humi_str[16] = {0};  // 湿度显示字符串缓冲区
    unsigned char dht11_status = 0;  // DHT11读取状态
    
    // 初始化LED灯
    Led_Init();
    // 点亮LED0
    Led_On(0);      // 点亮LED0
    
    // 初始化蜂鸣器
    Beep_Init();
    
    // 初始化LCD
    lcd_init();
    
    // 初始化DHT11
    dht11_status = dht11_init();
    
    // 在LCD上显示初始化信息
    lcd_clear();                    // 清屏
    
    if(dht11_status == 0)
    {
        lcd_gotoxy(0, 0);              // 定位到第一行第一列
        lcd_print_str((unsigned char*)"DHT11 Ready!");   // 显示DHT11准备就绪
    }
    else
    {
        lcd_gotoxy(0, 0);              // 定位到第一行第一列
        lcd_print_str((unsigned char*)"DHT11 Error!");   // 显示DHT11错误
    }
    
    safe_delay_ms(2000);            // 延时2秒显示初始化状态
    
    while (1)
    {
        // 读取DHT11温湿度数据
        dht11_status = dht11_read_data();
        
        lcd_clear();                // 清屏
        
        if(dht11_status == 0)       // 读取成功
        {
            // 格式化温度字符串
            sprintf((char*)temp_str, "Temp: %d C", dht11_temp);
            
            // 格式化湿度字符串  
            sprintf((char*)humi_str, "Humi: %d %%RH", dht11_humi);
            
            // 在LCD第一行显示温度
            lcd_gotoxy(0, 0);
            lcd_print_str(temp_str);
            
            // 在LCD第二行显示湿度
            lcd_gotoxy(1, 0);
            lcd_print_str(humi_str);
            
            // LED0闪烁表示正常工作
            Led_Toggle(0);
        }
        else                        // 读取失败
        {
            // 显示错误信息
            lcd_gotoxy(0, 0);
            lcd_print_str((unsigned char*)"Read Error!");
            
            lcd_gotoxy(1, 0);
            lcd_print_str((unsigned char*)"Check DHT11");
            
            // LED0常亮表示错误
            Led_On(0);
        }
        
        // 延时2秒后再次读取
        safe_delay_ms(2000);
    }
}



