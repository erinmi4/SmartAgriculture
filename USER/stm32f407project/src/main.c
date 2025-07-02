/* =================== 调试开关 =================== */
// 临时全部禁用传感器，只保留LCD和蓝牙进行基础功能调试
#define ENABLE_DHT11       0    // 1-启用DHT11, 0-禁用DHT11 (温湿度传感器) - 临时禁用调试LCD/蓝牙问题
#define ENABLE_MPU6050     0    // 1-启用MPU6050, 0-禁用MPU6050 (姿态传感器) - 临时禁用调试LCD/蓝牙问题
#define ENABLE_MQ2         0    // 1-启用MQ2, 0-禁用MQ2 (烟雾传感器) - 临时禁用调试LCD/蓝牙问题
#define ENABLE_LIGHT       0    // 1-启用光敏电阻, 0-禁用光敏电阻 (简单ADC) - 临时禁用调试LCD/蓝牙问题
#define ENABLE_BLUETOOTH   1    // 1-启用蓝牙, 0-禁用蓝牙 (远程控制) - 保留用于调试
#define ENABLE_BREATHING   0    // 1-启用呼吸灯, 0-禁用呼吸灯 (状态指示) - 临时禁用调试LCD/蓝牙问题
#define ENABLE_ALARM       0    // 1-启用报警, 0-禁用报警 (蜂鸣器和LED) - 临时禁用调试LCD/蓝牙问题

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
#include "mpu6050_angle_display.h"  // 添加角度显示功能
#include "mq2.h"
#include "bluetooth.h"
#include "beep.h"
#include "ADC3.h"
#include "uart.h"        // 添加UART头文件以支持UART_BAUD_9600
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* =================== 缺失函数声明 =================== */
// 注：实际的函数名在对应的头文件中定义
// delay_init 不存在，可以删除或创建空函数
// beep_init -> Beep_Init
// beep_on -> Beep_On  
// beep_off -> Beep_Off
// LED0_Toggle -> Led_Toggle

/* =================== 系统定义 =================== */
#define TEMP_HIGH_THRESHOLD  29    // 温度高报警阈值(℃)  
#define TEMP_LOW_THRESHOLD   20    // 温度低报警阈值(℃)
#define HUMI_HIGH_THRESHOLD  70    // 湿度高报警阈值(%)
#define HUMI_LOW_THRESHOLD   40    // 湿度低报警阈值(%)
#define LIGHT_LOW_THRESHOLD  40    // 光照低报警阈值(0-100)
#define SMOKE_HIGH_THRESHOLD 120   // 烟雾高报警阈值(ppm) - 方便演示取120

/* =================== 蓝牙参数化命令定义 =================== */
// 改进命令格式: 两位数字命令，例如 "01" 设置温度高阈值为35℃
// 支持的命令:
// 01 - 设置温度高阈值为35℃
// 02 - 设置温度低阈值为15℃  
// 03 - 设置湿度高阈值为80%
// 04 - 设置湿度低阈值为30%
// 05 - 设置光照低阈值为30%
// 06 - 设置烟雾高阈值为200ppm
// 07 - 启用报警并强制测试
// 08 - 禁用所有报警
// 09 - 查询当前状态
// 00 - 恢复默认阈值

#define BT_CMD_BUFFER_SIZE      20     // 蓝牙命令缓冲区大小

/* =================== LCD提示信息定义 =================== */
// LCD阈值修改提示结构
typedef struct {
    uint8_t active;           // 是否显示提示
    uint32_t start_time;      // 提示开始时间
    uint32_t duration;        // 提示持续时间(ms)
    char message[32];         // 提示消息内容
} LCD_Notification_t;

