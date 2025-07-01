#include "bluetooth.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* 蓝牙模块私有变量 */
static char bt_recv_buffer[64];         // 接收缓冲区
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
    smoke_threshold = 300;  // 默认阈值
    memset(bt_recv_buffer, 0, sizeof(bt_recv_buffer));
    
    // 发送初始化完成消息
    Bluetooth_SendString("BT Ready!\r\n");
    Bluetooth_SendString("Commands:\r\n");
    Bluetooth_SendString("1=100ppm 2=200ppm 3=300ppm\r\n");
    Bluetooth_SendString("4=400ppm 5=500ppm 0=query 9=status\r\n");
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
 * @note   此函数由uart.c中的中断处理函数调用
 */
void Bluetooth_ProcessRxData(uint8_t ch)
{
    // 简化处理：直接处理单字符命令
    bt_status = BT_STATUS_CMD_READY;
    bt_recv_buffer[0] = ch;
    bt_recv_buffer[1] = '\0';
}

/**
 * @brief  处理蓝牙命令
 * @param  None
 * @retval None
 */
void Bluetooth_ProcessCommand(void)
{
    if(bt_status != BT_STATUS_CMD_READY)
        return;
    
    char response[64];
    char cmd = bt_recv_buffer[0];
    
    // 处理简化的单字符命令
    switch(cmd)
    {
        case BT_CMD_SET_THR_100:  // '1' - 设置阈值100ppm
            smoke_threshold = 100;
            sprintf(response, "Threshold: 100ppm\r\n");
            break;
            
        case BT_CMD_SET_THR_200:  // '2' - 设置阈值200ppm
            smoke_threshold = 200;
            sprintf(response, "Threshold: 200ppm\r\n");
            break;
            
        case BT_CMD_SET_THR_300:  // '3' - 设置阈值300ppm
            smoke_threshold = 300;
            sprintf(response, "Threshold: 300ppm\r\n");
            break;
            
        case BT_CMD_SET_THR_400:  // '4' - 设置阈值400ppm
            smoke_threshold = 400;
            sprintf(response, "Threshold: 400ppm\r\n");
            break;
            
        case BT_CMD_SET_THR_500:  // '5' - 设置阈值500ppm
            smoke_threshold = 500;
            sprintf(response, "Threshold: 500ppm\r\n");
            break;
            
        case BT_CMD_GET_THRESHOLD:  // '0' - 获取当前阈值
            sprintf(response, "Current: %dppm\r\n", smoke_threshold);
            break;
            
        case BT_CMD_GET_STATUS:     // '9' - 获取状态
            sprintf(response, "Status request OK\r\n");
            break;
            
        default:
            sprintf(response, "Invalid cmd: %c\r\nUse: 1-5,0,9\r\n", cmd);
            break;
    }
    
    Bluetooth_SendString(response);
    
    // 重置接收状态
    bt_status = BT_STATUS_IDLE;
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
 */
void Bluetooth_SendStatus(uint16_t smoke_value)
{
    char status_msg[128];
    char* status_text;
    
    // 根据当前值和阈值判断状态
    if(smoke_value < smoke_threshold * 0.5)
        status_text = "NORMAL";
    else if(smoke_value < smoke_threshold)
        status_text = "CAUTION";
    else
        status_text = "ALARM";
    
    sprintf(status_msg, 
            "STATUS: %s\r\n"
            "Smoke: %d ppm\r\n" 
            "Threshold: %d ppm\r\n"
            "Ratio: %.1f%%\r\n\r\n",
            status_text,
            smoke_value, 
            smoke_threshold,
            (float)smoke_value * 100.0f / (float)smoke_threshold);
    
    Bluetooth_SendString(status_msg);
}
