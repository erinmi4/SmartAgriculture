#ifndef __MPU6050_ANGLE_DISPLAY_H
#define __MPU6050_ANGLE_DISPLAY_H

#include "stm32f4xx.h"
#include "mpu6050.h"
#include "lcd.h"        // �޸�Ϊϵͳ��ʵ��ʹ�õ�LCDͷ�ļ�

// ��ԭʼ����ת��Ϊ�Ƕ�����
void Convert_To_Angle(int16_t ax, int16_t ay, int16_t az, float *roll, float *pitch);

// ��������ת��Ϊ�ַ���
void Float_To_String(float num, char *str);

// ��ȡMPU6050���ݲ���ʾ��LCD�ϣ��Ƴ�LCD��ʼ�����ܣ������ͻ��
void MPU6050_Read_And_Display(void);

#endif

