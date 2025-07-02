/**
 * @file    main.c
 * @brief   智能农业监测系统主程序 - 按键控制多页面显示
 * @details 实现数据采集、报警判断、4按键4页面UI交互
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
#include "key.h"
#include <stdio.h>
#include <string.h>

/* =================== 第1步：全局变量定义 =================== */

// 报警阈值宏定义 - 可通过修改这些值来控制报警条件
#define TEMP_HIGH_THRESHOLD     35      // 温度上限 (°C)
#define TEMP_LOW_THRESHOLD      10      // 温度下限 (°C)
#define HUMID_HIGH_THRESHOLD    80      // 湿度上限 (%)
#define HUMID_LOW_THRESHOLD     30      // 湿度下限 (%)
#define LIGHT_LOW_THRESHOLD     200     // 光照下限
#define SMOKE_HIGH_THRESHOLD    3000    // 烟雾上限
#define SYSTEM_ERROR_THRESHOLD  10      // 系统错误累计上限

// 页面管理枚举
typedef enum {
    PAGE_TEMP_HUMID = 0,    // 按键0 - 温湿度页面
    PAGE_LIGHT_SMOKE,       // 按键1 - 光照/烟雾页面  
    PAGE_ATTITUDE,          // 按键2 - 姿态页面
    PAGE_SYSTEM_INFO,       // 按键3 - 系统信息页面
    PAGE_MAX
} page_t;

// 全局变量 - 页面状态
uint8_t current_page = PAGE_TEMP_HUMID;
uint8_t previous_page = 0xFF;  // 用于检测页面变化
uint32_t system_tick = 0;

// 全局变量 - 传感器数据
typedef struct {
    // 温湿度数据 (DHT11)
    uint8_t temperature;
    uint8_t humidity;
    uint8_t dht11_status;
    
    // 光照/烟雾数据 (ADC)
    uint16_t light_raw_value;
    uint16_t smoke_raw_value;
    uint8_t light_percent;
    float smoke_percent;
    
    // MPU6050姿态数据
    MPU6050_Data_t mpu_data;
    uint8_t mpu_status;
    
    // 系统状态
    uint8_t system_ready;
    uint32_t error_count;
    uint32_t data_update_count;
} SensorData_t;

SensorData_t sensor_data = {0};

// 全局变量 - 报警状态
typedef struct {
    uint8_t temp_alarm;     // 温度报警
    uint8_t humid_alarm;    // 湿度报警
    uint8_t light_alarm;    // 光照报警
    uint8_t smoke_alarm;    // 烟雾报警
    uint8_t system_alarm;   // 系统报警
    uint8_t any_alarm;      // 任何报警标志
} AlarmStatus_t;

AlarmStatus_t alarm_status = {0};

/* =================== 函数声明 =================== */
void System_Init(void);
void Sensors_Init(void);
void Data_Collection(void);
void Alarm_Check(void);
void Key_Handler(void);
void Display_Update(void);
void Display_TempHumid_Page(void);
void Display_LightSmoke_Page(void);
void Display_Attitude_Page(void);
void Display_SystemInfo_Page(void);

/* =================== 系统初始化函数 =================== */
void System_Init(void)
{
    // 基础系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 延时初始化
    Udelay_Lib(2000);
    
    // 外设初始化
    Led_Init();
    Beep_Init();
    Key_Init();
    
    // LCD初始化
    lcd_init();
    lcd_clear();
    
    // ADC初始化
    Adc3_Init();
    
    // 显示启动信息
    lcd_print_str(0, 0, "Smart Agriculture");
    lcd_print_str(1, 0, "System Starting...");
    Mdelay_Lib(2000);
}

