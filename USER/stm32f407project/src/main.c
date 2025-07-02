/**
 * @file    main.c
 * @brief   智能农业监测系统主程序
 * @details 集成温湿度、光照、烟雾、姿态等传感器数据采集与LCD显示
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include "mpu6050.h"
#include "i2c.h"
#include "DHT11.h"
#include "light.h"
#include "ADC3.h"
#include "led.h"
#include "beep.h"
#include <stdio.h>
#include <string.h>

/* 全局变量定义 */
// 页面管理
typedef enum {
    PAGE_STARTUP = 0,      // 启动页面
    PAGE_SYSTEM_INFO,      // 系统信息页面
    PAGE_TEMP_HUMID,       // 温湿度页面
    PAGE_LIGHT_SMOKE,      // 光照/烟雾页面
    PAGE_ATTITUDE,         // 姿态页面
    PAGE_ALARM,            // 报警页面
    PAGE_MAX
} page_t;

uint8_t current_page = PAGE_STARTUP;
uint32_t page_switch_timer = 0;
uint32_t system_tick = 0;

// 传感器数据
typedef struct {
    // 温湿度数据
    uint8_t temperature;
    uint8_t humidity;
    uint8_t dht11_status;
    
    // 光照/烟雾数据
    uint16_t light_value;
    uint16_t smoke_value;
    float light_percent;
    float smoke_percent;
    
    // MPU6050姿态数据
    MPU6050_Data_t mpu_data;
    uint8_t mpu_status;
    
    // 系统状态
    uint8_t system_ready;
    uint32_t error_count;
} SensorData_t;

SensorData_t sensor_data = {0};

// 报警阈值设置
typedef struct {
    uint8_t temp_high;      // 温度上限
    uint8_t temp_low;       // 温度下限
    uint8_t humid_high;     // 湿度上限
    uint8_t humid_low;      // 湿度下限
    uint16_t light_low;     // 光照下限
    uint16_t smoke_high;    // 烟雾上限
} AlarmThresholds_t;

AlarmThresholds_t alarm_thresholds = {
    .temp_high = 35,        // 35°C
    .temp_low = 10,         // 10°C
    .humid_high = 80,       // 80%
    .humid_low = 30,        // 30%
    .light_low = 200,       // 光照不足阈值
    .smoke_high = 3000      // 烟雾报警阈值
};

// 报警状态
typedef struct {
    uint8_t temp_alarm;
    uint8_t humid_alarm;
    uint8_t light_alarm;
    uint8_t smoke_alarm;
    uint8_t system_alarm;
} AlarmStatus_t;

AlarmStatus_t alarm_status = {0};

/* 函数声明 */
void System_Init(void);
void Sensors_Init(void);
void Display_StartupInfo(void);
void Display_SystemInfo(void);
void Update_SensorData(void);
void Display_CurrentPage(void);
void Check_Alarms(void);
void Handle_Alarms(void);
void Page_Switch_Handler(void);

/**
 * @brief 系统初始化
 */
void System_Init(void)
{
    // 基础系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 延时初始化
    Udelay_Lib(2000);
    
    // LED和蜂鸣器初始化
    Led_Init();
    Beep_Init();
    
    // LCD初始化
    lcd_init();
    lcd_clear();
    
    // ADC初始化
    Adc3_Init();
    
    // 显示启动信息
    Display_StartupInfo();
}

/**
 * @brief 传感器初始化
 */
void Sensors_Init(void)
{
    uint8_t init_status = 0;
    char str[20];
    
    lcd_clear();
    lcd_print_str(0, 0, "Sensor Init...");
    
    // 1. DHT11初始化
    lcd_print_str(1, 0, "DHT11...");
    dht11_init();
    Mdelay_Lib(500);
    
    // 2. 光敏电阻初始化
    lcd_print_str(1, 0, "Light...");
    Light_Init();
    Mdelay_Lib(500);
    
    // 3. MPU6050初始化
    lcd_print_str(1, 0, "MPU6050...");
    if(MPU6050_Init() == 0)
    {
        sensor_data.mpu_status = 1;
        init_status |= 0x01;
    }
    else
    {
        sensor_data.error_count++;
    }
    Mdelay_Lib(500);
    
    // 显示初始化结果
    lcd_clear();
    lcd_print_str(0, 0, "Init Complete");
    sprintf(str, "Status: 0x%02X", init_status);
    lcd_print_str(1, 0, str);
    Mdelay_Lib(2000);
    
    // 标记系统就绪
    sensor_data.system_ready = 1;
}

