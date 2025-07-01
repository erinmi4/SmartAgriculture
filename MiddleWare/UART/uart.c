#include "uart.h"
#include <stdarg.h>
#include <stdio.h>

/**
 * @file    uart.c
 * @brief   UART通信模块源文件
 * @details 实现STM32F4xx系列MCU串口通信相关函数
 * @author  龚维学
 * @date    2025-06-28
 * @version 1.0
 */

/* 声明 fputc 函数（用于重定向 printf 输出） */
int fputc(int ch, FILE *f);

/**
 * @brief  串口1初始化函数
 * @param  baudrate: 波特率，如UART_BAUD_115200
 * @retval None
 * @note   此函数配置串口1为8位数据，无校验，1位停止位
 */
void UART1_Init(UART_BaudRateTypeDef baudrate)
{
    UART_ConfigTypeDef config;
    
    config.USARTx = USART1;
    config.BaudRate = baudrate;
    config.Parity = UART_PARITY_NONE;
    config.StopBits = UART_STOPBIT_1;
    config.DataBits = UART_DATABIT_8;
    config.FlowControl = UART_FLOWCTRL_NONE;
    config.RxIntEnable = ENABLE;
    config.PreemptionPriority = 1;
    config.SubPriority = 1;
    
    UART_Init(&config);
}

/**
 * @brief  串口2初始化函数
 * @param  baudrate: 波特率，如UART_BAUD_115200
 * @retval None
 * @note   此函数配置串口2为8位数据，无校验，1位停止位
 */
void UART2_Init(UART_BaudRateTypeDef baudrate)
{
    UART_ConfigTypeDef config;
    
    config.USARTx = USART2;
    config.BaudRate = baudrate;
    config.Parity = UART_PARITY_NONE;
    config.StopBits = UART_STOPBIT_1;
    config.DataBits = UART_DATABIT_8;
    config.FlowControl = UART_FLOWCTRL_NONE;
    config.RxIntEnable = ENABLE;
    config.PreemptionPriority = 2;
    config.SubPriority = 2;
    
    UART_Init(&config);
}

/**
 * @brief  串口3初始化函数
 * @param  baudrate: 波特率，如UART_BAUD_115200
 * @retval None
 * @note   此函数配置串口3为8位数据，无校验，1位停止位
 */
void UART3_Init(UART_BaudRateTypeDef baudrate)
{
    UART_ConfigTypeDef config;
    
    config.USARTx = USART3;
    config.BaudRate = baudrate;
    config.Parity = UART_PARITY_NONE;
    config.StopBits = UART_STOPBIT_1;
    config.DataBits = UART_DATABIT_8;
    config.FlowControl = UART_FLOWCTRL_NONE;
    config.RxIntEnable = ENABLE;
    config.PreemptionPriority = 2;
    config.SubPriority = 3;
    
    UART_Init(&config);
}

/**
 * @brief  通用串口初始化函数
 * @param  config: 串口配置结构体指针
 * @retval None
 * @note   此函数可配置任意串口的详细参数
 */
