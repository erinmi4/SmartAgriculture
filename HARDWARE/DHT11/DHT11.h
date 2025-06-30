/**
 * @file    DHT11.h
 * @brief   DHT11温湿度传感器驱动头文件
 * @author  Mika
 * @date    2025-06-30
 * @version 1.0
 * 
 * @note    DHT11数字温湿度传感器驱动
 *          使用PE5引脚连接DHT11的DATA引脚
 *          支持温度和湿度读取
 */

#ifndef __DHT11_H
#define __DHT11_H

#include "stm32f4xx.h"

// DHT11引脚定义 - PE5
#define DHT11_PORT      GPIOE
#define DHT11_IO        GPIO_Pin_5
#define DHT11_RCC       RCC_AHB1Periph_GPIOE

// DHT11引脚操作宏定义
#define DHT11_DQ_OUT    GPIO_WriteBit(DHT11_PORT, DHT11_IO, Bit_SET)   // 拉高数据线
#define DHT11_DQ_LOW    GPIO_WriteBit(DHT11_PORT, DHT11_IO, Bit_RESET) // 拉低数据线
#define DHT11_DQ_IN     GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO)    // 读取数据线状态

// 全局变量声明
extern unsigned char dht11_temp;    // 温度值
extern unsigned char dht11_humi;    // 湿度值

// 函数声明
void dht11_io_out(void);            // 配置DHT11引脚为输出模式
void dht11_io_in(void);             // 配置DHT11引脚为输入模式
unsigned char dht11_init(void);     // DHT11初始化
void dht11_start(void);             // 发送开始信号
unsigned char dht11_check(void);    // 等待DHT11响应
unsigned char dht11_read_bit(void); // 读取一个位
unsigned char dht11_read_byte(void);// 读取一个字节
unsigned char dht11_read_data(void);// 读取温湿度数据

#endif /* __DHT11_H */
