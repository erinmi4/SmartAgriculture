#include "mpu6050_angle_display.h"
#include <math.h>
#include <stdio.h>  // ���� stdio.h ��ʹ�� sprintf ����

// �ֶ����� M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ��ʼ��MPU6050��LCD1602
void MPU6050_LCD_Init(void)
{
    MPU6050_Init();
    lcd_init();
}

// ��ԭʼ����ת��Ϊ�Ƕ�����
void Convert_To_Angle(int16_t ax, int16_t ay, int16_t az, float *roll, float *pitch)
{
    *roll = atan2(ay, az) * 180 / M_PI;
    *pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / M_PI;
}

// ��������ת��Ϊ�ַ���
void Float_To_String(float num, char *str)
{
    int int_part = (int)num;
    int dec_part = (int)((num - int_part) * 100);
    sprintf(str, "%d.%02d", int_part, dec_part);
}

// ��ȡMPU6050���ݲ���ʾ��LCD1602��
void MPU6050_Read_And_Display(void)
{
    int16_t ax, ay, az;
    float roll, pitch;
    char roll_str[10];
    char pitch_str[10];

    // ��ȡ���ٶȼ�����
    MPU6050_Read_Accel(&ax, &ay, &az);

    // ��ԭʼ����ת��Ϊ�Ƕ�����
    Convert_To_Angle(ax, ay, az, &roll, &pitch);

    // ��������ת��Ϊ�ַ���
    Float_To_String(roll, roll_str);
    Float_To_String(pitch, pitch_str);

    // ��LCD1602����ʾ����
    lcd_print_str(0, 0, "Roll:");  // Roll����������������ߣ����ᣩ����ת�Ƕ�,���������б��Roll��Ϊ����������б��Roll��Ϊ��
    lcd_print_str(0, 6, roll_str);
    lcd_print_str(1, 0, "Pitch:"); // �������̧����Pitch��Ϊ����������£�Pitch��Ϊ����
    lcd_print_str(1, 7, pitch_str);
}

