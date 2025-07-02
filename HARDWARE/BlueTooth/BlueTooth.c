#include "bluetooth.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* 蓝牙模块私有变量 */
static char bt_recv_buffer[64];         // 接收缓冲区
static uint8_t bt_recv_index = 0;       // 接收索引
static BluetoothStatus_t bt_status = BT_STATUS_IDLE;
static uint16_t smoke_threshold = 300;  // 默认烟雾阈值300ppm

/**
 * @brief  蓝牙模块初始化
 * @param  None
 * @retval None
 * @note   使用USART2作为蓝牙通信接口
 */
void Bluetooth_Init(void)
{
    // 蓝牙模块使用USART2，通过uart.c中的UART2_Init初始化
    // 这里只需要初始化蓝牙相关的变量
    bt_status = BT_STATUS_IDLE;
    bt_recv_index = 0;
    smoke_threshold = 300;  // 默认阈值
    memset(bt_recv_buffer, 0, sizeof(bt_recv_buffer));
    
    // 发送初始化完成消息
    Bluetooth_SendString("BT Ready!\r\n");
}

/**
 * @brief  通过蓝牙发送字符串
 * @param  str: 要发送的字符串
 * @retval None
 */
void Bluetooth_SendString(char* str)
{
    while(*str)
    {
        // 等待发送缓冲区空
        while(USART_GetFlagStatus(BLUETOOTH_UART, USART_FLAG_TXE) == RESET);
        USART_SendData(BLUETOOTH_UART, *str++);
    }
}

/**
 * @brief  通过蓝牙发送数据
 * @param  data: 要发送的数据
 * @param  len: 数据长度
 * @retval None
 */
void Bluetooth_SendData(uint8_t* data, uint16_t len)
{
    for(uint16_t i = 0; i < len; i++)
    {
        while(USART_GetFlagStatus(BLUETOOTH_UART, USART_FLAG_TXE) == RESET);
        USART_SendData(BLUETOOTH_UART, data[i]);
    }
}

/**
 * @brief  处理接收到的蓝牙数据
 * @param  ch: 接收到的字符
 * @retval None
 * @note   此函数由uart.c中的中断处理函数调用，支持多字符命令
 */
void Bluetooth_ProcessRxData(uint8_t ch)
{
    // 回显字符
    while(USART_GetFlagStatus(BLUETOOTH_UART, USART_FLAG_TXE) == RESET);
    USART_SendData(BLUETOOTH_UART, ch);
    
    // 处理特殊字符
    if(ch == '\r' || ch == '\n')
    {
        // 命令结束
        if(bt_recv_index > 0)
        {
            bt_recv_buffer[bt_recv_index] = '\0';
            bt_status = BT_STATUS_CMD_READY;
        }
        // 发送换行
        Bluetooth_SendString("\r\n");
        return;
    }
    else if(ch == '\b' || ch == 127) // 退格键
    {
        if(bt_recv_index > 0)
        {
            bt_recv_index--;
            bt_recv_buffer[bt_recv_index] = '\0';
            // 发送退格效果
            Bluetooth_SendString(" \b");
        }
        return;
    }
    
    // 添加字符到缓冲区
    if(bt_recv_index < sizeof(bt_recv_buffer) - 1)
    {
        bt_recv_buffer[bt_recv_index++] = ch;
        bt_recv_buffer[bt_recv_index] = '\0';
    }
}

/**
 * @brief  获取接收到的命令
 * @param  None
 * @retval 接收到的命令字符串
 */
char* Bluetooth_GetReceivedCommand(void)
{
    return bt_recv_buffer;
}

/**
 * @brief  处理蓝牙命令
 * @param  None
 * @retval None
 * @note   简化版本，只重置状态，具体命令处理由main.c完成
 */
void Bluetooth_ProcessCommand(void)
{
    // 重置接收状态和缓冲区
    bt_status = BT_STATUS_IDLE;
    bt_recv_index = 0;
    memset(bt_recv_buffer, 0, sizeof(bt_recv_buffer));
}

/**
 * @brief  获取蓝牙状态
 * @param  None
 * @retval 蓝牙状态
 */
BluetoothStatus_t Bluetooth_GetStatus(void)
{
    return bt_status;
}

/**
 * @brief  设置烟雾阈值
 * @param  threshold: 新的阈值
 * @retval None
 */
void Bluetooth_SetThreshold(uint16_t threshold)
{
    if(threshold >= 50 && threshold <= 2000)
    {
        smoke_threshold = threshold;
    }
}

/**
 * @brief  获取当前烟雾阈值
 * @param  None
 * @retval 当前阈值
 */
uint16_t Bluetooth_GetThreshold(void)
{
    return smoke_threshold;
}

/**
 * @brief  发送当前系统状态
 * @param  smoke_value: 当前烟雾浓度值
 * @retval None
 * @note   简化版本，详细状态由main.c中的SendSystemStatus()处理
 */
void Bluetooth_SendStatus(uint16_t smoke_value)
{
    char status_msg[64];
    
    sprintf(status_msg, "Smoke: %d ppm\r\n", smoke_value);
    Bluetooth_SendString(status_msg);
}
