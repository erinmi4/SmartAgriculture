/* =================== 调试开关 =================== */
// 如果出现卡死问题，可以逐个禁用模块进行排查
#define ENABLE_DHT11     0    // 1-启用DHT11, 0-禁用DHT11 (先禁用排查)
#define ENABLE_MPU6050   0    // 1-启用MPU6050, 0-禁用MPU6050 (先禁用排查)
#define ENABLE_MQ2       1    // 1-启用MQ2, 0-禁用MQ2  
#define ENABLE_LIGHT     1    // 1-启用光敏电阻, 0-禁用光敏电阻

/* =================== 头文件包含 =================== */
#include "stm32f4xx.h"
#include "delay.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "sys.h"
#include "DHT11.h"
#include "light.h"
#include "mpu6050.h"
#include "mq2.h"
#include "bluetooth.h"
#include "beep.h"
#include "ADC3.h"
#include <stdio.h>
#include <string.h>

/* =================== 系统定义 =================== */
#define TEMP_HIGH_THRESHOLD  30    // 温度高报警阈值(℃)  
#define TEMP_LOW_THRESHOLD   10    // 温度低报警阈值(℃)
#define HUMI_HIGH_THRESHOLD  80    // 湿度高报警阈值(%)
#define HUMI_LOW_THRESHOLD   20    // 湿度低报警阈值(%)
#define LIGHT_LOW_THRESHOLD  20    // 光照低报警阈值(0-100)
#define SMOKE_HIGH_THRESHOLD 30    // 烟雾高报警阈值(%)

// 页面枚举
typedef enum {
    PAGE_TEMP_HUMID = 0,
    PAGE_LIGHT_SMOKE,
    PAGE_ATTITUDE,
    PAGE_SYSTEM_INFO
} PageType_t;

// 全局传感器数据结构
typedef struct {
    // 温湿度数据 (DHT11)
    uint8_t temperature;
    uint8_t humidity;
    uint8_t dht11_status;
    
    // 光照数据
    uint16_t light_raw_value;
    uint8_t light_percent;
    
    // 烟雾数据 (MQ-2)
    uint16_t smoke_raw_value;
    float smoke_percent;
    
    // 姿态数据 (MPU6050)
    MPU6050_Data_t mpu_data;
    uint8_t mpu_status;
    
    // 系统状态
    uint32_t error_count;
    uint32_t data_update_count;
} SensorData_t;

// 报警状态结构
typedef struct {
    uint8_t temp_high_alarm;
    uint8_t temp_low_alarm; 
    uint8_t humi_high_alarm;
    uint8_t humi_low_alarm;
    uint8_t light_low_alarm;
    uint8_t smoke_high_alarm;
    uint8_t any_alarm;
} AlarmStatus_t;

/* =================== 全局变量 =================== */
SensorData_t sensor_data = {0};
AlarmStatus_t alarm_status = {0};
PageType_t current_page = PAGE_TEMP_HUMID;
PageType_t previous_page = PAGE_SYSTEM_INFO; // 初始化为不同值，强制第一次刷新
uint32_t system_tick = 0;

/* =================== 函数声明 =================== */
void System_Init(void);
void Sensors_Init(void);
void Data_Collection(void);
void Alarm_Check(void);
void Key_Handler(void);
void Display_Update(void);

/* =================== 第1步：系统和传感器初始化 =================== */
void System_Init(void)
{
    char str[50];
    
    // 系统时钟初始化
    SystemInit();
    
    // 延时初始化
    delay_init(168);  // 168MHz系统时钟
    
    // LCD初始化并显示欢迎信息
    lcd_init();
    lcd_clear();
    lcd_print_str(0, 0, "Smart Agriculture");
    lcd_print_str(1, 0, "Initializing...");
    Mdelay_Lib(1000);
    
    // LED初始化
    Led_Init();
    
    // 按键初始化
    Key_Init();
    
    // 蜂鸣器初始化
    beep_init();
    
    // ADC初始化
    Adc3_Init();
    
    lcd_print_str(1, 0, "System OK");
    Mdelay_Lib(500);
}