// 页面枚举
typedef enum {
    PAGE_TEMP_HUMID = 0,
    PAGE_LIGHT_SMOKE,
    PAGE_ATTITUDE,
    PAGE_BLUETOOTH,         // 新增蓝牙状态页面
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
    uint16_t smoke_ppm_value;        // MQ-2 ppm数值
    float smoke_percent;             // 保留百分比用于报警判断
    
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

// 动态阈值结构 - 支持蓝牙远程修改
typedef struct {
    uint8_t temp_high;      // 温度高阈值
    uint8_t temp_low;       // 温度低阈值
    uint8_t humi_high;      // 湿度高阈值
    uint8_t humi_low;       // 湿度低阈值
    uint8_t light_low;      // 光照低阈值
    uint16_t smoke_high;    // 烟雾高阈值
} Thresholds_t;

// 蓝牙状态结构
typedef struct {
    uint8_t enabled;        // 蓝牙是否启用
    uint8_t connected;      // 是否有设备连接
    uint32_t last_command;  // 上次命令时间
    uint32_t command_count; // 命令计数
} BluetoothState_t;

/* =================== 全局变量 =================== */
SensorData_t sensor_data = {0};
AlarmStatus_t alarm_status = {0};
PageType_t current_page = PAGE_TEMP_HUMID;
PageType_t previous_page = PAGE_SYSTEM_INFO; // 初始化为不同值，强制第一次刷新
volatile uint32_t system_tick = 0;           // 改为volatile，由SysTick中断更新，供delay.c访问
uint8_t alarm_disabled = 0;                // 报警禁用标志（命令8使用）

// 动态阈值实例 - 初始化为默认值
Thresholds_t thresholds = {
    .temp_high = TEMP_HIGH_THRESHOLD,
    .temp_low = TEMP_LOW_THRESHOLD,
    .humi_high = HUMI_HIGH_THRESHOLD,
    .humi_low = HUMI_LOW_THRESHOLD,
    .light_low = LIGHT_LOW_THRESHOLD,
    .smoke_high = SMOKE_HIGH_THRESHOLD
};

// 蓝牙状态实例
BluetoothState_t bt_state = {0};

// LCD提示实例
LCD_Notification_t lcd_notification = {0};

// 蓝牙命令缓冲区
char bt_command_buffer[BT_CMD_BUFFER_SIZE] = {0};
uint8_t bt_command_ready = 0;

/* =================== 函数声明 =================== */
void System_Init(void);
void Sensors_Init(void);
void Data_Collection(void);
void Alarm_Check(void);
void Key_Handler(void);
void Display_Update(void);
void Bluetooth_Handler(void);                    // 蓝牙命令处理
void Bluetooth_ParseCommand(char* command);      // 参数化命令解析
void LCD_ShowNotification(char* message, uint32_t duration);  // 显示LCD提示
void LCD_UpdateNotification(void);               // 更新LCD提示状态

/* =================== 系统时钟相关 =================== */
// 非阻塞延时函数 - 修复版，避免死循环
void delay_ms_non_blocking(uint32_t ms)
{
    uint32_t start_time = system_tick;
    uint32_t timeout = ms + 1000;  // 添加超时保护，防止死循环
    
    while((system_tick - start_time) < ms)
    {
        // 非阻塞等待，添加超时保护
        if((system_tick - start_time) > timeout)
        {
            break;  // 超时退出，防止死循环
        }
        __NOP();  // 空操作，让CPU处理其他任务
    }
}

/* =================== 第1步：系统和传感器初始化 =================== */
void System_Init(void)
{
    // 基础系统初始化
    SystemInit();
    
    // 首先确保LCD能工作
    lcd_init();
    lcd_clear();
    lcd_print_str(0, 0, "Smart Agriculture");
    lcd_print_str(1, 0, "Starting...");
    
    // 配置SysTick定时器，1ms中断一次 - 移到LCD后避免中断干扰初始化
    SysTick_Config(SystemCoreClock / 1000);
    NVIC_SetPriority(SysTick_IRQn, 0);
    
    // 基础硬件初始化
    Led_Init();
    lcd_print_str(1, 0, "LED OK");
    delay_ms_non_blocking(300);
    
    Key_Init();
    lcd_print_str(1, 0, "KEY OK");
    delay_ms_non_blocking(300);
    
    Beep_Init();
    lcd_print_str(1, 0, "BEEP OK");
    delay_ms_non_blocking(300);
    
    // ADC初始化 - 简化版，即使光敏电阻禁用也保留ADC基础功能
    Adc3_Init();
    lcd_print_str(1, 0, "ADC OK");
    delay_ms_non_blocking(300);

#if ENABLE_BLUETOOTH
    // 蓝牙初始化 - 简化版，减少阻塞
    lcd_print_str(1, 0, "BT Init...");
    UART2_Init(UART_BAUD_9600);
    delay_ms_non_blocking(200);  // 等待UART稳定
    
    Bluetooth_Init();
    bt_state.enabled = 1;
    lcd_print_str(1, 0, "BT OK");
    delay_ms_non_blocking(300);
#else
    lcd_print_str(1, 0, "BT Disabled");
    bt_state.enabled = 0;
    delay_ms_non_blocking(300);
#endif
    
    lcd_print_str(1, 0, "System Ready!");
    delay_ms_non_blocking(500);
    
#if ENABLE_BLUETOOTH
    // 发送蓝牙欢迎信息和命令格式说明
    delay_ms_non_blocking(200);  // 确保UART完全稳定
    Bluetooth_SendString("=== STM32 Smart Agriculture System ===\r\n");
    Bluetooth_SendString("DEBUG MODE: Only BT+LCD Enabled\r\n");
    Bluetooth_SendString("Command Format: Two Digits (00-09)\r\n");
    Bluetooth_SendString("Test Commands: 08-DisableAlarm, 09-QueryStatus\r\n");
    Bluetooth_SendString("Ready for Two-Digit Commands!\r\n");
#endif
}

void Sensors_Init(void)
{
    char str[50];
    
    // 显示传感器初始化开始
    lcd_print_str(1, 0, "Sensors Init...");
    delay_ms_non_blocking(300);
    
    // 在调试模式下，跳过所有传感器初始化，只设置默认值
    lcd_print_str(1, 0, "DEBUG: Skip Sensors");
    delay_ms_non_blocking(500);
    
    // 设置所有传感器的默认值和状态
    sensor_data.dht11_status = 0;
    sensor_data.temperature = 25;  // 默认温度
    sensor_data.humidity = 60;     // 默认湿度
    
    sensor_data.light_raw_value = 2048;
    sensor_data.light_percent = 50;
    
    sensor_data.smoke_ppm_value = 45;
    sensor_data.smoke_percent = 4.5f;
    
    sensor_data.mpu_status = 0;
    sensor_data.mpu_data.accel_x = 0.0f;
    sensor_data.mpu_data.accel_y = 0.0f;
    sensor_data.mpu_data.accel_z = 1.0f;
    sensor_data.mpu_data.gyro_x = 0.0f;
    sensor_data.mpu_data.gyro_y = 0.0f;
    sensor_data.mpu_data.gyro_z = 0.0f;
    sensor_data.mpu_data.temp = 25.0f;
    
    // 初始化完成
    lcd_print_str(1, 0, "Sensors Ready!");
    delay_ms_non_blocking(1000);
    
    // 清屏准备进入主循环显示
    lcd_clear();
}

/* =================== 第2步：数据采集 =================== */
void Data_Collection(void)
{
    static uint32_t last_read = 0;
    
    // 在调试模式下，跳过所有传感器数据采集，只更新计数
    if(system_tick - last_read >= 1000)
    {
        last_read = system_tick;
        
        // 保持默认值不变，只更新数据更新计数
        sensor_data.data_update_count++;
        
        // 模拟一些数据变化用于调试显示
        static uint8_t sim_counter = 0;
        sim_counter++;
        
        // 模拟温度在24-26度之间变化
        sensor_data.temperature = 24 + (sim_counter % 3);
        // 模拟湿度在58-62%之间变化  
        sensor_data.humidity = 58 + (sim_counter % 5);
        // 模拟光照在45-55%之间变化
        sensor_data.light_percent = 45 + (sim_counter % 11);
        // 模拟烟雾值在40-50ppm之间变化
        sensor_data.smoke_ppm_value = 40 + (sim_counter % 11);
        sensor_data.smoke_percent = (float)sensor_data.smoke_ppm_value / 10.0f;
    }
}

/* =================== 第3步：报警检查 =================== */
void Alarm_Check(void)
{
    // 如果报警被禁用，直接返回
    if(alarm_disabled)
    {
        // 清除所有报警状态
        alarm_status.temp_high_alarm = 0;
        alarm_status.temp_low_alarm = 0;
        alarm_status.humi_high_alarm = 0;
        alarm_status.humi_low_alarm = 0;
        alarm_status.light_low_alarm = 0;
        alarm_status.smoke_high_alarm = 0;
        alarm_status.any_alarm = 0;
        
        // 确保蜂鸣器关闭
        Beep_Off();
        return;
    }
    
    // 温度报警检查 - 使用动态阈值
    alarm_status.temp_high_alarm = (sensor_data.temperature > thresholds.temp_high);
    alarm_status.temp_low_alarm = (sensor_data.temperature < thresholds.temp_low);
    
    // 湿度报警检查 - 使用动态阈值
    alarm_status.humi_high_alarm = (sensor_data.humidity > thresholds.humi_high);
    alarm_status.humi_low_alarm = (sensor_data.humidity < thresholds.humi_low);
    
    // 光照报警检查 - 使用动态阈值
    alarm_status.light_low_alarm = (sensor_data.light_percent < thresholds.light_low);
    
    // 烟雾报警检查 - 使用动态阈值
    alarm_status.smoke_high_alarm = (sensor_data.smoke_ppm_value > thresholds.smoke_high);
    
    // 强制测试报警（调试用）- 可以通过修改这里来测试蜂鸣器
    // alarm_status.temp_high_alarm = 1;  // 取消注释以强制触发报警测试蜂鸣器
    
    // 综合报警状态
    alarm_status.any_alarm = alarm_status.temp_high_alarm || alarm_status.temp_low_alarm ||
                            alarm_status.humi_high_alarm || alarm_status.humi_low_alarm ||
                            alarm_status.light_low_alarm || alarm_status.smoke_high_alarm;
    
#if ENABLE_ALARM
    // LED指示灯控制
    // LED0 - 烟雾报警
    if(alarm_status.smoke_high_alarm) {
        Led_On(LED0);
    } else {
        Led_Off(LED0);
    }
    
    // LED1 - 温度报警
    if(alarm_status.temp_high_alarm || alarm_status.temp_low_alarm) {
        Led_On(LED1);
    } else {
        Led_Off(LED1);
    }
    
#if ENABLE_BREATHING
    // LED2 - 湿度报警 (呼吸灯)
    if(alarm_status.humi_high_alarm || alarm_status.humi_low_alarm) {
        Led_BreathingEffect(LED2);
    } else {
        Led_Off(LED2);
    }
    
    // LED3 - 光照报警 (呼吸灯)
    if(alarm_status.light_low_alarm) {
        Led_BreathingEffect(LED3);
    } else {
        Led_Off(LED3);
    }
#else
    // 普通LED模式
    if(alarm_status.humi_high_alarm || alarm_status.humi_low_alarm) {
        Led_On(LED2);
    } else {
        Led_Off(LED2);
    }
    
    if(alarm_status.light_low_alarm) {
        Led_On(LED3);
    } else {
        Led_Off(LED3);
    }
#endif
    
    // 蜂鸣器报警 - 简化版：报警时直接开启，无报警时关闭
    if(alarm_status.any_alarm)
    {
        Beep_On();   // 报警时蜂鸣器一直响
    }
    else
    {
        Beep_Off();  // 无报警时关闭蜂鸣器
    }
#endif // ENABLE_ALARM
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
    
    // 检测KEY3 - 蓝牙状态页面 (新增)
    if(Key_Debounce(KEY3_GPIO, KEY3_PIN))
    {
        if(!key_pressed[3])
        {
            current_page = PAGE_BLUETOOTH;
            last_key_time = system_tick;
            key_pressed[3] = 1;
        }
    }
    else
    {
        key_pressed[3] = 0;
    }
    
    // 按住KEY2+KEY3同时按下进入系统信息页面
    static uint8_t combo_pressed = 0;
    if(Key_Debounce(KEY2_GPIO, KEY2_PIN) && Key_Debounce(KEY3_GPIO, KEY3_PIN))
    {
        if(!combo_pressed)
        {
            current_page = PAGE_SYSTEM_INFO;
            last_key_time = system_tick;
            combo_pressed = 1;
        }
    }
    else
    {
        combo_pressed = 0;
    }
}

/* =================== 第5步：显示更新 =================== */
void Display_Update(void)
{
    char str[50];
    
    // 优先处理LCD通知
    if(lcd_notification.active)
    {
        LCD_UpdateNotification();
        return;  // 通知期间不显示其他内容
    }
    
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
            sprintf(str, "T:%dC H:%d%% %s", 
                    sensor_data.temperature, 
                    sensor_data.humidity,
                    alarm_status.temp_high_alarm ? "T-HIGH" : 
                    (alarm_status.temp_low_alarm ? "T-LOW" : 
                    (alarm_status.humi_high_alarm ? "H-HIGH" : 
                    (alarm_status.humi_low_alarm ? "H-LOW" : "OK"))));
            lcd_print_str(1, 0, str);
            break;
            
        case PAGE_LIGHT_SMOKE:
            lcd_print_str(0, 0, "=== Light/Smoke ===");
            sprintf(str, "L:%d%% MQ2:%dppm%s", 
                    sensor_data.light_percent, 
                    sensor_data.smoke_ppm_value,
                    (sensor_data.smoke_ppm_value == 0) ? "!" : 
                    (alarm_status.smoke_high_alarm ? "H" : ""));
            lcd_print_str(1, 0, str);
            break;
            
        case PAGE_ATTITUDE:
            lcd_print_str(0, 0, "=== MPU6050 ===");
            #if ENABLE_MPU6050
            // 如果MPU6050启用且状态正常，显示角度信息
            if(sensor_data.mpu_status)
            {
                // 使用新的角度显示功能
                MPU6050_Read_And_Display();
            }
            else
            {
                sprintf(str, "MPU6050 Offline");
                lcd_print_str(1, 0, str);
            }
            #else
            // MPU6050被禁用时显示默认信息
            sprintf(str, "X:%.1f Y:%.1f Z:%.1f", 
                    sensor_data.mpu_data.accel_x,
                    sensor_data.mpu_data.accel_y,
                    sensor_data.mpu_data.accel_z);
            lcd_print_str(1, 0, str);
            #endif
            break;
            
        case PAGE_BLUETOOTH:
            lcd_print_str(0, 0, "=== Bluetooth ===");
            sprintf(str, "BT:%s Cmd:%ld %s", 
                    bt_state.enabled ? "ON" : "OFF",
                    bt_state.command_count,
                    alarm_disabled ? "DISABLED" : "ENABLED");
            lcd_print_str(1, 0, str);
            break;
            
        case PAGE_SYSTEM_INFO:
            lcd_print_str(0, 0, "=== System Info ===");
            // 添加MQ2调试信息显示
            static uint8_t debug_mode = 0;
            static uint32_t last_switch = 0;
            
            // 每3秒切换显示内容
            if(system_tick - last_switch >= 3000)
            {
                last_switch = system_tick;
                debug_mode = (debug_mode + 1) % 3;
            }
            
            switch(debug_mode)
            {
                case 0: // 显示运行时间和错误
                    sprintf(str, "Time:%lds Err:%ld", 
                            system_tick / 1000, sensor_data.error_count);
                    break;
                case 1: // 显示MQ2状态
                    sprintf(str, "MQ2:%dppm Ready:%d", 
                            sensor_data.smoke_ppm_value, MQ2_IsDataReady());
                    break;
                case 2: // 显示传感器状态
                    sprintf(str, "DHT:%d L:%d MQ:%d", 
                            sensor_data.dht11_status,
                            (sensor_data.light_raw_value > 0 ? 1 : 0),
                            (sensor_data.smoke_ppm_value > 0 ? 1 : 0));
                    break;
            }
            lcd_print_str(1, 0, str);
            break;
    }
}

