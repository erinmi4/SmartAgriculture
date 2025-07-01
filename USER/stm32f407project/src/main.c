/**
 * @file    main.c
 * @brief   MQ-2烟雾传感器主程序
 * @details 实时获取环境烟雾数据并显示在LCD1602上
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "uart.h"
#include "lcd.h"
#include <stdio.h>

// MQ-2传感器命令帧
unsigned char mq2_cmd[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

// MQ-2传感器数据变量
static unsigned int mq2_value = 0;         // 烟雾浓度值
static unsigned char mq2_flag = 0;         // 数据接收完成标志

// 串口接收相关变量
static unsigned char recv_count = 0;        // 接收计数器
static unsigned char recv_buf[9] = {0};     // 接收缓冲区

// 函数声明
void MQ2_Init(void);
void MQ2_SendCommand(void);
unsigned int MQ2_GetValue(void);
unsigned char MQ2_IsDataReady(void);
void MQ2_ClearFlag(void);
void MQ2_ProcessData(uint8_t ch);

/**
 * @brief  MQ-2传感器初始化
 */
void MQ2_Init(void)
{
    UART_ConfigTypeDef uart_config;
    
    /* 配置USART3参数 */
    uart_config.USARTx = USART3;
    uart_config.BaudRate = UART_BAUD_9600;
    uart_config.Parity = UART_PARITY_NONE;
    uart_config.StopBits = UART_STOPBIT_1;
    uart_config.DataBits = UART_DATABIT_8;
    uart_config.FlowControl = UART_FLOWCTRL_NONE;
    uart_config.RxIntEnable = ENABLE;
    uart_config.PreemptionPriority = 2;
    uart_config.SubPriority = 3;
    
    /* 初始化USART3 */
    UART_Init(&uart_config);
    
    /* 初始化变量 */
    mq2_value = 0;
    mq2_flag = 0;
    recv_count = 0;
}

/**
 * @brief  发送命令到MQ-2传感器
 */
void MQ2_SendCommand(void)
{
    int i;
    for(i = 0; i < 9; i++)
    {
        UART_SendChar(USART3, mq2_cmd[i]);
    }
}

/**
 * @brief  获取烟雾浓度值
 */
unsigned int MQ2_GetValue(void)
{
    return mq2_value;
}

/**
 * @brief  检查数据是否就绪
 */
unsigned char MQ2_IsDataReady(void)
{
    return mq2_flag;
}

/**
 * @brief  清除数据就绪标志
 */
void MQ2_ClearFlag(void)
{
    mq2_flag = 0;
}

/**
 * @brief  MQ-2传感器数据处理函数
 */
void MQ2_ProcessData(uint8_t ch)
{
    /* 检查是否为帧头 */
    if(ch == 0xFF)
    {
        recv_count = 0;
    }
    
    /* 存储接收数据 */
    recv_buf[recv_count] = ch;
    recv_count++;
    
    /* 检查是否接收完一帧数据 */
    if(recv_count == 9)
    {
        /* 接收完了一个完整的数据帧 */
        mq2_flag = 1;  // 表示本次检测操作完成
        
        /* 解析烟雾浓度值 (高字节在前，低字节在后) */
        mq2_value = (recv_buf[2] << 8) | recv_buf[3];
        
        /* 重置接收计数器 */
        recv_count = 0;
    }
}

int main()
{
    // 系统初始化
    SystemInit();
    
    // 初始化MQ-2传感器
    MQ2_Init();
    
    // 初始化LCD
    lcd_init();
    
    // 显示启动信息
    lcd_clear();
    lcd_print_str(0, 0, "MQ-2 Starting...");
    Mdelay_Lib(2000);  // 使用delay.c中实际存在的函数
    
    while(1)
    {
        // 发送命令到MQ-2传感器
        MQ2_SendCommand();

        // 等待数据接收完成
        while(MQ2_IsDataReady() == 0)
        {
            Mdelay_Lib(10); // 使用正确的延时函数
        }

        // 获取烟雾浓度值
        unsigned int smoke_value = MQ2_GetValue();
        
        // 在LCD上显示烟雾浓度
        char str[32] = {0};
        sprintf(str, "Smoke:%4d ppm", smoke_value);
        lcd_print_str(0, 0, str);
        
        // 根据浓度显示状态
        if(smoke_value < 100)
        {
            lcd_print_str(1, 0, "Status: Normal  ");
        }
        else if(smoke_value < 300)
        {
            lcd_print_str(1, 0, "Status: Low     ");
        }
        else if(smoke_value < 500)
        {
            lcd_print_str(1, 0, "Status: Medium  ");
        }
        else
        {
            lcd_print_str(1, 0, "Status: HIGH!!! ");
        }
        
        // 清除数据就绪标志
        MQ2_ClearFlag();
        
        Mdelay_Lib(2000);  // 使用正确的延时函数
    }
}
