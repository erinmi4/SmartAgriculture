#ifndef __MPU6050_ANGLE_DISPLAY_H
#define __MPU6050_ANGLE_DISPLAY_H

#include "stm32f4xx.h"
#include "mpu6050.h"
#include "lcd1602.h"

// 初始化MPU6050和LCD1602
void MPU6050_LCD_Init(void);

// 读取MPU6050数据并显示在LCD1602上
void MPU6050_Read_And_Display(void);

#endif

