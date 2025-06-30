#ifndef __LCD_H
#define __LCD_H

#include "stm32f4xx.h"
#include "sys.h"

/**
 * LCD1602引脚连接定义
 * 使用的是板上的camera电路
 * 
 * LCD引脚 -> STM32引脚
 * VSS -> GND
 * VDD -> 3.3V
 * V0  -> GND (对比度调节，接GND为最大对比度)
 * RS  -> PB7  (寄存器选择：0=指令寄存器，1=数据寄存器)
 * RW  -> PD6  (读写选择：0=写，1=读，这里只用写操作)
 * E   -> PA4  (使能信号：高电平有效)
 * D0  -> PD7  (数据位0)
 * D1  -> PG15 (数据位1)
 * D2  -> PC6  (数据位2)
 * D3  -> PC7  (数据位3)
 * D4  -> PC8  (数据位4)
 * D5  -> PC9  (数据位5)
 * D6  -> PC11 (数据位6)
 * D7  -> PB6  (数据位7)
 * A   -> +5V (背光正极)
 * K   -> GND (背光负极)
 */

// 控制引脚定义
#define LCD_RS_PORT     GPIOB
#define LCD_RS_PIN      GPIO_Pin_7

#define LCD_RW_PORT     GPIOD
#define LCD_RW_PIN      GPIO_Pin_6

#define LCD_EN_PORT     GPIOA
#define LCD_EN_PIN      GPIO_Pin_4

// 数据引脚定义
#define LCD_D0_PORT     GPIOD
#define LCD_D0_PIN      GPIO_Pin_7

#define LCD_D1_PORT     GPIOG
#define LCD_D1_PIN      GPIO_Pin_15

#define LCD_D2_PORT     GPIOC
#define LCD_D2_PIN      GPIO_Pin_6

#define LCD_D3_PORT     GPIOC
#define LCD_D3_PIN      GPIO_Pin_7

#define LCD_D4_PORT     GPIOC
#define LCD_D4_PIN      GPIO_Pin_8

#define LCD_D5_PORT     GPIOC
#define LCD_D5_PIN      GPIO_Pin_9

#define LCD_D6_PORT     GPIOC
#define LCD_D6_PIN      GPIO_Pin_11

#define LCD_D7_PORT     GPIOB
#define LCD_D7_PIN      GPIO_Pin_6

// LCD1602常用指令定义
#define LCD_CMD_CLEAR           0x01    // 清屏
#define LCD_CMD_HOME            0x02    // 光标回到原点
#define LCD_CMD_ENTRY_MODE      0x06    // 设置输入模式：光标右移，显示器不移动
#define LCD_CMD_DISPLAY_CTRL    0x0C    // 显示控制：显示开，光标关，闪烁关
#define LCD_CMD_CURSOR_SHIFT    0x10    // 光标移动
#define LCD_CMD_FUNCTION_SET    0x38    // 功能设置：8位数据，2行显示，5x7字符
#define LCD_CMD_SET_CGRAM       0x40    // 设置字符发生器RAM地址
#define LCD_CMD_SET_DDRAM       0x80    // 设置显示数据RAM地址

// 行地址定义
#define LCD_LINE1_ADDR          0x80    // 第一行起始地址
#define LCD_LINE2_ADDR          0xC0    // 第二行起始地址

// 函数声明
void lcd_gpio_init(void);               // GPIO初始化
void lcd_write_cmd(unsigned char cmd);  // 写指令
void lcd_write_dat(unsigned char dat);  // 写数据
void lcd_init(void);                    // LCD初始化
void lcd_gotoxy(unsigned char line, unsigned char column);  // 设置光标位置
void lcd_print_str(unsigned char *str); // 显示字符串
void lcd_print_char(unsigned char ch);  // 显示单个字符
void lcd_clear(void);                   // 清屏

#endif /* __LCD_H */
