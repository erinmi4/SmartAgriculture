#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f4xx.h"

// MPU6050 I2C��ַ
#define MPU6050_ADDR 0xD0

// MPU6050�Ĵ�����ַ
#define PWR_MGMT_1 0x6B
#define SMPLRT_DIV 0x19
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define INT_ENABLE 0x38
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43

// ��ʼ��MPU6050
void MPU6050_Init(void);
// ��ȡMPU6050�Ĵ�������
uint8_t MPU6050_Read_Byte(uint8_t reg);
// ��ȡMPU6050���ֽ�����
void MPU6050_Read_Multiple(uint8_t reg, uint8_t *buf, uint8_t len);
// ��ȡ���ٶȼ�����
void MPU6050_Read_Accel(int16_t *ax, int16_t *ay, int16_t *az);
// ��ȡ����������
void MPU6050_Read_Gyro(int16_t *gx, int16_t *gy, int16_t *gz);

#endif