/**
 * @brief 显示启动信息
 */
void Display_StartupInfo(void)
{
    lcd_clear();
    lcd_print_str(0, 0, "Smart Agriculture");
    lcd_print_str(1, 0, "System V1.0");
    Mdelay_Lib(2000);
    
    lcd_clear();
    lcd_print_str(0, 0, "STM32F407 Based");
    lcd_print_str(1, 0, "Multi-Sensor");
    Mdelay_Lib(2000);
    
    lcd_clear();
    lcd_print_str(0, 0, "Initializing...");
    lcd_print_str(1, 0, "Please Wait");
    Mdelay_Lib(1000);
}

/**
 * @brief 显示系统信息
 */
void Display_SystemInfo(void)
{
    char str[20];
    
    lcd_clear();
    lcd_print_str(0, 0, "System Info");
    
    // 显示运行时间（秒）
    sprintf(str, "Time: %lds", system_tick / 1000);
    lcd_print_str(1, 0, str);
}

/**
 * @brief 更新传感器数据
 */
void Update_SensorData(void)
{
    static uint32_t last_update = 0;
    
    // 每500ms更新一次传感器数据
    if(system_tick - last_update >= 500)
    {
        last_update = system_tick;
        
        // 更新DHT11数据
        if(dht11_read_dat(&sensor_data.temperature, &sensor_data.humidity) == 0)
        {
            sensor_data.dht11_status = 1;
        }
        else
        {
            sensor_data.dht11_status = 0;
            sensor_data.error_count++;
        }
        
        // 更新光照数据
        sensor_data.light_value = Light_GetRawValue();
        sensor_data.light_percent = (float)Light_GetValue();
        
        // 更新烟雾数据 - 假设使用ADC通道5
        sensor_data.smoke_value = Get_Adc3(ADC_Channel_5);
        sensor_data.smoke_percent = (float)sensor_data.smoke_value / 4095.0f * 100.0f;
        
        // 更新MPU6050数据
        if(sensor_data.mpu_status)
        {
            MPU6050_GetData(&sensor_data.mpu_data);
        }
        
        // 检查报警条件
        Check_Alarms();
    }
}

/**
 * @brief 检查报警条件
 */
void Check_Alarms(void)
{
    // 重置报警状态
    memset(&alarm_status, 0, sizeof(alarm_status));
    
    // 检查温度报警
    if(sensor_data.dht11_status)
    {
        if(sensor_data.temperature > alarm_thresholds.temp_high || 
           sensor_data.temperature < alarm_thresholds.temp_low)
        {
            alarm_status.temp_alarm = 1;
        }
        
        // 检查湿度报警
        if(sensor_data.humidity > alarm_thresholds.humid_high || 
           sensor_data.humidity < alarm_thresholds.humid_low)
        {
            alarm_status.humid_alarm = 1;
        }
    }
    
    // 检查光照报警
    if(sensor_data.light_value < alarm_thresholds.light_low)
    {
        alarm_status.light_alarm = 1;
    }
    
    // 检查烟雾报警
    if(sensor_data.smoke_value > alarm_thresholds.smoke_high)
    {
        alarm_status.smoke_alarm = 1;
    }
    
    // 检查系统报警
    if(sensor_data.error_count > 10)
    {
        alarm_status.system_alarm = 1;
    }
}

/**
 * @brief 处理报警
 */
