#ifndef __LCD_H
#define __LCD_H

#include "stm32f4xx.h"

// 定义 LCD1602 的控制引脚
#define RS_PIN       GPIO_Pin_7
#define RW_PIN       GPIO_Pin_6
#define EN_PIN       GPIO_Pin_4

// 定义控制引脚所在的 GPIO 端口
#define RS_GPIO_PORT  GPIOB
#define RW_GPIO_PORT  GPIOD
#define EN_GPIO_PORT  GPIOA

// 定义数据总线引脚
#define D0_PIN       GPIO_Pin_7
#define D1_PIN       GPIO_Pin_15
#define D2_PIN       GPIO_Pin_6
#define D3_PIN       GPIO_Pin_7
#define D4_PIN       GPIO_Pin_8
#define D5_PIN       GPIO_Pin_9
#define D6_PIN       GPIO_Pin_11
#define D7_PIN       GPIO_Pin_6

// 定义数据总线所在的 GPIO 端口
#define D0_GPIO_PORT  GPIOD
#define D1_GPIO_PORT  GPIOG
#define D2_GPIO_PORT  GPIOC
#define D3_GPIO_PORT  GPIOC
#define D4_GPIO_PORT  GPIOC
#define D5_GPIO_PORT  GPIOC
#define D6_GPIO_PORT  GPIOC
#define D7_GPIO_PORT  GPIOB

// 初始化 LCD1602 的 GPIO 引脚
void lcd_gpio_init(void);

// 写指令到 LCD1602
void lcd_write_cmd(unsigned char cmd);

// 写数据到 LCD1602
void lcd_write_dat(unsigned char dat);

// 初始化 LCD1602
void lcd_init(void);

// 设置光标位置
void lcd_gotoxy(unsigned char line, unsigned char column);

// 在 LCD1602 上显示字符串
void lcd_print_str(unsigned char line, unsigned char column, const char *str);

// LCD调试和测试函数
void lcd_debug_test(void);
void lcd_clear(void);

#endif

