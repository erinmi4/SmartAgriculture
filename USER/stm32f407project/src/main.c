/* =================== 调试开关 =================== */
// 如果出现卡死问题，可以逐个禁用模块进行排查
#define ENABLE_DHT11       1    // 1-启用DHT11, 0-禁用DHT11 (先禁用排查)
#define ENABLE_MPU6050     0    // 1-启用MPU6050, 0-禁用MPU6050 (先禁用排查)
#define ENABLE_MQ2         1    // 1-启用MQ2, 0-禁用MQ2  
#define ENABLE_LIGHT       1    // 1-启用光敏电阻, 0-禁用光敏电阻
#define ENABLE_BLUETOOTH   1    // 1-启用蓝牙, 0-禁用蓝牙
#define ENABLE_BREATHING   1    // 1-启用呼吸灯, 0-禁用呼吸灯
#define ENABLE_ALARM       1    // 1-启用报警, 0-禁用报警

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
// 参数化命令格式: "CMD:VALUE"，例如 "TH:30" 设置温度高阈值为30
// 支持的命令:
// TH:xx - 设置温度高阈值
// TL:xx - 设置温度低阈值  
// HH:xx - 设置湿度高阈值
// HL:xx - 设置湿度低阈值
// LL:xx - 设置光照低阈值
// SH:xx - 设置烟雾高阈值

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
uint32_t system_tick = 0;

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

/* =================== 第1步：系统和传感器初始化 =================== */
void System_Init(void)
{
    // 系统时钟初始化
    SystemInit();
    
    // 延时初始化 - 删除不存在的delay_init
    // delay_init(168);  // 168MHz系统时钟
    
    // LCD初始化并显示欢迎信息
    lcd_init();
    lcd_clear();
    lcd_print_str(0, 0, "Smart Agriculture");
    lcd_print_str(1, 0, "Initializing...");
    Mdelay_Lib(1000);
    
    // LED初始化
    Led_Init();
    
#if ENABLE_BREATHING
    // 呼吸灯初始化
    lcd_print_str(1, 0, "Breathing LED...");
    Led_BreathingInit();
    Mdelay_Lib(500);
#endif
    
    // 按键初始化
    Key_Init();
    
    // 蜂鸣器初始化
    Beep_Init();
    
    // ADC初始化
    Adc3_Init();
    
#if ENABLE_BLUETOOTH
    // 蓝牙初始化
    lcd_print_str(1, 0, "Bluetooth...");
    UART2_Init(UART_BAUD_9600);  // 修复：使用正确的波特率枚举类型
    Bluetooth_Init();
    bt_state.enabled = 1;
    
    // 发送启动确认消息（参考main_bluetooth.c）
    Mdelay_Lib(100);  // 等待蓝牙模块稳定
    Bluetooth_SendString("Smart Agriculture v2.0\r\n");
    Bluetooth_SendString("Bluetooth Ready!\r\n");
    Bluetooth_SendString("Commands: TH:30 TL:20 HH:70 HL:40 LL:40 SH:150 ST:0\r\n");
    
    Mdelay_Lib(500);
#else
    lcd_print_str(1, 0, "BT Disabled");
    bt_state.enabled = 0;
    Mdelay_Lib(500);
#endif
    
    lcd_print_str(1, 0, "System OK");
    Mdelay_Lib(500);
}

