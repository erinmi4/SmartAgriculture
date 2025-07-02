/**
 * @file    main_all_sensors.c
 * @brief   智能农业系统综合测试程序
 * @details 集成光敏传感器、DHT11、MPU6050等传感器，在LCD上轮流显示数据
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "light.h"
#include "mpu6050.h"
#include "DHT11.h"
#include <stdio.h>

// 显示模式枚举
typedef enum
{
    DISPLAY_LIGHT = 0,      // 显示光照数据
    DISPLAY_DHT11,          // 显示温湿度数据
    DISPLAY_MPU6050_ACCEL,  // 显示加速度数据
    DISPLAY_MPU6050_GYRO,   // 显示陀螺仪数据
    DISPLAY_MAX
} DisplayMode_t;

int main(void)
{
    DisplayMode_t display_mode = DISPLAY_LIGHT;
    uint32_t last_switch_time = 0;
    const uint32_t SWITCH_INTERVAL = 3000;  // 3秒切换一次显示模式
    
    // 传感器数据变量
    uint16_t light_raw = 0;
    uint8_t light_percent = 0;
    LightLevel_t light_level;
    
    MPU6050_Data_t mpu_data;
    
    uint8_t temperature, humidity;
    
    char str[32];
    
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    lcd_clear();
    
    // 显示启动信息
    lcd_print_str(0, 0, "Smart Agriculture");
    lcd_print_str(1, 0, "System Starting...");
    Mdelay_Lib(2000);
    
    // 初始化各个传感器
    lcd_clear();
    lcd_print_str(0, 0, "Init Sensors...");
    
    // 初始化光敏传感器
    Light_Init();
    lcd_print_str(1, 0, "Light OK");
    Mdelay_Lib(500);
    
    // 初始化DHT11
    DHT11_Init();
    lcd_print_str(1, 0, "DHT11 OK");
    Mdelay_Lib(500);
    
    // 初始化MPU6050
    if(MPU6050_Init() == 0)
    {
        lcd_print_str(1, 0, "MPU6050 OK");
    }
    else
    {
        lcd_print_str(1, 0, "MPU6050 Fail");
    }
    Mdelay_Lib(1000);
    
    lcd_clear();
    last_switch_time = 0;  // 重置时间计数
    
    while(1)
    {
        // 检查是否需要切换显示模式
        if((last_switch_time % SWITCH_INTERVAL) == 0)
        {
            display_mode = (DisplayMode_t)((display_mode + 1) % DISPLAY_MAX);
            lcd_clear();
        }
        
        // 根据当前显示模式显示不同的传感器数据
        switch(display_mode)
        {
            case DISPLAY_LIGHT:
                // 获取光照数据
                light_raw = Light_GetRawValue();
                light_percent = Light_GetValue();
                light_level = Light_GetLevel();
                
                // 显示光照数据
                sprintf(str, "Light: %u%% (%u)", light_percent, light_raw);
                lcd_print_str(0, 0, str);
                sprintf(str, "Level: %s", Light_GetLevelString(light_level));
                lcd_print_str(1, 0, str);
                break;
                
            case DISPLAY_DHT11:
                // 获取温湿度数据
                if(DHT11_Read_Data(&temperature, &humidity) == 0)
                {
                    sprintf(str, "Temp: %u'C", temperature);
                    lcd_print_str(0, 0, str);
                    sprintf(str, "Humi: %u%%", humidity);
                    lcd_print_str(1, 0, str);
                }
                else
                {
                    lcd_print_str(0, 0, "DHT11 Error");
                    lcd_print_str(1, 0, "Check Connection");
                }
                break;
                
            case DISPLAY_MPU6050_ACCEL:
                // 获取加速度数据
                MPU6050_GetData(&mpu_data);
                
                sprintf(str, "Accel(g)");
                lcd_print_str(0, 0, str);
                sprintf(str, "X%2.1f Y%2.1f Z%2.1f", 
                        mpu_data.accel_x, mpu_data.accel_y, mpu_data.accel_z);
                lcd_print_str(1, 0, str);
                break;
                
            case DISPLAY_MPU6050_GYRO:
                // 获取陀螺仪数据
                MPU6050_GetData(&mpu_data);
                
                sprintf(str, "Gyro(dps)");
                lcd_print_str(0, 0, str);
                sprintf(str, "X%3.0f Y%3.0f Z%3.0f", 
                        mpu_data.gyro_x, mpu_data.gyro_y, mpu_data.gyro_z);
                lcd_print_str(1, 0, str);
                break;
                
            default:
                display_mode = DISPLAY_LIGHT;
                break;
        }
        
        Mdelay_Lib(100);  // 100ms更新一次
        last_switch_time += 100;
        
        // 防止溢出
        if(last_switch_time >= (SWITCH_INTERVAL * DISPLAY_MAX))
        {
            last_switch_time = 0;
        }
    }
}