/* =================== 蓝牙处理函数 =================== */

/**
 * @brief 显示LCD通知提示
 * @param message: 提示消息
 * @param duration: 持续时间(ms)
 */
void LCD_ShowNotification(char* message, uint32_t duration)
{
    lcd_notification.active = 1;
    lcd_notification.start_time = system_tick;
    lcd_notification.duration = duration;
    strncpy(lcd_notification.message, message, sizeof(lcd_notification.message)-1);
    lcd_notification.message[sizeof(lcd_notification.message)-1] = '\0';
}

/**
 * @brief 更新LCD通知状态
 */
void LCD_UpdateNotification(void)
{
    if(lcd_notification.active)
    {
        // 检查是否超时
        if(system_tick - lcd_notification.start_time >= lcd_notification.duration)
        {
            lcd_notification.active = 0;
        }
        else
        {
            // 在屏幕顶部显示通知
            lcd_print_str(0, 0, lcd_notification.message);
        }
    }
}

/**
 * @brief 解析蓝牙两位数字命令（调试简化版）
 * @param command: 命令字符串，格式如"01"表示设置温度高阈值
 */
void Bluetooth_ParseCommand(char* command)
{
    char response[100];
    int cmd_num;
    
    Bluetooth_SendString("PARSE: Starting command analysis...\r\n");
    
    // 验证命令长度
    if(strlen(command) < 2)
    {
        Bluetooth_SendString("ERROR: Command too short (need 2 digits)\r\n");
        return;
    }
    
    // 验证命令是否都是数字
    if(command[0] < '0' || command[0] > '9' || command[1] < '0' || command[1] > '9')
    {
        sprintf(response, "ERROR: Invalid chars: %c%c (need digits)\r\n", command[0], command[1]);
        Bluetooth_SendString(response);
        return;
    }
    
    // 解析两位数命令
    cmd_num = (command[0] - '0') * 10 + (command[1] - '0');
    
    sprintf(response, "PARSE: Command %02d recognized\r\n", cmd_num);
    Bluetooth_SendString(response);
    
    // 根据命令执行对应操作 - 简化版本，只保留关键命令
    switch(cmd_num)
    {
        case 1: // 01 - 设置温度高阈值为35℃
            thresholds.temp_high = 35;
            Bluetooth_SendString("SUCCESS: Temp High = 35C\r\n");
            LCD_ShowNotification("TempHigh: 35C", 2000);
            break;
            
        case 2: // 02 - 设置温度低阈值为15℃
            thresholds.temp_low = 15;
            Bluetooth_SendString("SUCCESS: Temp Low = 15C\r\n");
            LCD_ShowNotification("TempLow: 15C", 2000);
            break;
            
        case 8: // 08 - 禁用所有报警
            alarm_disabled = 1;
            Bluetooth_SendString("SUCCESS: All Alarms DISABLED\r\n");
            LCD_ShowNotification("Alarms DISABLED", 2000);
            break;
            
        case 9: // 09 - 查询当前状态（调试增强版）
            sprintf(response, "Status: T=%dC H=%d%% DEBUG_MODE\r\n",
                    sensor_data.temperature, sensor_data.humidity);
            Bluetooth_SendString(response);
            sprintf(response, "Thresholds: TH=%d TL=%d AlarmOff=%d\r\n",
                    thresholds.temp_high, thresholds.temp_low, alarm_disabled);
            Bluetooth_SendString(response);
            sprintf(response, "System: Tick=%ld Updates=%ld\r\n",
                    system_tick, sensor_data.data_update_count);
            Bluetooth_SendString(response);
            return;
            
        case 0: // 00 - 恢复默认阈值
            alarm_disabled = 0;
            thresholds.temp_high = TEMP_HIGH_THRESHOLD;
            thresholds.temp_low = TEMP_LOW_THRESHOLD;
            thresholds.humi_high = HUMI_HIGH_THRESHOLD;
            thresholds.humi_low = HUMI_LOW_THRESHOLD;
            thresholds.light_low = LIGHT_LOW_THRESHOLD;
            thresholds.smoke_high = SMOKE_HIGH_THRESHOLD;
            Bluetooth_SendString("SUCCESS: Reset to defaults\r\n");
            LCD_ShowNotification("Reset & ENABLED", 2000);
            break;
            
        default:
            sprintf(response, "ERROR: Unknown command %02d\r\n", cmd_num);
            Bluetooth_SendString(response);
            Bluetooth_SendString("DEBUG: Valid commands: 00,01,02,08,09\r\n");
            return;
    }
    
    // 更新蓝牙状态
    bt_state.command_count++;
    bt_state.last_command = system_tick;
    
    Bluetooth_SendString("PARSE: Command completed successfully!\r\n");
}