void UART_Init(UART_ConfigTypeDef *config)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    /* 根据不同的UART确定不同的引脚和时钟 */
    GPIO_TypeDef* GPIO_TX = NULL;
    GPIO_TypeDef* GPIO_RX = NULL;
    uint16_t GPIO_PIN_TX = 0;
    uint16_t GPIO_PIN_RX = 0;
    uint8_t GPIO_PIN_SOURCE_TX = 0;
    uint8_t GPIO_PIN_SOURCE_RX = 0;
    uint8_t GPIO_AF = 0;
    uint32_t GPIO_RCC_TX = 0;
    uint32_t GPIO_RCC_RX = 0;
    uint32_t USART_RCC = 0;
    uint8_t USART_IRQn = 0;
    
    if(config->USARTx == USART1)
    {
        GPIO_TX = UART1_TX_GPIO;
        GPIO_RX = UART1_RX_GPIO;
        GPIO_PIN_TX = UART1_TX_PIN;
        GPIO_PIN_RX = UART1_RX_PIN;
        GPIO_PIN_SOURCE_TX = UART1_TX_GPIO_SOURCE;
        GPIO_PIN_SOURCE_RX = UART1_RX_GPIO_SOURCE;
        GPIO_AF = UART1_TX_AF;
        GPIO_RCC_TX = UART1_TX_GPIO_RCC;
        GPIO_RCC_RX = UART1_RX_GPIO_RCC;
        USART_RCC = UART1_RCC;
        USART_IRQn = USART1_IRQn;
    }
    else if(config->USARTx == USART2)
    {
        GPIO_TX = UART2_TX_GPIO;
        GPIO_RX = UART2_RX_GPIO;
        GPIO_PIN_TX = UART2_TX_PIN;
        GPIO_PIN_RX = UART2_RX_PIN;
        GPIO_PIN_SOURCE_TX = UART2_TX_GPIO_SOURCE;
        GPIO_PIN_SOURCE_RX = UART2_RX_GPIO_SOURCE;
        GPIO_AF = UART2_TX_AF;
        GPIO_RCC_TX = UART2_TX_GPIO_RCC;
        GPIO_RCC_RX = UART2_RX_GPIO_RCC;
        USART_RCC = UART2_RCC;
        USART_IRQn = USART2_IRQn;
    }
    else if(config->USARTx == USART3)
    {
        GPIO_TX = UART3_TX_GPIO;
        GPIO_RX = UART3_RX_GPIO;
        GPIO_PIN_TX = UART3_TX_PIN;
        GPIO_PIN_RX = UART3_RX_PIN;
        GPIO_PIN_SOURCE_TX = UART3_TX_GPIO_SOURCE;
        GPIO_PIN_SOURCE_RX = UART3_RX_GPIO_SOURCE;
        GPIO_AF = UART3_TX_AF;
        GPIO_RCC_TX = UART3_TX_GPIO_RCC;
        GPIO_RCC_RX = UART3_RX_GPIO_RCC;
        USART_RCC = UART3_RCC;
        USART_IRQn = USART3_IRQn;
    }
    else
    {
        /* 不支持的UART */
        return;
    }
    
    /* 使能GPIO时钟 */
    RCC_AHB1PeriphClockCmd(GPIO_RCC_TX, ENABLE);
    RCC_AHB1PeriphClockCmd(GPIO_RCC_RX, ENABLE);
    
    /* 使能UART时钟 */
    if(config->USARTx == USART1)
    {
        RCC_APB2PeriphClockCmd(USART_RCC, ENABLE);
    }
    else /* USART2 或 USART3 */
    {
        RCC_APB1PeriphClockCmd(USART_RCC, ENABLE);
    }
    
    /* 配置GPIO复用功能 */
    GPIO_PinAFConfig(GPIO_TX, GPIO_PIN_SOURCE_TX, GPIO_AF);
    GPIO_PinAFConfig(GPIO_RX, GPIO_PIN_SOURCE_RX, GPIO_AF);
    
    /* 配置TX引脚 */
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_TX;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIO_TX, &GPIO_InitStructure);
    
    /* 配置RX引脚 */
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_RX;
    GPIO_Init(GPIO_RX, &GPIO_InitStructure);
    
    /* 配置USART参数 */
    USART_InitStructure.USART_BaudRate = config->BaudRate;
    
    /* 根据枚举配置数据位 */
    if(config->DataBits == UART_DATABIT_9)
    {
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
    }
    else /* UART_DATABIT_8 */
    {
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    }
    
    /* 根据枚举配置停止位 */
    if(config->StopBits == UART_STOPBIT_2)
    {
        USART_InitStructure.USART_StopBits = USART_StopBits_2;
    }
    else /* UART_STOPBIT_1 */
    {
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
    }
    
    /* 根据枚举配置校验位 */
    if(config->Parity == UART_PARITY_ODD)
    {
        USART_InitStructure.USART_Parity = USART_Parity_Odd;
    }
    else if(config->Parity == UART_PARITY_EVEN)
    {
        USART_InitStructure.USART_Parity = USART_Parity_Even;
    }
    else /* UART_PARITY_NONE */
    {
        USART_InitStructure.USART_Parity = USART_Parity_No;
    }
    
    /* 根据枚举配置流控制 */
    if(config->FlowControl == UART_FLOWCTRL_RTS)
    {
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS;
    }
    else if(config->FlowControl == UART_FLOWCTRL_CTS)
    {
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_CTS;
    }
    else if(config->FlowControl == UART_FLOWCTRL_RTS_CTS)
    {
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_RTS_CTS;
    }
    else /* UART_FLOWCTRL_NONE */
    {
        USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    }
    
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(config->USARTx, &USART_InitStructure);
    
    /* 配置中断 */
    if(config->RxIntEnable == ENABLE)
    {
        USART_ITConfig(config->USARTx, USART_IT_RXNE, ENABLE);
        
        NVIC_InitStructure.NVIC_IRQChannel = USART_IRQn;
        NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = config->PreemptionPriority;
        NVIC_InitStructure.NVIC_IRQChannelSubPriority = config->SubPriority;
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
        NVIC_Init(&NVIC_InitStructure);
    }
    
    /* 使能USART */
    USART_Cmd(config->USARTx, ENABLE);
}

