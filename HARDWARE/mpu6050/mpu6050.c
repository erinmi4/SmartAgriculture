#include "mpu6050.h"
#include "I2C.h"  // 使用大写的I2C.h（匹配你修改的文件名）

// 初始化MPU6050，返回0表示成功（兼容原有系统）
uint8_t MPU6050_Init(void)
{
    uint8_t temp;
    
    My_I2C_Init();  // 初始化I2C接口

    // 检测MPU6050是否存在
    temp = MPU6050_Read_Byte(0x75);  // WHO_AM_I寄存器
    if(temp != 0x68)  // MPU6050的WHO_AM_I应该返回0x68
    {
        return 1;  // 初始化失败
    }

    // 唤醒MPU6050
    I2C_Write_Byte(MPU6050_ADDR, PWR_MGMT_1, 0x00);

    // 设置采样率
    I2C_Write_Byte(MPU6050_ADDR, SMPLRT_DIV, 0x07);

    // 配置陀螺仪和加速度计的低通滤波器
    I2C_Write_Byte(MPU6050_ADDR, CONFIG, 0x00);

    // 配置陀螺仪量程（±250°/s）
    I2C_Write_Byte(MPU6050_ADDR, GYRO_CONFIG, 0x00);

    // 配置加速度计量程（±2g）
    I2C_Write_Byte(MPU6050_ADDR, ACCEL_CONFIG, 0x00);

    // 使能中断
    I2C_Write_Byte(MPU6050_ADDR, INT_ENABLE, 0x01);
    
    return 0;  // 初始化成功
}

// 读取MPU6050寄存器数据
uint8_t MPU6050_Read_Byte(uint8_t reg)
{
    return I2C_Read_Byte(MPU6050_ADDR, reg);
}

// 读取MPU6050多字节数据
void MPU6050_Read_Multiple(uint8_t reg, uint8_t *buf, uint8_t len)
{
    I2C_Read_Multiple(MPU6050_ADDR, reg, buf, len);
}

// 读取加速度计数据
void MPU6050_Read_Accel(int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t buf[6];
    MPU6050_Read_Multiple(ACCEL_XOUT_H, buf, 6);
    *ax = (buf[0] << 8) | buf[1];
    *ay = (buf[2] << 8) | buf[3];
    *az = (buf[4] << 8) | buf[5];
}

// 读取陀螺仪数据
void MPU6050_Read_Gyro(int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t buf[6];
    MPU6050_Read_Multiple(GYRO_XOUT_H, buf, 6);
    *gx = (buf[0] << 8) | buf[1];
    *gy = (buf[2] << 8) | buf[3];
    *gz = (buf[4] << 8) | buf[5];
}

// 读取温度数据
int16_t MPU6050_Read_Temp(void)
{
    uint8_t buf[2];
    MPU6050_Read_Multiple(TEMP_OUT_H, buf, 2);
    return (buf[0] << 8) | buf[1];
}

// 兼容原有系统的数据获取函数
void MPU6050_GetData(MPU6050_Data_t *data)
{
    int16_t ax, ay, az, gx, gy, gz, temp;
    
    // 读取原始数据
    MPU6050_Read_Accel(&ax, &ay, &az);
    MPU6050_Read_Gyro(&gx, &gy, &gz);
    temp = MPU6050_Read_Temp();
    
    // 转换为浮点数（根据量程配置进行缩放）
    // 加速度计：±2g，LSB Sensitivity = 16384 LSB/g
    data->accel_x = (float)ax / 16384.0f;
    data->accel_y = (float)ay / 16384.0f;
    data->accel_z = (float)az / 16384.0f;
    
    // 陀螺仪：±250°/s，LSB Sensitivity = 131 LSB/(°/s)
    data->gyro_x = (float)gx / 131.0f;
    data->gyro_y = (float)gy / 131.0f;
    data->gyro_z = (float)gz / 131.0f;
    
    // 温度：Temperature in degrees C = (TEMP_OUT Register Value as a signed quantity)/340 + 36.53
    data->temp = (float)temp / 340.0f + 36.53f;
}

