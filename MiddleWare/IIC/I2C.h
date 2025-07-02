#ifndef __I2C_H
#define __I2C_H

#include "stm32f4xx.h"

// 初始化I2C接口
void My_I2C_Init(void);
// 向I2C设备写入一个字节
void I2C_Write_Byte(uint8_t addr, uint8_t reg, uint8_t data);
// 从I2C设备读取一个字节
uint8_t I2C_Read_Byte(uint8_t addr, uint8_t reg);
// 从I2C设备读取多个字节
void I2C_Read_Multiple(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);

#endif