/**
 * @brief 蓝牙命令处理主函数（调试增强版）
 */
void Bluetooth_Handler(void)
{
#if ENABLE_BLUETOOTH
    static uint32_t last_bluetooth_check = 0;
    static uint32_t bt_check_counter = 0;
    
    // 每10ms检查一次蓝牙命令（提高响应速度）
    if(system_tick - last_bluetooth_check >= 10)
    {
        last_bluetooth_check = system_tick;
        bt_check_counter++;
        
        // 每500次检查输出一次调试信息（约5秒）
        if(bt_check_counter % 500 == 0)
        {
            char debug_info[60];
            sprintf(debug_info, "BT: Checking %ld, Ready=%d\r\n", bt_check_counter, bt_command_ready);
            Bluetooth_SendString(debug_info);
        }
        
        // 处理蓝牙命令
        if(bt_command_ready)
        {
            // 立即确认收到命令
            Bluetooth_SendString("BT: Command received!\r\n");
            
            // 显示收到的原始命令
            Bluetooth_SendString("BT: Raw command=[");
            Bluetooth_SendString(bt_command_buffer);
            Bluetooth_SendString("]\r\n");
            
            // 处理完整命令
            Bluetooth_SendString("BT: Parsing command...\r\n");
            Bluetooth_ParseCommand(bt_command_buffer);
            
            // 清空缓冲区
            memset(bt_command_buffer, 0, BT_CMD_BUFFER_SIZE);
            bt_command_ready = 0;
            
            // 发送处理完成确认
            Bluetooth_SendString("BT: Command processing completed!\r\n");
        }
    }
#endif
}

