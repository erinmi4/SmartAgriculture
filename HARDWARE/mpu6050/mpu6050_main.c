/**
 * @file    main_mpu6050_test.c
 * @brief   MPU6050三轴陀螺仪和加速度计测试程序
 * @details 通过I2C接口读取MPU6050数据并在LCD上显示
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "mpu6050.h"
#include "i2c.h"
#include <stdio.h>

int main(void)
{
    MPU6050_Data_t mpu_data;
    uint8_t device_id;
    char str[32];
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    lcd_clear();
    
    // 显示初始化信息
    lcd_print_str(0, 0, "MPU6050 Init...");
    
    // 初始化MPU6050
    if(MPU6050_Init() != 0)
    {
        lcd_clear();
        lcd_print_str(0, 0, "MPU6050 Init");
        lcd_print_str(1, 0, "Failed!");
        while(1);  // 初始化失败，停止程序
    }
    
    // 读取设备ID
    device_id = MPU6050_GetDeviceID();
    sprintf(str, "ID: 0x%02X", device_id);
    lcd_print_str(1, 0, str);
    
    Mdelay_Lib(2000);  // 显示2秒
    
    // 测试读取电源管理寄存器
    uint8_t pwr_mgmt = MPU6050_ReadReg(MPU6050_RA_PWR_MGMT_1);
    lcd_clear();
    sprintf(str, "PWR: 0x%02X", pwr_mgmt);
    lcd_print_str(0, 0, str);
    Mdelay_Lib(2000);
    
    while(1)
    {
        // 获取MPU6050数据并进行解析
        MPU6050_Data_t mpu_data;
        int16_t accel_raw[3], gyro_raw[3];
        char temp_str[20];
        
        lcd_clear();
        
        // 获取原始数据
        MPU6050_GetAccel(&accel_raw[0], &accel_raw[1], &accel_raw[2]);
        MPU6050_GetGyro(&gyro_raw[0], &gyro_raw[1], &gyro_raw[2]);
        
        // 获取转换后的数据
        MPU6050_GetData(&mpu_data);
        
        // 第一行显示加速度数据（g为单位）
        sprintf(str, "A:%.2f,%.2f,%.2f", mpu_data.accel_x, mpu_data.accel_y, mpu_data.accel_z);
        lcd_print_str(0, 0, str);
        
        // 第二行显示陀螺仪数据（°/s为单位）
        sprintf(str, "G:%.1f,%.1f,%.1f", mpu_data.gyro_x, mpu_data.gyro_y, mpu_data.gyro_z);
        lcd_print_str(1, 0, str);
        
        Mdelay_Lib(500);  // 500ms更新一次
    }
}
