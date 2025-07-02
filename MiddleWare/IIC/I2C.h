#ifndef __I2C_H
#define __I2C_H

#include "stm32f4xx.h"

// ��ʼ��I2C�ӿ�
void My_I2C_Init(void);
// ��I2C�豸д��һ���ֽ�
void I2C_Write_Byte(uint8_t addr, uint8_t reg, uint8_t data);
// ��I2C�豸��ȡһ���ֽ�
uint8_t I2C_Read_Byte(uint8_t addr, uint8_t reg);
// ��I2C�豸��ȡ����ֽ�
void I2C_Read_Multiple(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len);

#endif