void Handle_Alarms(void)
{
    static uint32_t last_beep = 0;
    
    // 如果有任何报警，切换到报警页面
    if(alarm_status.temp_alarm || alarm_status.humid_alarm || 
       alarm_status.light_alarm || alarm_status.smoke_alarm || 
       alarm_status.system_alarm)
    {
        // 闪烁LED
        Led_Toggle(LED0);
        
        // 蜂鸣器报警（每秒一次）
        if(system_tick - last_beep >= 1000)
        {
            last_beep = system_tick;
            if(system_tick % 2000 < 1000)
            {
                Beep_On();
            }
            else
            {
                Beep_Off();
            }
        }
        
        // 切换到报警页面
        current_page = PAGE_ALARM;
    }
    else
    {
        // 关闭报警指示
        Led_Off(LED0);  // 关闭LED
        Beep_Off();     // 关闭蜂鸣器
    }
}

/**
 * @brief 页面切换处理
 */
void Page_Switch_Handler(void)
{
    static uint32_t last_switch = 0;
    
    // 如果没有报警，每3秒自动切换页面
    if(!alarm_status.temp_alarm && !alarm_status.humid_alarm && 
       !alarm_status.light_alarm && !alarm_status.smoke_alarm && 
       !alarm_status.system_alarm)
    {
        if(system_tick - last_switch >= 3000)
        {
            last_switch = system_tick;
            current_page++;
            if(current_page >= PAGE_ALARM)
            {
                current_page = PAGE_SYSTEM_INFO;
            }
        }
    }
}

/**
 * @brief 显示当前页面
 */
void Display_CurrentPage(void)
{
    char str[20];
    
    switch(current_page)
    {
        case PAGE_STARTUP:
            // 启动页面在初始化时已显示
            break;
            
        case PAGE_SYSTEM_INFO:
            Display_SystemInfo();
            break;
            
        case PAGE_TEMP_HUMID:
            lcd_clear();
            lcd_print_str(0, 0, "Temp & Humidity");
            if(sensor_data.dht11_status)
            {
                sprintf(str, "T:%d°C H:%d%%", sensor_data.temperature, sensor_data.humidity);
                lcd_print_str(1, 0, str);
            }
            else
            {
                lcd_print_str(1, 0, "DHT11 Error");
            }
            break;
            
        case PAGE_LIGHT_SMOKE:
            lcd_clear();
            lcd_print_str(0, 0, "Light & Smoke");
            sprintf(str, "L:%.0f%% S:%.0f%%", sensor_data.light_percent, sensor_data.smoke_percent);
            lcd_print_str(1, 0, str);
            break;
            
        case PAGE_ATTITUDE:
            lcd_clear();
            lcd_print_str(0, 0, "MPU6050 Attitude");
            if(sensor_data.mpu_status)
            {
                sprintf(str, "X:%.1f Y:%.1f", sensor_data.mpu_data.accel_x, sensor_data.mpu_data.accel_y);
                lcd_print_str(1, 0, str);
            }
            else
            {
                lcd_print_str(1, 0, "MPU6050 Error");
            }
            break;
            
        case PAGE_ALARM:
            lcd_clear();
            lcd_print_str(0, 0, "!!! ALARM !!!");
            if(alarm_status.temp_alarm)
                lcd_print_str(1, 0, "Temperature!");
            else if(alarm_status.humid_alarm)
                lcd_print_str(1, 0, "Humidity!");
            else if(alarm_status.light_alarm)
                lcd_print_str(1, 0, "Light Low!");
            else if(alarm_status.smoke_alarm)
                lcd_print_str(1, 0, "Smoke High!");
            else if(alarm_status.system_alarm)
                lcd_print_str(1, 0, "System Error!");
            break;
            
        default:
            current_page = PAGE_SYSTEM_INFO;
            break;
    }
}

int main(void)
{
    // 系统初始化
    System_Init();
    
    // 传感器初始化
    Sensors_Init();
    
    // 主循环
    while(1)
    {
        // 更新系统时钟
        system_tick++;
        
        // 更新传感器数据
        Update_SensorData();
        
        // 处理报警
        Handle_Alarms();
        
        // 页面切换处理
        Page_Switch_Handler();
        
        // 显示当前页面
        Display_CurrentPage();
        
        // 延时1ms
        Mdelay_Lib(1);
    }
}