void Sensors_Init(void)
{
    char str[50];
    
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

#if ENABLE_MQ2
    // MQ-2烟雾传感器初始化
    lcd_print_str(1, 0, "MQ2 Sensor...");
    MQ2_Init();
    Mdelay_Lib(1000);  // MQ2需要更长的初始化时间
    
    // 测试MQ2通信
    lcd_print_str(1, 0, "Testing MQ2...");
    MQ2_ClearFlag();
    MQ2_SendCommand();
    Mdelay_Lib(500);  // 等待响应
    
    if(MQ2_IsDataReady())
    {
        uint16_t test_ppm = MQ2_GetValue();
        sprintf(str, "MQ2 OK: %dppm", test_ppm);
        lcd_print_str(1, 0, str);
    }
    else
    {
        lcd_print_str(1, 0, "MQ2 No Response");
        sensor_data.error_count++;
    }
    Mdelay_Lib(1000);
#endif

#if ENABLE_MPU6050
    // MPU6050初始化
    lcd_print_str(1, 0, "MPU6050...");
    if(MPU6050_Init() == 0)
    {
        sensor_data.mpu_status = 1;
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
        // 光照数据采集 - 修复光敏电阻读取问题
        sensor_data.light_raw_value = Light_GetRawValue();
        if(sensor_data.light_raw_value > 4095) sensor_data.light_raw_value = 4095;
        
        // 使用专用函数获取百分比值
        sensor_data.light_percent = Light_GetValue();
        
        // 调试信息：如果光照值异常，记录错误
        if(sensor_data.light_raw_value == 0 && sensor_data.light_percent == 0) {
            sensor_data.error_count++;
        }
#else
        sensor_data.light_raw_value = 2048;
        sensor_data.light_percent = 50;
#endif

#if ENABLE_MQ2
        // MQ2烟雾数据采集 - 使用MQ-2专用接口，增加调试信息
        static uint32_t mq2_timeout_count = 0;
        
        MQ2_ClearFlag();           // 清除数据标志
        MQ2_SendCommand();         // 发送读取命令
        
        // 等待数据就绪，但不要无限等待
        uint32_t wait_start = system_tick;
        while(!MQ2_IsDataReady() && (system_tick - wait_start) < 200)
        {
            // 最多等待200ms
            Mdelay_Lib(1);
        }
        
        if(MQ2_IsDataReady())      // 检查数据是否就绪
        {
            sensor_data.smoke_ppm_value = MQ2_GetValue();  // 获取ppm数值
            // 将ppm转换为百分比用于报警判断 (假设1000ppm为100%)
            sensor_data.smoke_percent = (float)sensor_data.smoke_ppm_value / 10.0f;
            mq2_timeout_count = 0;  // 重置超时计数
        }
        else
        {
            // 数据未就绪，超时
            mq2_timeout_count++;
            sensor_data.error_count++;
            // 保持之前的数值，不设为0
        }
#else
        sensor_data.smoke_ppm_value = 45;   // 默认ppm值
        sensor_data.smoke_percent = 4.5f;   // 对应的百分比
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
            sprintf(str, "X:%.1f Y:%.1f Z:%.1f", 
                    sensor_data.mpu_data.accel_x,
                    sensor_data.mpu_data.accel_y,
                    sensor_data.mpu_data.accel_z);
            lcd_print_str(1, 0, str);
            break;
            
        case PAGE_BLUETOOTH:
            lcd_print_str(0, 0, "=== Bluetooth ===");
            sprintf(str, "BT:%s Cmd:%ld", 
                    bt_state.enabled ? "ON" : "OFF",
                    bt_state.command_count);
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
 * @brief 解析蓝牙参数化命令（参考main_bluetooth.c - 简洁版）
 * @param command: 命令字符串，格式如"TH:30"
 */
void Bluetooth_ParseCommand(char* command)
{
    char response[80];
    
    // 基本格式检查
    if(strlen(command) < 4 || command[2] != ':')
    {
        Bluetooth_SendString("ERROR: Use format CMD:VALUE\r\n");
        Bluetooth_SendString("Example: TH:30 TL:20 HH:70 HL:40 LL:40 SH:150\r\n");
        return;
    }
    
    // 提取命令和数值
    char cmd[3] = {command[0], command[1], '\0'};
    int value = atoi(&command[3]);
    
    // 发送解析确认
    sprintf(response, "PARSING: %s=%d\r\n", cmd, value);
    Bluetooth_SendString(response);
    
    // 根据命令设置阈值
    if(strcmp(cmd, "TH") == 0 && value >= 0 && value <= 60)
    {
        thresholds.temp_high = value;
        sprintf(response, "SUCCESS: TempHigh=%d\r\n", value);
        LCD_ShowNotification("TempHigh Set", 2000);
    }
    else if(strcmp(cmd, "TL") == 0 && value >= 0 && value <= 60)
    {
        thresholds.temp_low = value;
        sprintf(response, "SUCCESS: TempLow=%d\r\n", value);
        LCD_ShowNotification("TempLow Set", 2000);
    }
    else if(strcmp(cmd, "HH") == 0 && value >= 0 && value <= 100)
    {
        thresholds.humi_high = value;
        sprintf(response, "SUCCESS: HumiHigh=%d\r\n", value);
        LCD_ShowNotification("HumiHigh Set", 2000);
    }
    else if(strcmp(cmd, "HL") == 0 && value >= 0 && value <= 100)
    {
        thresholds.humi_low = value;
        sprintf(response, "SUCCESS: HumiLow=%d\r\n", value);
        LCD_ShowNotification("HumiLow Set", 2000);
    }
    else if(strcmp(cmd, "LL") == 0 && value >= 0 && value <= 100)
    {
        thresholds.light_low = value;
        sprintf(response, "SUCCESS: LightLow=%d\r\n", value);
        LCD_ShowNotification("LightLow Set", 2000);
    }
    else if(strcmp(cmd, "SH") == 0 && value >= 0 && value <= 1000)
    {
        thresholds.smoke_high = value;
        sprintf(response, "SUCCESS: SmokeHigh=%d\r\n", value);
        LCD_ShowNotification("SmokeHigh Set", 2000);
    }
    else if(strcmp(cmd, "ST") == 0)  // 状态查询
    {
        sprintf(response, "STATUS: T=%dC H=%d%% L=%d%% S=%dppm Alarms=%s\r\n",
                sensor_data.temperature, sensor_data.humidity, 
                sensor_data.light_percent, sensor_data.smoke_ppm_value,
                alarm_status.any_alarm ? "YES" : "NO");
        Bluetooth_SendString(response);
        return;
    }
    else
    {
        sprintf(response, "ERROR: Invalid command or range\r\n");
        Bluetooth_SendString(response);
        Bluetooth_SendString("Valid: TH:0-60 TL:0-60 HH:0-100 HL:0-100 LL:0-100 SH:0-1000 ST:0\r\n");
        return;
    }
    
    // 发送成功确认
    Bluetooth_SendString(response);
    
    // 更新蓝牙状态
    bt_state.command_count++;
    bt_state.last_command = system_tick;
    
    // 最终确认
    Bluetooth_SendString("COMPLETE\r\n");
}

/**
 * @brief 蓝牙命令处理主函数（参考main_bluetooth.c实现）
 */
void Bluetooth_Handler(void)
{
#if ENABLE_BLUETOOTH
    static uint32_t last_bluetooth_check = 0;
    static uint32_t last_heartbeat = 0;
    
    // 每100ms检查一次蓝牙命令（参考main_bluetooth.c）
    if(system_tick - last_bluetooth_check >= 100)
    {
        last_bluetooth_check = system_tick;
        
        // 每10秒发送一次心跳包，确认蓝牙连接正常
        if(system_tick - last_heartbeat >= 10000)
        {
            char heartbeat[60];
            sprintf(heartbeat, "HEARTBEAT: T=%dC H=%d%% S=%dppm\r\n", 
                    sensor_data.temperature, 
                    sensor_data.humidity,
                    sensor_data.smoke_ppm_value);
            Bluetooth_SendString(heartbeat);
            last_heartbeat = system_tick;
        }
        
        // 处理蓝牙命令
        if(bt_command_ready)
        {
            // 立即确认收到命令
            Bluetooth_SendString("RECEIVED\r\n");
            
            // 处理命令
            Bluetooth_ParseCommand(bt_command_buffer);
            
            // 清空缓冲区
            memset(bt_command_buffer, 0, BT_CMD_BUFFER_SIZE);
            bt_command_ready = 0;
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
    
    // 主循环
    while(1)
    {
        // LED闪烁指示系统运行
        if(system_tick % 1000 == 0)
        {
            Led_Toggle(LED0);
        }
        
        // 数据采集
        Data_Collection();
        
        // 报警检查
        Alarm_Check();
        
        // 按键处理
        Key_Handler();
        
        // 蓝牙命令处理
        Bluetooth_Handler();
        
        // 显示更新
        Display_Update();
        
        // 主循环延时
        Mdelay_Lib(10);  // 10ms延时，降低CPU占用
        
        // 更新系统时钟
        system_tick += 10;
    }
}
