/**
 * @file    DHT11.h
 * @brief   DHT11温湿度传感器驱动头文件
 * @author  Mika
 * @date    2025-07-01
 * @version 3.0
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
#define DHT11_DQ_OUT    GPIO_SetBits(DHT11_PORT, DHT11_IO)     // 拉高数据线
#define DHT11_DQ_LOW    GPIO_ResetBits(DHT11_PORT, DHT11_IO)   // 拉低数据线
#define DHT11_DQ_IN     GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO)  // 读取数据线状态

// 函数声明
void DHT11_IO_Output(void);                         // 配置DHT11引脚为输出模式
void DHT11_IO_Input(void);                          // 配置DHT11引脚为输入模式
void DHT11_Start(void);                             // 启动DHT11通信
unsigned char DHT11_Check(void);                    // 检查DHT11响应
unsigned char DHT11_ReadBit(void);                  // 读取一个位
unsigned char DHT11_ReadByte(void);                 // 读取一个字节
void DHT11_Init(void);                              // DHT11初始化
unsigned char DHT11_ReadData(unsigned char *temp, unsigned char *humi);  // 读取温湿度数据

#endif /* __DHT11_H */
