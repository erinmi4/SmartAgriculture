#ifndef __MPU6050_H
#define __MPU6050_H

#include "stm32f4xx.h"

// MPU6050���ݽṹ������ԭ��ϵͳ��
typedef struct {
    float accel_x;     // ���ٶ�X��
    float accel_y;     // ���ٶ�Y��  
    float accel_z;     // ���ٶ�Z��
    float gyro_x;      // ������X��
    float gyro_y;      // ������Y��
    float gyro_z;      // ������Z��
    float temp;        // �¶�
} MPU6050_Data_t;

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
#define TEMP_OUT_H 0x41

// ��������
uint8_t MPU6050_Init(void);                    // ����0��ʾ�ɹ�������ԭ��ϵͳ
uint8_t MPU6050_Read_Byte(uint8_t reg);
void MPU6050_Read_Multiple(uint8_t reg, uint8_t *buf, uint8_t len);
void MPU6050_Read_Accel(int16_t *ax, int16_t *ay, int16_t *az);
void MPU6050_Read_Gyro(int16_t *gx, int16_t *gy, int16_t *gz);
void MPU6050_GetData(MPU6050_Data_t *data);    // ����ԭ��ϵͳ�����ݻ�ȡ����

#endif

