#include "mq2.h"
#include "uart.h"
#include "delay.h"

/* MQ-2传感器命令帧 */
static unsigned char mq2_cmd[MQ2_CMD_FRAME_SIZE] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};

/* MQ-2传感器数据变量 */
static unsigned int mq2_value = 0;         // 烟雾浓度值
static unsigned char mq2_flag = 0;         // 数据接收完成标志

/* 串口接收相关变量 */
static unsigned char recv_count = 0;        // 接收计数器
static unsigned char recv_buf[MQ2_RESP_FRAME_SIZE] = {0};  // 接收缓冲区

/**
 * @brief  MQ-2传感器初始化
 * @param  None
 * @retval None
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
 * @param  None
 * @retval None
 */
void MQ2_SendCommand(void)
{
    int i;
    for(i = 0; i < MQ2_CMD_FRAME_SIZE; i++)
    {
        UART_SendChar(USART3, mq2_cmd[i]);
    }
}

/**
 * @brief  获取烟雾浓度值
 * @param  None
 * @retval 烟雾浓度值
 */
unsigned int MQ2_GetValue(void)
{
    return mq2_value;
}

/**
 * @brief  检查数据是否就绪
 * @param  None
 * @retval 1-数据就绪, 0-数据未就绪
 */
unsigned char MQ2_IsDataReady(void)
{
    return mq2_flag;
}

/**
 * @brief  清除数据就绪标志
 * @param  None
 * @retval None
 */
void MQ2_ClearFlag(void)
{
    mq2_flag = 0;
}

/**
 * @brief  MQ-2传感器数据处理函数
 * @param  ch: 接收到的字符
 * @retval None
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
    if(recv_count == MQ2_RESP_FRAME_SIZE)
    {
        /* 接收完了一个完整的数据帧 */
        mq2_flag = 1;  // 表示本次检测操作完成
        
        /* 解析烟雾浓度值 (高字节在前，低字节在后) */
        mq2_value = (recv_buf[2] << 8) | recv_buf[3];
        
        /* 重置接收计数器 */
        recv_count = 0;
    }
}