void Sensors_Init(void)
{
    char str[50];
    uint8_t init_status = 0;
    
#if ENABLE_DHT11
    // DHT11初始化
    lcd_print_str(1, 0, "DHT11...");
    dht11_init();
    Mdelay_Lib(500);
    
    // 测试DHT11读取
    uint8_t test_temp, test_humi;
    if(dht11_read_dat(&test_temp, &test_humi) == 0)
    {
        sensor_data.dht11_status = 1;
        sprintf(str, "DHT11 OK: %d,%d", test_temp, test_humi);
        lcd_print_str(1, 0, str);
    }
    else
    {
        sensor_data.dht11_status = 0;
        lcd_print_str(1, 0, "DHT11 Failed");
    }
    Mdelay_Lib(1000);
#else
    lcd_print_str(1, 0, "DHT11 Disabled");
    sensor_data.dht11_status = 0;
    Mdelay_Lib(500);
#endif

#if ENABLE_LIGHT
    // 光敏电阻初始化
    lcd_print_str(1, 0, "Light Sensor...");
    Light_Init();
    Mdelay_Lib(500);
#endif

#if ENABLE_MPU6050
    // MPU6050初始化
    lcd_print_str(1, 0, "MPU6050...");
    if(MPU6050_Init() == 0)
    {
        sensor_data.mpu_status = 1;
        init_status |= 0x01;
        lcd_print_str(1, 0, "MPU6050 OK");
    }
    else
    {
        sensor_data.error_count++;
        lcd_print_str(1, 0, "MPU6050 Failed");
    }
    Mdelay_Lib(1000);
#else
    lcd_print_str(1, 0, "MPU6050 Disabled");
    sensor_data.mpu_status = 0;
    Mdelay_Lib(500);
#endif
    
    // 初始化完成
    sprintf(str, "Init Complete!");
    lcd_print_str(1, 0, str);
    Mdelay_Lib(1000);
    
    // 清屏准备进入主循环显示
    lcd_clear();
}

/* =================== 第2步：数据采集 =================== */
void Data_Collection(void)
{
    static uint32_t last_read = 0;
    
    // 每500ms读取一次所有传感器
    if(system_tick - last_read >= 500)
    {
        last_read = system_tick;
        
#if ENABLE_DHT11
        // DHT11数据采集 - 只在状态正常时读取
        if(sensor_data.dht11_status)
        {
            if(dht11_read_dat(&sensor_data.temperature, &sensor_data.humidity) != 0)
            {
                sensor_data.error_count++;
            }
        }
#else
        // DHT11被禁用，设置默认值
        sensor_data.temperature = 25;
        sensor_data.humidity = 60;
#endif

#if ENABLE_LIGHT
        // 光照数据采集
        sensor_data.light_raw_value = Light_GetRawValue();
        if(sensor_data.light_raw_value > 4095) sensor_data.light_raw_value = 4095;
        sensor_data.light_percent = 100 - (sensor_data.light_raw_value * 100 / 4095);
#else
        sensor_data.light_raw_value = 2048;
        sensor_data.light_percent = 50;
#endif

#if ENABLE_MQ2
        // MQ2烟雾数据采集
        sensor_data.smoke_raw_value = Get_Adc3(ADC_Channel_5);
        sensor_data.smoke_percent = (float)sensor_data.smoke_raw_value / 4095.0f * 100.0f;
#else
        sensor_data.smoke_raw_value = 1024;
        sensor_data.smoke_percent = 25.0f;
#endif

#if ENABLE_MPU6050
        // MPU6050数据采集
        if(sensor_data.mpu_status)
        {
            MPU6050_GetData(&sensor_data.mpu_data);
        }
#else
        sensor_data.mpu_data.accel_x = 0.0f;
        sensor_data.mpu_data.accel_y = 0.0f;
        sensor_data.mpu_data.accel_z = 1.0f;
        sensor_data.mpu_data.gyro_x = 0.0f;
        sensor_data.mpu_data.gyro_y = 0.0f;
        sensor_data.mpu_data.gyro_z = 0.0f;
        sensor_data.mpu_data.temp = 25.0f;
#endif
        
        sensor_data.data_update_count++;
    }
}

/* =================== 第3步：报警检查 =================== */
void Alarm_Check(void)
{
    // 温度报警检查
    alarm_status.temp_high_alarm = (sensor_data.temperature > TEMP_HIGH_THRESHOLD);
    alarm_status.temp_low_alarm = (sensor_data.temperature < TEMP_LOW_THRESHOLD);
    
    // 湿度报警检查
    alarm_status.humi_high_alarm = (sensor_data.humidity > HUMI_HIGH_THRESHOLD);
    alarm_status.humi_low_alarm = (sensor_data.humidity < HUMI_LOW_THRESHOLD);
    
    // 光照报警检查
    alarm_status.light_low_alarm = (sensor_data.light_percent < LIGHT_LOW_THRESHOLD);
    
    // 烟雾报警检查
    alarm_status.smoke_high_alarm = (sensor_data.smoke_percent > SMOKE_HIGH_THRESHOLD);
    
    // 综合报警状态
    alarm_status.any_alarm = alarm_status.temp_high_alarm || alarm_status.temp_low_alarm ||
                            alarm_status.humi_high_alarm || alarm_status.humi_low_alarm ||
                            alarm_status.light_low_alarm || alarm_status.smoke_high_alarm;
    
    // 蜂鸣器报警
    if(alarm_status.any_alarm)
    {
        beep_on();
        Mdelay_Lib(100);
        beep_off();
    }
}

