#include "mpu6050.h"
#include "I2C.h"  // ʹ�ô�д��I2C.h��ƥ�����޸ĵ��ļ�����

// ��ʼ��MPU6050������0��ʾ�ɹ�������ԭ��ϵͳ��
uint8_t MPU6050_Init(void)
{
    uint8_t temp;
    
    My_I2C_Init();  // ��ʼ��I2C�ӿ�

    // ���MPU6050�Ƿ����
    temp = MPU6050_Read_Byte(0x75);  // WHO_AM_I�Ĵ���
    if(temp != 0x68)  // MPU6050��WHO_AM_IӦ�÷���0x68
    {
        return 1;  // ��ʼ��ʧ��
    }

    // ����MPU6050
    I2C_Write_Byte(MPU6050_ADDR, PWR_MGMT_1, 0x00);

    // ���ò�����
    I2C_Write_Byte(MPU6050_ADDR, SMPLRT_DIV, 0x07);

    // ���������Ǻͼ��ٶȼƵĵ�ͨ�˲���
    I2C_Write_Byte(MPU6050_ADDR, CONFIG, 0x00);

    // �������������̣���250��/s��
    I2C_Write_Byte(MPU6050_ADDR, GYRO_CONFIG, 0x00);

    // ���ü��ٶȼ����̣���2g��
    I2C_Write_Byte(MPU6050_ADDR, ACCEL_CONFIG, 0x00);

    // ʹ���ж�
    I2C_Write_Byte(MPU6050_ADDR, INT_ENABLE, 0x01);
    
    return 0;  // ��ʼ���ɹ�
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

// ��ȡ�¶�����
int16_t MPU6050_Read_Temp(void)
{
    uint8_t buf[2];
    MPU6050_Read_Multiple(TEMP_OUT_H, buf, 2);
    return (buf[0] << 8) | buf[1];
}

// ����ԭ��ϵͳ�����ݻ�ȡ����
void MPU6050_GetData(MPU6050_Data_t *data)
{
    int16_t ax, ay, az, gx, gy, gz, temp;
    
    // ��ȡԭʼ����
    MPU6050_Read_Accel(&ax, &ay, &az);
    MPU6050_Read_Gyro(&gx, &gy, &gz);
    temp = MPU6050_Read_Temp();
    
    // ת��Ϊ�������������������ý������ţ�
    // ���ٶȼƣ���2g��LSB Sensitivity = 16384 LSB/g
    data->accel_x = (float)ax / 16384.0f;
    data->accel_y = (float)ay / 16384.0f;
    data->accel_z = (float)az / 16384.0f;
    
    // �����ǣ���250��/s��LSB Sensitivity = 131 LSB/(��/s)
    data->gyro_x = (float)gx / 131.0f;
    data->gyro_y = (float)gy / 131.0f;
    data->gyro_z = (float)gz / 131.0f;
    
    // �¶ȣ�Temperature in degrees C = (TEMP_OUT Register Value as a signed quantity)/340 + 36.53
    data->temp = (float)temp / 340.0f + 36.53f;
}

