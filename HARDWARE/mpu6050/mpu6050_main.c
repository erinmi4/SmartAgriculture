#include "stm32f4xx.h"
#include "mpu6050_angle_display.h"
#include "delay.h"

int main(void)
{
    // 初始化MPU6050和LCD1602
    MPU6050_LCD_Init();

    while (1)
    {
        // 读取MPU6050数据并显示在LCD1602上
        MPU6050_Read_And_Display();

        // 延时一段时间
        Mdelay_Lib(500);
    }
}