/* =================== 主函数 =================== */
int main(void)
{
    // 系统初始化
    System_Init();
    
    // 传感器初始化
    Sensors_Init();
    
#if ENABLE_BLUETOOTH
    // 发送主循环启动确认
    Bluetooth_SendString("MAIN: Entering main loop...\r\n");
#endif
    
    // 主循环 - 专注于LCD和蓝牙调试
    uint32_t loop_counter = 0;
    uint32_t last_debug_output = 0;
    
    while(1)
    {
        loop_counter++;
        
        // 每5秒输出一次主循环运行状态
        if(system_tick - last_debug_output >= 5000)
        {
            last_debug_output = system_tick;
#if ENABLE_BLUETOOTH
            char debug_msg[50];
            sprintf(debug_msg, "MAIN: Loop %ld, Tick %ld\r\n", loop_counter, system_tick);
            Bluetooth_SendString(debug_msg);
#endif
        }
        
        // 系统运行指示 - LED每2秒闪烁一次（降低频率）
        static uint32_t last_led_toggle = 0;
        if(system_tick - last_led_toggle >= 2000)
        {
            last_led_toggle = system_tick;
            Led_Toggle(LED0);
        }
        
        // 数据采集 - 简化版，主要用于LCD显示
        Data_Collection();
        
        // 报警检查 - 在调试模式下禁用
#if ENABLE_ALARM
        Alarm_Check();
#endif
        
        // 按键处理
        Key_Handler();
        
        // 蓝牙命令处理 - 重点调试对象
        Bluetooth_Handler();
        
        // 显示更新 - 重点调试对象
        Display_Update();
        
        // 主循环无阻塞延时，让系统快速响应
        delay_ms_non_blocking(10);  // 添加小延时，避免过度占用CPU
    }
}