/**
 * @brief  USART1中断服务函数
 * @param  None
 * @retval None
 * @note   直接在中断函数中处理LED和蜂鸣器控制
 */
void USART1_IRQHandler(void)
{
    uint8_t ch;
    
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        /* 读取接收到的数据 */
        ch = USART_ReceiveData(USART1);
        
        /* 回显接收到的字符 */
        UART_SendChar(USART1, ch);
        
        /* 直接在中断中处理命令 */
        switch(ch)
        {
            case '1':
                /* 点亮LED0 */
                Led_On(0);
                UART_SendString(USART1, "LED0 ON\r\n");
                break;
                
            case '2':
                /* 熄灭LED0 */
                Led_Off(0);
                UART_SendString(USART1, "LED0 OFF\r\n");
                break;
                
            case '3':
                /* 点亮LED1 */
                Led_On(1);
                UART_SendString(USART1, "LED1 ON\r\n");
                break;
                
            case '4':
                /* 熄灭LED1 */
                Led_Off(1);
                UART_SendString(USART1, "LED1 OFF\r\n");
                break;
                
            case '5':
                /* 打开蜂鸣器 */
                Beep_On();
                UART_SendString(USART1, "BEEP ON\r\n");
                break;
                
            case '6':
                /* 关闭蜂鸣器 */
                Beep_Off();
                UART_SendString(USART1, "BEEP OFF\r\n");
                break;
                
            case '0':
                /* 关闭所有LED和蜂鸣器 */
                Led_Off(0);
                Led_Off(1);
                Led_Off(2);
                Led_Off(3);
                Beep_Off();
                UART_SendString(USART1, "ALL OFF\r\n");
                break; 
            default:
                /* 对于其他字符，不做特殊处理 */
                break;
        }

        /* 清除中断标志 */
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}


/**
 * @brief  串口发送单个字符
 * @param  USARTx: UART外设
 * @param  ch: 要发送的字符
 * @retval None
 */
void UART_SendChar(USART_TypeDef* USARTx, uint8_t ch)
{
    /* 等待上一个字节发送完成 */
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    
    /* 发送数据 */
    USART_SendData(USARTx, ch);
}

/**
 * @brief  串口发送字符串
 * @param  USARTx: UART外设
 * @param  str: 要发送的字符串
 * @retval None
 */
void UART_SendString(USART_TypeDef* USARTx, char *str)
{
    while(*str)
    {
        UART_SendChar(USARTx, *str++);
    }
}

/**
 * @brief  USART1发送指定长度的字符串
 * @param  USARTx: UART外设
 * @param  Sendbuf: 要发送的字符串
 * @param  n: 要发送的字节长度
 * @retval None
 */
void USART1_SendString(USART_TypeDef* USARTx, uint8_t* Sendbuf, uint8_t n)
{
    int i;
    for(i = 0; i < n; i++)
    {
        /* 等待TXE标志位被设置，即上一个数据已发送完成 */
        while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
        {
            /* 等待发送数据寄存器为空 */
        }
        /* 发送数据 */
        USART_SendData(USARTx, Sendbuf[i]);
    }
}

/**
 * @brief  串口发送格式化字符串（类似printf）
 * @param  USARTx: UART外设
 * @param  fmt: 格式化字符串
 * @param  ...: 可变参数
 * @retval None
 */
void UART_Printf(USART_TypeDef* USARTx, const char *fmt, ...)
{
    char buffer[256]; /* 格式化字符串缓冲区 */
    va_list args;
    
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    UART_SendString(USARTx, buffer);
}

/**
 * @brief  重定向printf到串口1
 * @note   在使用printf前需要调用此函数
 */
void UART_RedirectPrintf(void)
{
    /* 在使用printf前调用此函数，完成重定向 */
    /* 具体实现是通过fputc函数完成的，无需在此执行任何操作 */
}

/**
 * @brief  重定向fputc函数，使printf输出到串口
 * @param  ch: 要输出的字符
 * @param  f: 文件指针
 * @retval 输出的字符
 */
int fputc(int ch, FILE *f)
{
    /* 将Printf内容发往串口 */
    UART_SendChar(USART1, (uint8_t)ch);
    return ch;
}


