#include "mpu6050.h"
#include "i2c.h"  // �������i2c.h��i2c.c�ļ�����I2Cͨ��

// ��ʼ��MPU6050
void MPU6050_Init(void)
{
    My_I2C_Init();  // ��ʼ��I2C�ӿ�

    // ����MPU6050
    I2C_Write_Byte(MPU6050_ADDR, PWR_MGMT_1, 0x00);

    // ���ò�����
    I2C_Write_Byte(MPU6050_ADDR, SMPLRT_DIV, 0x07);

    // ���������Ǻͼ��ٶȼƵĵ�ͨ�˲���
    I2C_Write_Byte(MPU6050_ADDR, CONFIG, 0x00);

    // ��������������
    I2C_Write_Byte(MPU6050_ADDR, GYRO_CONFIG, 0x00);

    // ���ü��ٶȼ�����
    I2C_Write_Byte(MPU6050_ADDR, ACCEL_CONFIG, 0x00);

    // ʹ���ж�
    I2C_Write_Byte(MPU6050_ADDR, INT_ENABLE, 0x01);
}

// ��ȡMPU6050�Ĵ�������
uint8_t MPU6050_Read_Byte(uint8_t reg)
{
    return I2C_Read_Byte(MPU6050_ADDR, reg);
}

// ��ȡMPU6050���ֽ�����
void MPU6050_Read_Multiple(uint8_t reg, uint8_t *buf, uint8_t len)
{
    I2C_Read_Multiple(MPU6050_ADDR, reg, buf, len);
}

// ��ȡ���ٶȼ�����
void MPU6050_Read_Accel(int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t buf[6];
    MPU6050_Read_Multiple(ACCEL_XOUT_H, buf, 6);
    *ax = (buf[0] << 8) | buf[1];
    *ay = (buf[2] << 8) | buf[3];
    *az = (buf[4] << 8) | buf[5];
}

// ��ȡ����������
void MPU6050_Read_Gyro(int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t buf[6];
    MPU6050_Read_Multiple(GYRO_XOUT_H, buf, 6);
    *gx = (buf[0] << 8) | buf[1];
    *gy = (buf[2] << 8) | buf[3];
    *gz = (buf[4] << 8) | buf[5];
}

