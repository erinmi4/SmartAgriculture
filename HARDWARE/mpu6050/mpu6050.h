#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f4xx.h"

// MPU6050数据结构（兼容原有系统）
typedef struct {
    float accel_x;     // 加速度X轴
    float accel_y;     // 加速度Y轴  
    float accel_z;     // 加速度Z轴
    float gyro_x;      // 陀螺仪X轴
    float gyro_y;      // 陀螺仪Y轴
    float gyro_z;      // 陀螺仪Z轴
    float temp;        // 温度
} MPU6050_Data_t;

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
#define TEMP_OUT_H 0x41

// 函数声明
uint8_t MPU6050_Init(void);                    // 返回0表示成功，兼容原有系统
uint8_t MPU6050_Read_Byte(uint8_t reg);
void MPU6050_Read_Multiple(uint8_t reg, uint8_t *buf, uint8_t len);
void MPU6050_Read_Accel(int16_t *ax, int16_t *ay, int16_t *az);
void MPU6050_Read_Gyro(int16_t *gx, int16_t *gy, int16_t *gz);
void MPU6050_GetData(MPU6050_Data_t *data);    // 兼容原有系统的数据获取函数

#endif

