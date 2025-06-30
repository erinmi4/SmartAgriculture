#ifndef __UART_H
#define __UART_H

/**
 * @file    uart.h
 * @brief   UART通信模块头文件
 * @details 提供STM32F4xx系列MCU串口通信相关的函数声明和宏定义
 * @author  龚维学
 * @date    2025-06-28
 * @version 1.0
 */

#include "stm32f4xx.h"
#include <stdio.h>
#include "led.h"
#include "beep.h"

/* USART1 引脚定义  tx pa9 , rx pa10*/
#define UART1_TX_PIN           GPIO_Pin_9
#define UART1_TX_GPIO          GPIOA
#define UART1_TX_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define UART1_TX_GPIO_SOURCE   GPIO_PinSource9
#define UART1_TX_AF            GPIO_AF_USART1

#define UART1_RX_PIN           GPIO_Pin_10
#define UART1_RX_GPIO          GPIOA
#define UART1_RX_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define UART1_RX_GPIO_SOURCE   GPIO_PinSource10
#define UART1_RX_AF            GPIO_AF_USART1

#define UART1_RCC              RCC_APB2Periph_USART1

/* USART2 引脚定义 rx pa2 , txt pa3 */
#define UART2_TX_PIN           GPIO_Pin_2
#define UART2_TX_GPIO          GPIOA
#define UART2_TX_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define UART2_TX_GPIO_SOURCE   GPIO_PinSource2
#define UART2_TX_AF            GPIO_AF_USART2

#define UART2_RX_PIN           GPIO_Pin_3
#define UART2_RX_GPIO          GPIOA
#define UART2_RX_GPIO_RCC      RCC_AHB1Periph_GPIOA
#define UART2_RX_GPIO_SOURCE   GPIO_PinSource3
#define UART2_RX_AF            GPIO_AF_USART2

#define UART2_RCC              RCC_APB1Periph_USART2

/* USART3 引脚定义 tx pb10,rx pb11 */
#define UART3_TX_PIN           GPIO_Pin_10
#define UART3_TX_GPIO          GPIOB
#define UART3_TX_GPIO_RCC      RCC_AHB1Periph_GPIOB
#define UART3_TX_GPIO_SOURCE   GPIO_PinSource10
#define UART3_TX_AF            GPIO_AF_USART3

#define UART3_RX_PIN           GPIO_Pin_11
#define UART3_RX_GPIO          GPIOB
#define UART3_RX_GPIO_RCC      RCC_AHB1Periph_GPIOB
#define UART3_RX_GPIO_SOURCE   GPIO_PinSource11
#define UART3_RX_AF            GPIO_AF_USART3

#define UART3_RCC              RCC_APB1Periph_USART3



/* 串口波特率定义 */
typedef enum {
    UART_BAUD_9600 = 9600,
    UART_BAUD_19200 = 19200,
    UART_BAUD_38400 = 38400,
    UART_BAUD_57600 = 57600,
    UART_BAUD_115200 = 115200,
    UART_BAUD_230400 = 230400,
    UART_BAUD_460800 = 460800,
    UART_BAUD_921600 = 921600
} UART_BaudRateTypeDef;

/* 串口校验位定义 */
typedef enum {
    UART_PARITY_NONE = 0x00,  /* 无校验 */
    UART_PARITY_EVEN = 0x01,  /* 偶校验 */
    UART_PARITY_ODD = 0x02    /* 奇校验 */
} UART_ParityTypeDef;

/* 串口停止位定义 */
typedef enum {
    UART_STOPBIT_1 = 0x00,    /* 1位停止位 */
    UART_STOPBIT_2 = 0x01     /* 2位停止位 */
} UART_StopBitTypeDef;

/* 串口数据位定义 */
typedef enum {
    UART_DATABIT_8 = 0x00,    /* 8位数据位 */
    UART_DATABIT_9 = 0x01     /* 9位数据位 */
} UART_DataBitTypeDef;

/* 串口流控制定义 */
typedef enum {
    UART_FLOWCTRL_NONE = 0x00,    /* 无流控 */
    UART_FLOWCTRL_RTS = 0x01,     /* RTS流控 */
    UART_FLOWCTRL_CTS = 0x02,     /* CTS流控 */
    UART_FLOWCTRL_RTS_CTS = 0x03  /* RTS和CTS流控 */
} UART_FlowControlTypeDef;

/**
 * @brief  UART配置结构体
 */
typedef struct {
    USART_TypeDef* USARTx;               /* UART外设 */
    UART_BaudRateTypeDef BaudRate;       /* 波特率 */
    UART_ParityTypeDef Parity;           /* 校验位 */
    UART_StopBitTypeDef StopBits;        /* 停止位 */
    UART_DataBitTypeDef DataBits;        /* 数据位 */
    UART_FlowControlTypeDef FlowControl; /* 流控 */
    FunctionalState RxIntEnable;         /* 接收中断使能 */
    uint8_t PreemptionPriority;          /* 抢占优先级 */
    uint8_t SubPriority;                 /* 子优先级 */
} UART_ConfigTypeDef;

/* 函数声明 */
/**
 * @brief  串口1初始化函数
 * @param  baudrate: 波特率，如UART_BAUD_115200
 * @retval None
 * @note   此函数配置串口1为8位数据，无校验，1位停止位
 */
void UART1_Init(UART_BaudRateTypeDef baudrate);

/**
 * @brief  串口2初始化函数
 * @param  baudrate: 波特率，如UART_BAUD_115200
 * @retval None
 * @note   此函数配置串口2为8位数据，无校验，1位停止位
 */
void UART2_Init(UART_BaudRateTypeDef baudrate);

/**
 * @brief  串口3初始化函数
 * @param  baudrate: 波特率，如UART_BAUD_115200
 * @retval None
 * @note   此函数配置串口3为8位数据，无校验，1位停止位
 */
void UART3_Init(UART_BaudRateTypeDef baudrate);

/**
 * @brief  通用串口初始化函数
 * @param  config: 串口配置结构体指针
 * @retval None
 * @note   此函数可配置任意串口的详细参数
 */
void UART_Init(UART_ConfigTypeDef *config);

/**
 * @brief  串口发送单个字符
 * @param  USARTx: UART外设
 * @param  ch: 要发送的字符
 * @retval None
 */
void UART_SendChar(USART_TypeDef* USARTx, uint8_t ch);

/**
 * @brief  串口发送字符串
 * @param  USARTx: UART外设
 * @param  str: 要发送的字符串
 * @retval None
 */
void UART_SendString(USART_TypeDef* USARTx, char *str);

/**
 * @brief  串口发送格式化字符串（类似printf）
 * @param  USARTx: UART外设
 * @param  fmt: 格式化字符串
 * @param  ...: 可变参数
 * @retval None
 */
void UART_Printf(USART_TypeDef* USARTx, const char *fmt, ...);

/**
 * @brief  重定向printf到串口1
 * @note   在使用printf前需要调用此函数
 */
void UART_RedirectPrintf(void);

/**
 * @brief  串口接收中断回调函数
 * @param  USARTx: UART外设
 * @param  ch: 接收到的字符
 * @note   用户需要自己实现此函数
 */
void UART_RxCallback(USART_TypeDef* USARTx, uint8_t ch);

/**
 * @brief  USART1发送指定长度的字符串
 * @param  USARTx: UART外设
 * @param  Sendbuf: 要发送的字符串
 * @param  n: 要发送的字节长度
 * @retval None
 */
void USART1_SendString(USART_TypeDef* USARTx, uint8_t* Sendbuf, uint8_t n);

#endif /* __UART_H */
