#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "stm32f4xx.h"

/* 蓝牙配置参数 */
#define BLUETOOTH_UART          USART2
#define BLUETOOTH_BAUDRATE      9600

/* 函数声明 */
void Bluetooth_Init(void);                          // 蓝牙模块初始化
void Bluetooth_SendString(char* str);               // 发送字符串
void Bluetooth_SendData(uint8_t* data, uint16_t len); // 发送数据
void Bluetooth_ProcessCommand(void);                // 处理接收到的命令
void Bluetooth_ProcessRxData(uint8_t ch);           // 处理接收数据（由UART中断调用）

#endif /* __BLUETOOTH_H */