/* =================== 第4步：按键处理 =================== */
void Key_Handler(void)
{
    static uint32_t last_key_time = 0;
    static uint8_t key_pressed[4] = {0};
    
    // 按键防抖：200ms内只处理一次按键
    if(system_tick - last_key_time < 200)
        return;
    
    // 检测KEY0 - 温湿度页面
    if(Key_Debounce(KEY0_GPIO, KEY0_PIN))
    {
        if(!key_pressed[0])
        {
            current_page = PAGE_TEMP_HUMID;
            last_key_time = system_tick;
            key_pressed[0] = 1;
        }
    }
    else
    {
        key_pressed[0] = 0;
    }
    
    // 检测KEY1 - 光照烟雾页面
    if(Key_Debounce(KEY1_GPIO, KEY1_PIN))
    {
        if(!key_pressed[1])
        {
            current_page = PAGE_LIGHT_SMOKE;
            last_key_time = system_tick;
            key_pressed[1] = 1;
        }
    }
    else
    {
        key_pressed[1] = 0;
    }
    
    // 检测KEY2 - 姿态页面
    if(Key_Debounce(KEY2_GPIO, KEY2_PIN))
    {
        if(!key_pressed[2])
        {
            current_page = PAGE_ATTITUDE;
            last_key_time = system_tick;
            key_pressed[2] = 1;
        }
    }
    else
    {
        key_pressed[2] = 0;
    }
    
    // 检测KEY3 - 系统信息页面
    if(Key_Debounce(KEY3_GPIO, KEY3_PIN))
    {
        if(!key_pressed[3])
        {
            current_page = PAGE_SYSTEM_INFO;
            last_key_time = system_tick;
            key_pressed[3] = 1;
        }
    }
    else
    {
        key_pressed[3] = 0;
    }
}

/* =================== 第5步：显示更新 =================== */
void Display_Update(void)
{
    char str[50];
    
    // 只在页面变化时清屏
    if(current_page != previous_page)
    {
        lcd_clear();
        previous_page = current_page;
    }
    
    // 根据当前页面显示内容
    switch(current_page)
    {
        case PAGE_TEMP_HUMID:
            lcd_print_str(0, 0, "=== Temp/Humid ===");
            sprintf(str, "Temp: %d C %s", sensor_data.temperature, 
                    alarm_status.temp_high_alarm ? "HIGH!" : 
                    (alarm_status.temp_low_alarm ? "LOW!" : "OK"));
            lcd_print_str(1, 0, str);
            break;
            
        case PAGE_LIGHT_SMOKE:
            lcd_print_str(0, 0, "=== Light/Smoke ===");
            sprintf(str, "Light: %d%% Smoke: %.0f%%", 
                    sensor_data.light_percent, sensor_data.smoke_percent);
            lcd_print_str(1, 0, str);
            break;
            
        case PAGE_ATTITUDE:
            lcd_print_str(0, 0, "=== MPU6050 ===");
            sprintf(str, "X:%.1f Y:%.1f Z:%.1f", 
                    sensor_data.mpu_data.accel_x,
                    sensor_data.mpu_data.accel_y,
                    sensor_data.mpu_data.accel_z);
            lcd_print_str(1, 0, str);
            break;
            
        case PAGE_SYSTEM_INFO:
            lcd_print_str(0, 0, "=== System Info ===");
            sprintf(str, "Time:%lds Err:%ld", 
                    system_tick / 1000, sensor_data.error_count);
            lcd_print_str(1, 0, str);
            break;
    }
}

/* =================== 主函数 =================== */
int main(void)
{
    // 系统初始化
    System_Init();
    
    // 传感器初始化
    Sensors_Init();
    
    // 主循环
    while(1)
    {
        // LED闪烁指示系统运行
        if(system_tick % 1000 == 0)
        {
            LED0_Toggle();
        }
        
        // 数据采集
        Data_Collection();
        
        // 报警检查
        Alarm_Check();
        
        // 按键处理
        Key_Handler();
        
        // 显示更新
        Display_Update();
        
        // 主循环延时
        Mdelay_Lib(10);  // 10ms延时，降低CPU占用
        
        // 更新系统时钟
        system_tick += 10;
    }
}
