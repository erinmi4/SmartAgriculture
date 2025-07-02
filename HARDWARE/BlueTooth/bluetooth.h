#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "stm32f4xx.h"

/* 蓝牙配置参数 */
#define BLUETOOTH_UART          USART2
#define BLUETOOTH_BAUDRATE      9600

/* 蓝牙状态枚举 */
typedef enum {
    BT_STATUS_IDLE = 0,
    BT_STATUS_RECEIVING,
    BT_STATUS_CMD_READY
} BluetoothStatus_t;

/* 函数声明 */
void Bluetooth_Init(void);                          // 蓝牙模块初始化
void Bluetooth_SendString(char* str);               // 发送字符串
void Bluetooth_SendData(uint8_t* data, uint16_t len); // 发送数据
void Bluetooth_ProcessCommand(void);                // 处理接收到的命令（简化版）
BluetoothStatus_t Bluetooth_GetStatus(void);        // 获取蓝牙状态
void Bluetooth_SetThreshold(uint16_t threshold);    // 设置烟雾阈值
uint16_t Bluetooth_GetThreshold(void);              // 获取当前阈值
void Bluetooth_SendStatus(uint16_t smoke_value);    // 发送当前状态（简化版）
void Bluetooth_ProcessRxData(uint8_t ch);           // 处理接收数据
char* Bluetooth_GetReceivedCommand(void);           // 获取接收到的完整命令

#endif /* __BLUETOOTH_H */
