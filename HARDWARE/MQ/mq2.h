#ifndef __MQ2_H
#define __MQ2_H

#include "stm32f4xx.h"

/* MQ-2传感器命令帧格式 */
#define MQ2_CMD_FRAME_SIZE  9
#define MQ2_RESP_FRAME_SIZE 9

/* 函数声明 */
void MQ2_Init(void);                // MQ-2初始化
void MQ2_SendCommand(void);         // 发送命令到MQ-2传感器
unsigned int MQ2_GetValue(void);    // 获取烟雾浓度值
unsigned char MQ2_IsDataReady(void);// 检查数据是否就绪
void MQ2_ClearFlag(void);           // 清除数据就绪标志
void MQ2_ProcessData(uint8_t ch);   // MQ-2数据处理函数

#endif /* __MQ2_H */
