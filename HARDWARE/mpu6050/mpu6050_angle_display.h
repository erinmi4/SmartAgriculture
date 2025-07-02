#ifndef __MPU6050_ANGLE_DISPLAY_H
#define __MPU6050_ANGLE_DISPLAY_H

#include "stm32f4xx.h"
#include "mpu6050.h"
#include "lcd.h"        // 修改为系统中实际使用的LCD头文件

// 将原始数据转换为角度数据
void Convert_To_Angle(int16_t ax, int16_t ay, int16_t az, float *roll, float *pitch);

// 将浮点数转换为字符串
void Float_To_String(float num, char *str);

// 读取MPU6050数据并显示在LCD上（移除LCD初始化功能，避免冲突）
void MPU6050_Read_And_Display(void);

#endif

