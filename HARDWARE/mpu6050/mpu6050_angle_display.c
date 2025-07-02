#include "mpu6050_angle_display.h"
#include <math.h>
#include <stdio.h>  // 包含 stdio.h 以使用 sprintf 函数

// 手动定义 M_PI
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 初始化MPU6050和LCD1602
void MPU6050_LCD_Init(void)
{
    MPU6050_Init();
    lcd_init();
}

// 将原始数据转换为角度数据
void Convert_To_Angle(int16_t ax, int16_t ay, int16_t az, float *roll, float *pitch)
{
    *roll = atan2(ay, az) * 180 / M_PI;
    *pitch = atan2(-ax, sqrt(ay * ay + az * az)) * 180 / M_PI;
}

// 将浮点数转换为字符串
void Float_To_String(float num, char *str)
{
    int int_part = (int)num;
    int dec_part = (int)((num - int_part) * 100);
    sprintf(str, "%d.%02d", int_part, dec_part);
}

// 读取MPU6050数据并显示在LCD1602上
void MPU6050_Read_And_Display(void)
{
    int16_t ax, ay, az;
    float roll, pitch;
    char roll_str[10];
    char pitch_str[10];

    // 读取加速度计数据
    MPU6050_Read_Accel(&ax, &ay, &az);

    // 将原始数据转换为角度数据
    Convert_To_Angle(ax, ay, az, &roll, &pitch);

    // 将浮点数转换为字符串
    Float_To_String(roll, roll_str);
    Float_To_String(pitch, pitch_str);

    // 在LCD1602上显示数据
    lcd_print_str(0, 0, "Roll:");  // Roll角是绕其机身中心线（纵轴）的旋转角度,如果向左倾斜，Roll角为负；向右倾斜，Roll角为正
    lcd_print_str(0, 6, roll_str);
    lcd_print_str(1, 0, "Pitch:"); // 如果向上抬升，Pitch角为正；如果向下，Pitch角为负。
    lcd_print_str(1, 7, pitch_str);
}

