#ifndef __MPU6050_ANGLE_DISPLAY_H
#define __MPU6050_ANGLE_DISPLAY_H

#include "stm32f4xx.h"
#include "mpu6050.h"
#include "lcd1602.h"

// ��ʼ��MPU6050��LCD1602
void MPU6050_LCD_Init(void);

// ��ȡMPU6050���ݲ���ʾ��LCD1602��
void MPU6050_Read_And_Display(void);

#endif