void Sensors_Init(void)
{
    uint8_t init_status = 0;
    char str[20];
    
    lcd_clear();
    lcd_print_str(0, 0, "Sensors Init...");
    
    // DHT11初始化
    lcd_print_str(1, 0, "DHT11...");
    dht11_init();
    Mdelay_Lib(300);
    
    // 光敏电阻初始化
    lcd_print_str(1, 0, "Light Sensor...");
    Light_Init();
    Mdelay_Lib(300);
    
    // MPU6050初始化
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
    Mdelay_Lib(300);
    
    // 初始化完成
    lcd_clear();
    lcd_print_str(0, 0, "Init Complete!");
    sprintf(str, "Status: 0x%02X", init_status);
    lcd_print_str(1, 0, str);
    Mdelay_Lib(1500);
    
    // 显示操作提示
    lcd_clear();
    lcd_print_str(0, 0, "Press KEY0-3 to");
    lcd_print_str(1, 0, "switch pages");
    Mdelay_Lib(2000);
    
    sensor_data.system_ready = 1;
    previous_page = 0xFF; // 强制刷新第一页
}

/* =================== 第2步：数据采集和报警判断 =================== */
void Data_Collection(void)
{
    // 更新DHT11温湿度数据
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
    sensor_data.light_raw_value = Light_GetRawValue();
    sensor_data.light_percent = Light_GetValue();
    
    // 更新烟雾数据
    sensor_data.smoke_raw_value = Get_Adc3(ADC_Channel_5);
    sensor_data.smoke_percent = (float)sensor_data.smoke_raw_value / 4095.0f * 100.0f;
    
    // 更新MPU6050数据
    if(sensor_data.mpu_status)
    {
        MPU6050_GetData(&sensor_data.mpu_data);
    }
    
    // 更新计数器
    sensor_data.data_update_count++;
}

void Alarm_Check(void)
{
    // 重置报警状态
    memset(&alarm_status, 0, sizeof(alarm_status));
    
    // 检查温度报警
    if(sensor_data.dht11_status)
    {
        if(sensor_data.temperature > TEMP_HIGH_THRESHOLD || 
           sensor_data.temperature < TEMP_LOW_THRESHOLD)
        {
            alarm_status.temp_alarm = 1;
        }
        
        // 检查湿度报警
        if(sensor_data.humidity > HUMID_HIGH_THRESHOLD || 
           sensor_data.humidity < HUMID_LOW_THRESHOLD)
        {
            alarm_status.humid_alarm = 1;
        }
    }
    
    // 检查光照报警
    if(sensor_data.light_raw_value < LIGHT_LOW_THRESHOLD)
    {
        alarm_status.light_alarm = 1;
    }
    
    // 检查烟雾报警
    if(sensor_data.smoke_raw_value > SMOKE_HIGH_THRESHOLD)
    {
        alarm_status.smoke_alarm = 1;
    }
    
    // 检查系统报警
    if(sensor_data.error_count > SYSTEM_ERROR_THRESHOLD)
    {
        alarm_status.system_alarm = 1;
    }
    
    // 设置总报警标志
    alarm_status.any_alarm = (alarm_status.temp_alarm || alarm_status.humid_alarm || 
                             alarm_status.light_alarm || alarm_status.smoke_alarm || 
                             alarm_status.system_alarm);
    
    // 处理报警指示
    if(alarm_status.any_alarm)
    {
        // LED闪烁
        static uint32_t led_toggle_time = 0;
        if(system_tick - led_toggle_time >= 500)
        {
            led_toggle_time = system_tick;
            Led_Toggle(LED0);
        }
        
        // 蜂鸣器间歇鸣叫
        static uint32_t beep_time = 0;
        if(system_tick - beep_time >= 1000)
        {
            beep_time = system_tick;
            if((system_tick / 1000) % 2 == 0)
            {
                Beep_On();
            }
            else
            {
                Beep_Off();
            }
        }
    }
    else
    {
        // 关闭报警指示
        Led_Off(LED0);
        Beep_Off();
    }
}

/* =================== 第3步：按键处理和页面显示 =================== */
void Key_Handler(void)
{
    // 检测按键状态 - 使用防抖函数
    if(Key_Debounce(KEY0_GPIO, KEY0_PIN))
    {
        current_page = PAGE_TEMP_HUMID;
        Mdelay_Lib(200); // 按键防抖延时
    }
    else if(Key_Debounce(KEY1_GPIO, KEY1_PIN))
    {
        current_page = PAGE_LIGHT_SMOKE;
        Mdelay_Lib(200);
    }
    else if(Key_Debounce(KEY2_GPIO, KEY2_PIN))
    {
        current_page = PAGE_ATTITUDE;
        Mdelay_Lib(200);
    }
    else if(Key_Debounce(KEY3_GPIO, KEY3_PIN))
    {
        current_page = PAGE_SYSTEM_INFO;
        Mdelay_Lib(200);
    }
}

