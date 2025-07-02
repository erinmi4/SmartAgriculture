#include "mpu6050.h"
#include "i2c.h"
#include "delay.h"
#include <math.h>

/**
 * @brief MPU6050写寄存器
 * @param reg  寄存器地址
 * @param data 要写入的数据
 * @retval 0-成功，其他-失败
 */
uint8_t MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    return MyI2C_WriteReg(MPU6050_ADDR, reg, data);
}

/**
 * @brief MPU6050读寄存器
 * @param reg 寄存器地址
 * @retval 读取到的数据
 */
uint8_t MPU6050_ReadReg(uint8_t reg)
{
    return MyI2C_ReadReg(MPU6050_ADDR, reg);
}

/**
 * @brief MPU6050连续读取多个寄存器
 * @param reg 起始寄存器地址
 * @param buf 数据缓冲区
 * @param len 读取长度
 * @retval None
 */
void MPU6050_ReadMultiReg(uint8_t reg, uint8_t *buf, uint8_t len)
{
    MyI2C_ReadMultiReg(MPU6050_ADDR, reg, buf, len);
}

/**
 * @brief  MPU6050初始化
 * @param  None
 * @retval 0-成功, 1-失败
 */
uint8_t MPU6050_Init(void)
{
    uint8_t res;
    uint8_t retry_count = 0;
    
    /* 初始化硬件I2C */
    MyI2C_Init();
    
    /* 1. 在所有操作前，先延时一小段时间让芯片稳定 */
    Mdelay_Lib(100);
    
    /* 初始化中断引脚(可选) */
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(MPU6050_INT_CLK, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = MPU6050_INT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(MPU6050_INT_PORT, &GPIO_InitStructure);
    
    /* 检测MPU6050是否存在 */
    for(retry_count = 0; retry_count < 5; retry_count++)
    {
        res = MPU6050_ReadReg(MPU6050_RA_WHO_AM_I);
        if(res == 0x68)
        {
            break; // 找到设备
        }
        Mdelay_Lib(10);
    }
    
    if(res != 0x68)
    {
        return 1; // 设备不存在
    }
    
    /* 2. (可选但推荐) 发送复位命令 */
    MPU6050_WriteReg(MPU6050_RA_PWR_MGMT_1, 0x80);
    
    /* 3. 延时，等待复位完成 */
    Mdelay_Lib(100);
    
    /* 4. (必须) 唤醒MPU6050，并选择PLL X Gyro作为时钟源 */
    MPU6050_WriteReg(MPU6050_RA_PWR_MGMT_1, 0x01);
    
    /* 5. (必须) 延时，等待时钟稳定 */
    Mdelay_Lib(10);
    
    /* 6. 配置其他寄存器 */
    /* 设置陀螺仪采样率: 1KHz */
    MPU6050_WriteReg(MPU6050_RA_SMPLRT_DIV, 0x07);
    Mdelay_Lib(5);
    
    /* 配置数字低通滤波器 */
    MPU6050_WriteReg(MPU6050_RA_CONFIG, 0x06);
    Mdelay_Lib(5);
    
    /* 配置加速度传感器工作在±2g模式 */
    MPU6050_WriteReg(MPU6050_RA_ACCEL_CONFIG, 0x00);
    Mdelay_Lib(5);
    
    /* 陀螺仪自检及测量范围，典型值：0x18(不自检，±2000dps) */
    MPU6050_WriteReg(MPU6050_RA_GYRO_CONFIG, 0x18);
    Mdelay_Lib(5);
    
    /* 再次验证设备ID */
    res = MPU6050_ReadReg(MPU6050_RA_WHO_AM_I);
    if(res != 0x68)
    {
        return 1; // 验证失败
    }
    
    return 0;
}

/**
 * @brief  获取MPU6050设备ID
 * @param  None
 * @retval 设备ID
 */
uint8_t MPU6050_GetDeviceID(void)
{
    return MPU6050_ReadReg(MPU6050_RA_WHO_AM_I);
}

/**
 * @brief  获取MPU6050原始数据
 * @param  raw_data: 原始数据结构体指针
 * @retval None
 */
void MPU6050_GetRawData(MPU6050_RawData_t *raw_data)
{
    uint8_t buf[14];
    
    /* 连续读取14个字节 */
    MPU6050_ReadMultiReg(MPU6050_RA_ACCEL_XOUT_H, buf, 14);
    
    /* 组合数据 */
    raw_data->accel_x = (buf[0] << 8) | buf[1];
    raw_data->accel_y = (buf[2] << 8) | buf[3];
    raw_data->accel_z = (buf[4] << 8) | buf[5];
    raw_data->temp = (buf[6] << 8) | buf[7];
    raw_data->gyro_x = (buf[8] << 8) | buf[9];
    raw_data->gyro_y = (buf[10] << 8) | buf[11];
    raw_data->gyro_z = (buf[12] << 8) | buf[13];
}

/**
 * @brief  获取MPU6050转换后的数据
 * @param  data: 转换后数据结构体指针
 * @retval None
 */
void MPU6050_GetData(MPU6050_Data_t *data)
{
    MPU6050_RawData_t raw_data;
    
    /* 获取原始数据 */
    MPU6050_GetRawData(&raw_data);
    
    /* 转换加速度数据 (量程±2g，灵敏度16384 LSB/g) */
    data->accel_x = (float)raw_data.accel_x / 16384.0f;
    data->accel_y = (float)raw_data.accel_y / 16384.0f;
    data->accel_z = (float)raw_data.accel_z / 16384.0f;
    
    /* 转换温度数据 */
    data->temp = (float)raw_data.temp / 340.0f + 36.53f;
    
    /* 转换陀螺仪数据 (量程±2000°/s，灵敏度16.4 LSB/(°/s)) */
    data->gyro_x = (float)raw_data.gyro_x / 16.4f;
    data->gyro_y = (float)raw_data.gyro_y / 16.4f;
    data->gyro_z = (float)raw_data.gyro_z / 16.4f;
}

/**
 * @brief  获取加速度数据
 * @param  ax: X轴加速度指针
 * @param  ay: Y轴加速度指针
 * @param  az: Z轴加速度指针
 * @retval None
 */
void MPU6050_GetAccel(int16_t *ax, int16_t *ay, int16_t *az)
{
    uint8_t buf[6];
    
    // 逐个读取寄存器，更可靠
    buf[0] = MPU6050_ReadReg(MPU6050_RA_ACCEL_XOUT_H);
    buf[1] = MPU6050_ReadReg(MPU6050_RA_ACCEL_XOUT_L);
    buf[2] = MPU6050_ReadReg(MPU6050_RA_ACCEL_YOUT_H);
    buf[3] = MPU6050_ReadReg(MPU6050_RA_ACCEL_YOUT_L);
    buf[4] = MPU6050_ReadReg(MPU6050_RA_ACCEL_ZOUT_H);
    buf[5] = MPU6050_ReadReg(MPU6050_RA_ACCEL_ZOUT_L);
    
    *ax = (buf[0] << 8) | buf[1];
    *ay = (buf[2] << 8) | buf[3];
    *az = (buf[4] << 8) | buf[5];
}

/**
 * @brief  获取陀螺仪数据
 * @param  gx: X轴角速度指针
 * @param  gy: Y轴角速度指针
 * @param  gz: Z轴角速度指针
 * @retval None
 */
void MPU6050_GetGyro(int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t buf[6];
    
    // 逐个读取寄存器，更可靠
    buf[0] = MPU6050_ReadReg(MPU6050_RA_GYRO_XOUT_H);
    buf[1] = MPU6050_ReadReg(MPU6050_RA_GYRO_XOUT_L);
    buf[2] = MPU6050_ReadReg(MPU6050_RA_GYRO_YOUT_H);
    buf[3] = MPU6050_ReadReg(MPU6050_RA_GYRO_YOUT_L);
    buf[4] = MPU6050_ReadReg(MPU6050_RA_GYRO_ZOUT_H);
    buf[5] = MPU6050_ReadReg(MPU6050_RA_GYRO_ZOUT_L);
    
    *gx = (buf[0] << 8) | buf[1];
    *gy = (buf[2] << 8) | buf[3];
    *gz = (buf[4] << 8) | buf[5];
}

/**
 * @brief  获取温度数据
 * @param  None
 * @retval 温度原始数据
 */
int16_t MPU6050_GetTemp(void)
{
    uint8_t buf[2];
    
    MPU6050_ReadMultiReg(MPU6050_RA_TEMP_OUT_H, buf, 2);
    
    return (buf[0] << 8) | buf[1];
}
