#ifndef __I2C_H
#define __I2C_H

#include "stm32f4xx.h"

/* 函数声明 */
void MyI2C_Init(void);
uint8_t MyI2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data);
uint8_t MyI2C_ReadReg(uint8_t addr, uint8_t reg);
void MyI2C_ReadMultiReg(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);

#endif /* __I2C_H */