void Display_Update(void)
{
    // 只在页面变化时清屏，避免频繁刷新
    if(current_page != previous_page)
    {
        lcd_clear();
        previous_page = current_page;
    }
    
    // 根据当前页面显示内容
    switch(current_page)
    {
        case PAGE_TEMP_HUMID:
            Display_TempHumid_Page();
            break;
        case PAGE_LIGHT_SMOKE:
            Display_LightSmoke_Page();
            break;
        case PAGE_ATTITUDE:
            Display_Attitude_Page();
            break;
        case PAGE_SYSTEM_INFO:
            Display_SystemInfo_Page();
            break;
        default:
            current_page = PAGE_TEMP_HUMID;
            break;
    }
}

void Display_TempHumid_Page(void)
{
    char str[20];
    
    // 第一行：页面标题和报警指示
    if(alarm_status.temp_alarm || alarm_status.humid_alarm)
    {
        lcd_print_str(0, 0, "Temp&Humid [ALM]");
    }
    else
    {
        lcd_print_str(0, 0, "Temp & Humidity");
    }
    
    // 第二行：数据显示
    if(sensor_data.dht11_status)
    {
        sprintf(str, "T:%2d°C H:%2d%%", sensor_data.temperature, sensor_data.humidity);
        lcd_print_str(1, 0, str);
    }
    else
    {
        lcd_print_str(1, 0, "DHT11 Error!");
    }
}

void Display_LightSmoke_Page(void)
{
    char str[20];
    
    // 第一行：页面标题和报警指示
    if(alarm_status.light_alarm || alarm_status.smoke_alarm)
    {
        lcd_print_str(0, 0, "Light&Smoke[ALM]");
    }
    else
    {
        lcd_print_str(0, 0, "Light & Smoke");
    }
    
    // 第二行：数据显示
    sprintf(str, "L:%3d%% S:%3.0f%%", sensor_data.light_percent, sensor_data.smoke_percent);
    lcd_print_str(1, 0, str);
}

void Display_Attitude_Page(void)
{
    char str[20];
    
    // 第一行：页面标题
    lcd_print_str(0, 0, "MPU6050 Attitude");
    
    // 第二行：数据显示
    if(sensor_data.mpu_status)
    {
        sprintf(str, "X:%+.1fg Y:%+.1fg", sensor_data.mpu_data.accel_x, sensor_data.mpu_data.accel_y);
        lcd_print_str(1, 0, str);
    }
    else
    {
        lcd_print_str(1, 0, "MPU6050 Error!");
    }
}

void Display_SystemInfo_Page(void)
{
    char str[20];
    
    // 第一行：页面标题和报警指示
    if(alarm_status.system_alarm)
    {
        lcd_print_str(0, 0, "System Info[ALM]");
    }
    else
    {
        lcd_print_str(0, 0, "System Info");
    }
    
    // 第二行：运行时间或错误计数
    if(alarm_status.any_alarm)
    {
        sprintf(str, "Errors: %ld", sensor_data.error_count);
        lcd_print_str(1, 0, str);
    }
    else
    {
        sprintf(str, "Time: %lds", system_tick / 1000);
        lcd_print_str(1, 0, str);
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
        /* ========== 第2步：核心后台任务 ========== */
        // 数据采集模块 - 无条件读取所有传感器数据
        Data_Collection();
        
        // 报警逻辑模块 - 使用宏定义的阈值进行判断
        Alarm_Check();
        
        /* ========== 第3步：UI交互逻辑 ========== */
        // 按键状态获取 - 使用中断获取按键值
        Key_Handler();
        
        // 页面状态切换和显示分发
        Display_Update();
        
        // 更新系统时钟
        system_tick++;
        
        // 主循环延时 - 可根据需要调整
        Mdelay_Lib(50);  // 50ms循环周期，20Hz刷新率
    }
}
