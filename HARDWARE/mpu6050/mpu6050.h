#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f4xx.h"

// MPU6050 I2C地址
#define MPU6050_ADDR 0xD0

// MPU6050寄存器地址
#define PWR_MGMT_1 0x6B
#define SMPLRT_DIV 0x19
#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C
#define INT_ENABLE 0x38
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43

// 初始化MPU6050
void MPU6050_Init(void);
// 读取MPU6050寄存器数据
uint8_t MPU6050_Read_Byte(uint8_t reg);
// 读取MPU6050多字节数据
void MPU6050_Read_Multiple(uint8_t reg, uint8_t *buf, uint8_t len);
// 读取加速度计数据
void MPU6050_Read_Accel(int16_t *ax, int16_t *ay, int16_t *az);
// 读取陀螺仪数据
void MPU6050_Read_Gyro(int16_t *gx, int16_t *gy, int16_t *gz);

#endif

