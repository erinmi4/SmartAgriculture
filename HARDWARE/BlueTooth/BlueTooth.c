#include "bluetooth.h"
#include <string.h>
#include <stdio.h>

/* 蓝牙模块私有变量 */
static char bt_recv_buffer[64];         // 接收缓冲区
static uint8_t bt_recv_index = 0;       // 接收索引

/* 外部变量（在main.c中定义）*/
extern char bt_command_buffer[20];      // 命令缓冲区
extern uint8_t bt_command_ready;        // 命令就绪标志

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
    memset(bt_recv_buffer, 0, sizeof(bt_recv_buffer));
    bt_recv_index = 0;
    
    // 发送初始化完成消息
    Bluetooth_SendString("BT Ready! Use: CMD:VALUE\r\n");
    Bluetooth_SendString("TH:30 TL:20 HH:70 HL:40 LL:40 SH:150\r\n");
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
 * @brief  处理接收到的蓝牙数据（由UART中断调用）
 * @param  ch: 接收到的字符
 * @retval None
 * @note   此函数由uart.c中的中断处理函数调用
 */
void Bluetooth_ProcessRxData(uint8_t ch)
{
    // 处理参数化命令接收
    if(ch == '\r' || ch == '\n')
    {
        if(bt_recv_index > 0)
        {
            bt_recv_buffer[bt_recv_index] = '\0';
            
            // 复制到命令缓冲区
            if(bt_recv_index < 20)
            {
                strcpy(bt_command_buffer, bt_recv_buffer);
                bt_command_ready = 1;
            }
            
            // 重置接收缓冲区
            bt_recv_index = 0;
            memset(bt_recv_buffer, 0, sizeof(bt_recv_buffer));
        }
    }
    else if(bt_recv_index < sizeof(bt_recv_buffer) - 1)
    {
        bt_recv_buffer[bt_recv_index++] = ch;
    }
}

/**
 * @brief  处理蓝牙命令（兼容性函数，主要由main.c处理）
 * @param  None
 * @retval None
 */
void Bluetooth_ProcessCommand(void)
{
    // 这个函数为了兼容性保留，实际命令处理在main.c中进行
}