/**
 * @brief  USART2中断服务函数
 * @param  None
 * @retval None
 */
void USART2_IRQHandler(void)
{
    uint8_t ch;
    
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        /* 读取接收到的数据 */
        ch = USART_ReceiveData(USART2);
        
        /* 调用回调函数处理接收到的数据 */
        UART_RxCallback(USART2, ch);
        
        /* 清除中断标志 */
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

/**
 * @brief  USART3中断服务函数
 * @param  None
 * @retval None
 */
void USART3_IRQHandler(void)
{
    uint8_t ch;
    
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
    {
        /* 读取接收到的数据 */
        ch = USART_ReceiveData(USART3);
        
        /* 调用回调函数处理接收到的数据 */
        UART_RxCallback(USART3, ch);
        
        /* 清除中断标志 */
        USART_ClearITPendingBit(USART3, USART_IT_RXNE);
    }
}

/**
 * @brief  串口接收中断回调函数
 * @param  USARTx: UART外设
 * @param  ch: 接收到的字符
 * @note   用户可以在其他文件中重新实现此函数以处理接收数据
 */
void UART_RxCallback(USART_TypeDef* USARTx, uint8_t ch)
{
    /* 处理USART3的MQ-2传感器数据 */
    if(USARTx == USART3)
    {
        /* 调用MQ-2数据处理函数 */
        extern void MQ2_ProcessData(uint8_t ch);
        MQ2_ProcessData(ch);
        return;  /* 直接返回，不进行echo */
    }
    
    /* Echo received data back for other USARTs */
    UART_SendChar(USARTx, ch);
    
    /* Process USART1 data and control peripherals based on received character */
    if(USARTx == USART1)
    {
        /* Control LEDs and beep based on received character */
        if(ch == '1')
        {
            /* Turn ON LED0 */
            extern void Led_On(uint16_t Led_Num);
            Led_On(0);
            
            /* Send confirmation */
            UART_SendString(USART1, "LED0 ON\r\n");
        }
        else if(ch == '2')
        {
            /* Turn OFF LED0 */
            extern void Led_Off(uint16_t Led_Num);
            Led_Off(0);
            
            /* Send confirmation */
            UART_SendString(USART1, "LED0 OFF\r\n");
        }
        else if(ch == '3')
        {
            /* Turn ON LED1 */
            extern void Led_On(uint16_t Led_Num);
            Led_On(1);
            
            /* Send confirmation */
            UART_SendString(USART1, "LED1 ON\r\n");
        }
        else if(ch == '4')
        {
            /* Turn OFF LED1 */
            extern void Led_Off(uint16_t Led_Num);
            Led_Off(1);
            
            /* Send confirmation */
            UART_SendString(USART1, "LED1 OFF\r\n");
        }
        else if(ch == '5')
        {
            /* Turn ON Beep */
            extern void Beep_On(void);
            Beep_On();
            
            /* Send confirmation */
            UART_SendString(USART1, "BEEP ON\r\n");
        }
        else if(ch == '6')
        {
            /* Turn OFF Beep */
            extern void Beep_Off(void);
            Beep_Off();
            
            /* Send confirmation */
            UART_SendString(USART1, "BEEP OFF\r\n");
        }
        else if(ch == '0')
        {
            /* Turn OFF all LEDs and Beep */
            extern void Led_Off(uint16_t Led_Num);
            extern void Beep_Off(void);
            
            Led_Off(0);
            Led_Off(1);
            Led_Off(2);
            Led_Off(3);
            Beep_Off();
            
            /* Send confirmation */
            UART_SendString(USART1, "ALL OFF\r\n");
        }
        else if(ch == 'h' || ch == 'H')
        {
            /* Send help information */
            UART_SendString(USART1, "\r\n===== UART Control Help =====\r\n");
            UART_SendString(USART1, "1: Turn ON LED0\r\n");
            UART_SendString(USART1, "2: Turn OFF LED0\r\n");
            UART_SendString(USART1, "3: Turn ON LED1\r\n");
            UART_SendString(USART1, "4: Turn OFF LED1\r\n");
            UART_SendString(USART1, "5: Turn ON Beep\r\n");
            UART_SendString(USART1, "6: Turn OFF Beep\r\n");
            UART_SendString(USART1, "0: Turn OFF All LEDs and Beep\r\n");
            UART_SendString(USART1, "h/H: Show this help\r\n");
            UART_SendString(USART1, "==========================\r\n");
        }
    }
}
