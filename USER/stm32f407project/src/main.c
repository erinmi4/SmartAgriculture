/* =================== 调试开关 =================== */
// 如果出现卡死问题，可以逐个禁用模块进行排查
#define ENABLE_DHT11     1    // 1-启用DHT11, 0-禁用DHT11 (先禁用排查)
#define ENABLE_MPU6050   1    // 1-启用MPU6050, 0-禁用MPU6050 (先禁用排查)
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
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* =================== 缺失函数声明 =================== */
// 注：实际的函数名在对应的头文件中定义
// delay_init 不存在，可以删除或创建空函数
// beep_init -> Beep_Init
// beep_on -> Beep_On  
// beep_off -> Beep_Off
// LED0_Toggle -> Led_Toggle

/* =================== 系统定义 =================== */
// 阈值宏定义（可在编译时修改）
#define DEFAULT_TEMP_HIGH_THRESHOLD  29    // 温度高报警阈值(℃)  
#define DEFAULT_TEMP_LOW_THRESHOLD   20    // 温度低报警阈值(℃)
#define DEFAULT_HUMI_HIGH_THRESHOLD  70    // 湿度高报警阈值(%)
#define DEFAULT_HUMI_LOW_THRESHOLD   40    // 湿度低报警阈值(%)
#define DEFAULT_LIGHT_LOW_THRESHOLD  40    // 光照低报警阈值(0-100)
#define DEFAULT_SMOKE_HIGH_THRESHOLD 120   // 烟雾高报警阈值(ppm) - 方便演示取120

// 运行时可修改的阈值变量（初始化为宏定义值）
uint8_t TEMP_HIGH_THRESHOLD = DEFAULT_TEMP_HIGH_THRESHOLD;
uint8_t TEMP_LOW_THRESHOLD = DEFAULT_TEMP_LOW_THRESHOLD;
uint8_t HUMI_HIGH_THRESHOLD = DEFAULT_HUMI_HIGH_THRESHOLD;
uint8_t HUMI_LOW_THRESHOLD = DEFAULT_HUMI_LOW_THRESHOLD;
uint8_t LIGHT_LOW_THRESHOLD = DEFAULT_LIGHT_LOW_THRESHOLD;
uint16_t SMOKE_HIGH_THRESHOLD = DEFAULT_SMOKE_HIGH_THRESHOLD;

// 蓝牙指令定义
#define BT_CMD_SHOW_STATUS      "00"    // 显示所有状态
#define BT_CMD_SET_TEMP_HIGH    "01"    // 设置温度高阈值
#define BT_CMD_SET_TEMP_LOW     "02"    // 设置温度低阈值
#define BT_CMD_SET_HUMI_HIGH    "03"    // 设置湿度高阈值
#define BT_CMD_SET_HUMI_LOW     "04"    // 设置湿度低阈值
#define BT_CMD_SET_LIGHT_LOW    "05"    // 设置光照低阈值
#define BT_CMD_SET_SMOKE_HIGH   "06"    // 设置烟雾高阈值

// MPU6050数据结构定义
typedef struct {
    float accel_x;      // X轴加速度
    float accel_y;      // Y轴加速度
    float accel_z;      // Z轴加速度
    float gyro_x;       // X轴角速度
    float gyro_y;       // Y轴角速度
    float gyro_z;       // Z轴角速度
    float temp;         // 温度
} MPU6050_Data_t;

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

// 蓝牙数据处理结构
typedef struct {
    char rx_buffer[64];     // 接收缓冲区
    uint8_t rx_index;       // 接收索引
    uint8_t command_ready;  // 命令就绪标志
    char current_command[16]; // 当前处理的命令
} BluetoothData_t;

/* =================== 全局变量 =================== */
SensorData_t sensor_data = {0};
AlarmStatus_t alarm_status = {0};
BluetoothData_t bt_data = {0};
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
void Custom_Bluetooth_Init(void);
void Bluetooth_Handler(void);
// 兼容bluetooth.h的接口
void Bluetooth_ProcessCommand(void); // 由bluetooth.c实现
void Bluetooth_SendStatus(uint16_t mode); // 由bluetooth.c实现
// 用户自定义蓝牙指令处理
void UserBluetoothCommandHandler(const char* cmd, const char* value);
uint16_t UserBluetoothParseValue(const char* str);
void CustomBluetoothSendString(const char* str);
void ProcessReceivedCommand(const char* command);
void SendSystemStatus(void);
void MPU6050_GetData(MPU6050_Data_t *mpu_data);

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
    lcd_print_str(1, 0, "LED Init OK");
    
    // 按键初始化
    Key_Init();
    lcd_print_str(1, 0, "Key Init OK");
    
    // 蜂鸣器初始化
    Beep_Init();
    lcd_print_str(1, 0, "Beep Init OK");
    
    // ADC初始化
    Adc3_Init();
    lcd_print_str(1, 0, "ADC Init OK");
    Mdelay_Lib(300);
    
    // UART2初始化（蓝牙需要）
    UART2_Init(UART_BAUD_9600);
    lcd_print_str(1, 0, "UART2 Init OK");
    Mdelay_Lib(300);
    
    // 蓝牙初始化
    lcd_print_str(1, 0, "Bluetooth Init...");
    Mdelay_Lib(100);
    
    Bluetooth_Init();
    lcd_print_str(1, 0, "Bluetooth Init OK");
    Mdelay_Lib(300);
    
    // 自定义蓝牙设置
    lcd_print_str(1, 0, "BT Config...");
    Custom_Bluetooth_Init();
    lcd_print_str(1, 0, "BT Config OK");
    Mdelay_Lib(300);
    
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
    
    // 测试光敏电阻读取
    uint16_t test_light_raw = Light_GetRawValue();
    uint8_t test_light_percent = 100 - (test_light_raw * 100 / 4095);
    sprintf(str, "Light: %d (%d%%)", test_light_raw, test_light_percent);
    lcd_print_str(1, 0, str);
    Mdelay_Lib(1000);
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
    Mdelay_Lib(100); // 确保LCD显示刷新
    
    // 调试信息：开始初始化MPU6050
    sensor_data.error_count++; // 暂时增加错误计数，成功后减回来
    
    // 原始的MPU6050_Init函数无返回值，我们自己进行初始化并判断
    MPU6050_Init();
    Mdelay_Lib(200); // 给MPU6050一些时间稳定
    
    // 简单读取一些数据来验证MPU6050是否工作正常
    int16_t ax = 0, ay = 0, az = 0;
    MPU6050_Read_Accel(&ax, &ay, &az);
    
    char debug_str[30];
    sprintf(debug_str, "MPU:%d,%d,%d", ax, ay, az);
    lcd_print_str(1, 0, debug_str);
    Mdelay_Lib(1000); // 显示原始值用于调试
    
    // 如果加速度数据不全为0，则认为初始化成功
    if(ax != 0 || ay != 0 || az != 0)
    {
        sensor_data.mpu_status = 1;
        sensor_data.error_count--; // 成功，减回错误计数
        lcd_print_str(1, 0, "MPU6050 OK");
    }
    else
    {
        sensor_data.mpu_status = 0;
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
        uint16_t raw_light = Light_GetRawValue();
        // 如果读取到的值为0，尝试再读取一次
        if(raw_light == 0) {
            Mdelay_Lib(10);
            raw_light = Light_GetRawValue();
        }
        
        sensor_data.light_raw_value = raw_light;
        if(sensor_data.light_raw_value > 4095) sensor_data.light_raw_value = 4095;
        // 光照百分比：ADC值是反的，值越小表示光照越强
        sensor_data.light_percent = 100 - (sensor_data.light_raw_value * 100 / 4095);
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
    // 温度报警检查
    alarm_status.temp_high_alarm = (sensor_data.temperature > TEMP_HIGH_THRESHOLD);
    alarm_status.temp_low_alarm = (sensor_data.temperature < TEMP_LOW_THRESHOLD);
    
    // 湿度报警检查
    alarm_status.humi_high_alarm = (sensor_data.humidity > HUMI_HIGH_THRESHOLD);
    alarm_status.humi_low_alarm = (sensor_data.humidity < HUMI_LOW_THRESHOLD);
    
    // 光照报警检查
    alarm_status.light_low_alarm = (sensor_data.light_percent < LIGHT_LOW_THRESHOLD);
    
    // 烟雾报警检查 - 基于ppm数值
    alarm_status.smoke_high_alarm = (sensor_data.smoke_ppm_value > SMOKE_HIGH_THRESHOLD);
    
    // 综合报警状态
    alarm_status.any_alarm = alarm_status.temp_high_alarm || alarm_status.temp_low_alarm ||
                            alarm_status.humi_high_alarm || alarm_status.humi_low_alarm ||
                            alarm_status.light_low_alarm || alarm_status.smoke_high_alarm;
    
    // 蜂鸣器报警
    if(alarm_status.any_alarm)
    {
        Beep_On();
        Mdelay_Lib(100);
        Beep_Off();
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
        {
            // 计算Roll和Pitch角度
            float roll = 0, pitch = 0;
            // 计算姿态角（基于加速度计数据）
            if(sensor_data.mpu_status) {
                // atan2函数需要单精度float参数，计算角度
                roll = atan2f(sensor_data.mpu_data.accel_y, sensor_data.mpu_data.accel_z) * 57.29578f; // 180/π ≈ 57.29578
                pitch = atan2f(-sensor_data.mpu_data.accel_x, 
                             sqrtf(sensor_data.mpu_data.accel_y*sensor_data.mpu_data.accel_y + 
                                  sensor_data.mpu_data.accel_z*sensor_data.mpu_data.accel_z)) * 57.29578f;
            }
            
            // 标题显示MPU6050状态
            lcd_print_str(0, 0, "=== MPU6050 ===");
            
            // 循环显示不同的数据
            static uint8_t mpu_display_mode = 0;
            static uint32_t last_mpu_switch = 0;
            
            // 每2秒切换显示内容
            if(system_tick - last_mpu_switch >= 2000) {
                last_mpu_switch = system_tick;
                mpu_display_mode = (mpu_display_mode + 1) % 3;
            }
            
            // 根据显示模式选择不同的数据显示
            switch(mpu_display_mode) {
                case 0: // 显示加速度数据
                    sprintf(str, "A:%.1f %.1f %.1f", 
                            sensor_data.mpu_data.accel_x,
                            sensor_data.mpu_data.accel_y,
                            sensor_data.mpu_data.accel_z);
                    break;
                case 1: // 显示陀螺仪数据
                    sprintf(str, "G:%.1f %.1f %.1f",
                            sensor_data.mpu_data.gyro_x,
                            sensor_data.mpu_data.gyro_y,
                            sensor_data.mpu_data.gyro_z);
                    break;
                case 2: // 显示角度数据
                    sprintf(str, "R:%.1f P:%.1f", roll, pitch);
                    break;
            }
            
            lcd_print_str(1, 0, str);
            break;
        }
            
        case PAGE_SYSTEM_INFO:
        {
            lcd_print_str(0, 0, "=== System Info ===");
            // 添加MQ2调试信息显示
            static uint8_t debug_mode = 0;
            static uint32_t last_switch = 0;
            
            // 每3秒切换显示内容
            if(system_tick - last_switch >= 3000)
            {
                last_switch = system_tick;
                debug_mode = (debug_mode + 1) % 4;
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
                case 2: // 显示光照传感器状态
                    sprintf(str, "Light:%d (Raw:%d)", 
                            sensor_data.light_percent,
                            sensor_data.light_raw_value);
                    break;
                case 3: // 显示传感器状态
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
        
        // 蓝牙处理
        Bluetooth_Handler(); // 蓝牙接收和命令分发
        
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

/* =================== 用户自定义蓝牙指令处理 =================== */
// 该函数处理所有传感器阈值设置命令
void UserBluetoothCommandHandler(const char* cmd, const char* value)
{
    char resp[64];
    uint16_t v = UserBluetoothParseValue(value);
    
    if(strcmp(cmd, BT_CMD_SET_TEMP_HIGH) == 0) {
        if(v <= 60) { 
            TEMP_HIGH_THRESHOLD = v; 
            sprintf(resp, "OK: Temp High = %d°C\r\n", v); 
        }
        else sprintf(resp, "ERR: Range 0-60°C\r\n");
    } 
    else if(strcmp(cmd, BT_CMD_SET_TEMP_LOW) == 0) {
        if(v <= 60) { 
            TEMP_LOW_THRESHOLD = v; 
            sprintf(resp, "OK: Temp Low = %d°C\r\n", v); 
        }
        else sprintf(resp, "ERR: Range 0-60°C\r\n");
    } 
    else if(strcmp(cmd, BT_CMD_SET_HUMI_HIGH) == 0) {
        if(v <= 100) { 
            HUMI_HIGH_THRESHOLD = v; 
            sprintf(resp, "OK: Humidity High = %d%%\r\n", v); 
        }
        else sprintf(resp, "ERR: Range 0-100%%\r\n");
    } 
    else if(strcmp(cmd, BT_CMD_SET_HUMI_LOW) == 0) {
        if(v <= 100) { 
            HUMI_LOW_THRESHOLD = v; 
            sprintf(resp, "OK: Humidity Low = %d%%\r\n", v); 
        }
        else sprintf(resp, "ERR: Range 0-100%%\r\n");
    } 
    else if(strcmp(cmd, BT_CMD_SET_LIGHT_LOW) == 0) {
        if(v <= 100) { 
            LIGHT_LOW_THRESHOLD = v; 
            sprintf(resp, "OK: Light Low = %d%%\r\n", v); 
        }
        else sprintf(resp, "ERR: Range 0-100%%\r\n");
    } 
    else if(strcmp(cmd, BT_CMD_SET_SMOKE_HIGH) == 0) {
        if(v <= 1000) { 
            SMOKE_HIGH_THRESHOLD = v; 
            sprintf(resp, "OK: Smoke High = %dppm\r\n", v); 
        }
        else sprintf(resp, "ERR: Range 0-1000ppm\r\n");
    } 
    else {
        sprintf(resp, "ERR: Unknown command '%s'\r\n", cmd);
    }
    
    Bluetooth_SendString(resp);
}

uint16_t UserBluetoothParseValue(const char* str)
{
    int v = 0;
    if(sscanf(str, "%d", &v) == 1 && v >= 0) return (uint16_t)v;
    return 0;
}

/* =================== 蓝牙功能扩展实现 =================== */

// 蓝牙初始化功能
void Custom_Bluetooth_Init(void)
{
    // 基础初始化已在 System_Init 中调用 Bluetooth_Init()
    // 这里可以添加额外的初始化逻辑
    
    // 清空接收缓冲区
    memset(&bt_data, 0, sizeof(BluetoothData_t));
    
    // 添加延时确保蓝牙模块稳定
    Mdelay_Lib(500);
    
    // 发送新的命令说明（替换原来的简单命令）
    Bluetooth_SendString("\r\n=== Smart Agriculture System ===\r\n");
    Mdelay_Lib(100);
    Bluetooth_SendString("Extended Commands Available:\r\n");
    Mdelay_Lib(100);
    Bluetooth_SendString("00 = Show all status\r\n");
    Mdelay_Lib(100);
    Bluetooth_SendString("01 VALUE = Set temp high threshold\r\n");
    Mdelay_Lib(100);
    Bluetooth_SendString("02 VALUE = Set temp low threshold\r\n");
    Mdelay_Lib(100);
    Bluetooth_SendString("03 VALUE = Set humidity high threshold\r\n");
    Mdelay_Lib(100);
    Bluetooth_SendString("04 VALUE = Set humidity low threshold\r\n");
    Mdelay_Lib(100);
    Bluetooth_SendString("05 VALUE = Set light low threshold\r\n");
    Mdelay_Lib(100);
    Bluetooth_SendString("06 VALUE = Set smoke high threshold\r\n");
    Mdelay_Lib(100);
    Bluetooth_SendString("Example: 01 30 (set temp high to 30C)\r\n");
    Mdelay_Lib(100);
    Bluetooth_SendString("=============================\r\n");
}

// 蓝牙数据处理主函数
void Bluetooth_Handler(void)
{
    static uint32_t last_bt_check = 0;
    
    // 每50ms检查一次蓝牙状态
    if(system_tick - last_bt_check >= 50)
    {
        last_bt_check = system_tick;
        
        // 检查是否有命令需要处理
        if(Bluetooth_GetStatus() == BT_STATUS_CMD_READY)
        {
            // 获取接收到的完整命令并处理
            char* received_cmd = Bluetooth_GetReceivedCommand();
            
            // 处理我们的扩展命令
            ProcessReceivedCommand(received_cmd);
            
            // 重置蓝牙状态
            Bluetooth_ProcessCommand();
        }
    }
}

// 处理接收到的命令
void ProcessReceivedCommand(const char* command)
{
    char cmd[4];
    char value_str[16];
    
    Bluetooth_SendString("\r\n"); // 换行
    
    // 解析命令格式：CMD VALUE 或 单独的CMD
    if(sscanf(command, "%2s %s", cmd, value_str) == 2)
    {
        // 有参数的命令
        UserBluetoothCommandHandler(cmd, value_str);
    }
    else if(sscanf(command, "%2s", cmd) == 1)
    {
        // 无参数的命令
        if(strcmp(cmd, BT_CMD_SHOW_STATUS) == 0)
        {
            SendSystemStatus();
        }
        else
        {
            Bluetooth_SendString("ERR: Command needs value\r\n");
        }
    }
    else
    {
        Bluetooth_SendString("ERR: Invalid format\r\n");
        Bluetooth_SendString("Use: CMD VALUE or 00 for status\r\n");
    }
    
    Bluetooth_SendString(">>> "); // 提示符
}

// 发送系统状态
void SendSystemStatus(void)
{
    char status[128];
    
    Bluetooth_SendString("=== SYSTEM STATUS ===\r\n");
    
    // 当前传感器数据
    sprintf(status, "Temp: %d°C (%d-%d)\r\n", 
            sensor_data.temperature, TEMP_LOW_THRESHOLD, TEMP_HIGH_THRESHOLD);
    Bluetooth_SendString(status);
    
    sprintf(status, "Humidity: %d%% (%d-%d)\r\n", 
            sensor_data.humidity, HUMI_LOW_THRESHOLD, HUMI_HIGH_THRESHOLD);
    Bluetooth_SendString(status);
    
    sprintf(status, "Light: %d%% (>%d)\r\n", 
            sensor_data.light_percent, LIGHT_LOW_THRESHOLD);
    Bluetooth_SendString(status);
    
    sprintf(status, "Smoke: %dppm (<%d)\r\n", 
            sensor_data.smoke_ppm_value, SMOKE_HIGH_THRESHOLD);
    Bluetooth_SendString(status);
    
    // MPU6050数据
    if (sensor_data.mpu_status)
    {
        Bluetooth_SendString("\r\nMPU6050 Data:\r\n");
        
        // 加速度数据
        sprintf(status, "Accel: X:%.2f Y:%.2f Z:%.2f g\r\n", 
                sensor_data.mpu_data.accel_x,
                sensor_data.mpu_data.accel_y, 
                sensor_data.mpu_data.accel_z);
        Bluetooth_SendString(status);
        
        // 陀螺仪数据
        sprintf(status, "Gyro: X:%.2f Y:%.2f Z:%.2f °/s\r\n", 
                sensor_data.mpu_data.gyro_x,
                sensor_data.mpu_data.gyro_y, 
                sensor_data.mpu_data.gyro_z);
        Bluetooth_SendString(status);
        
        // 姿态角计算
        float roll = atan2f(sensor_data.mpu_data.accel_y, sensor_data.mpu_data.accel_z) * 57.29578f;
        float pitch = atan2f(-sensor_data.mpu_data.accel_x, 
                     sqrtf(sensor_data.mpu_data.accel_y*sensor_data.mpu_data.accel_y + 
                          sensor_data.mpu_data.accel_z*sensor_data.mpu_data.accel_z)) * 57.29578f;
        
        // 姿态角
        sprintf(status, "Attitude: Roll:%.2f° Pitch:%.2f°\r\n", roll, pitch);
        Bluetooth_SendString(status);
        
        // MPU6050温度
        sprintf(status, "Temp: %.2f°C\r\n", sensor_data.mpu_data.temp);
        Bluetooth_SendString(status);
    }
    else
    {
        Bluetooth_SendString("\r\nMPU6050: Not Available\r\n");
    }
    
    // 报警状态
    Bluetooth_SendString("\r\nAlarms: ");
    if(alarm_status.any_alarm)
    {
        if(alarm_status.temp_high_alarm) Bluetooth_SendString("T-HIGH ");
        if(alarm_status.temp_low_alarm) Bluetooth_SendString("T-LOW ");
        if(alarm_status.humi_high_alarm) Bluetooth_SendString("H-HIGH ");
        if(alarm_status.humi_low_alarm) Bluetooth_SendString("H-LOW ");
        if(alarm_status.light_low_alarm) Bluetooth_SendString("L-LOW ");
        if(alarm_status.smoke_high_alarm) Bluetooth_SendString("S-HIGH ");
    }
    else
    {
        Bluetooth_SendString("NONE");
    }
    Bluetooth_SendString("\r\n");
    
    // 系统信息
    sprintf(status, "Uptime: %lds, Errors: %ld\r\n", 
            system_tick / 1000, sensor_data.error_count);
    Bluetooth_SendString(status);
    
    Bluetooth_SendString("===================\r\n");
}

// 自定义蓝牙发送字符串函数（兼容不同的函数名）
void CustomBluetoothSendString(const char* str)
{
    Bluetooth_SendString((char*)str);
}

/**
 * @brief 获取MPU6050所有数据
 * @param mpu_data: 指向MPU6050_Data_t结构体的指针，用于存储读取的数据
 * @retval None
 */
void MPU6050_GetData(MPU6050_Data_t *mpu_data)
{
    int16_t ax, ay, az, gx, gy, gz;
    uint8_t buf[2];
    
    // 读取加速度计数据
    MPU6050_Read_Accel(&ax, &ay, &az);
    
    // 读取陀螺仪数据
    MPU6050_Read_Gyro(&gx, &gy, &gz);
    
    // 读取温度数据
    MPU6050_Read_Multiple(0x41, buf, 2); // 温度寄存器地址为0x41
    int16_t temp_raw = (buf[0] << 8) | buf[1];
    
    // 转换原始数据为工程单位
    // 加速度: ±2g量程时，LSB灵敏度为16384 LSB/g
    mpu_data->accel_x = (float)ax / 16384.0f;
    mpu_data->accel_y = (float)ay / 16384.0f;
    mpu_data->accel_z = (float)az / 16384.0f;
    
    // 陀螺仪: ±250°/s量程时，LSB灵敏度为131 LSB/(°/s)
    mpu_data->gyro_x = (float)gx / 131.0f;
    mpu_data->gyro_y = (float)gy / 131.0f;
    mpu_data->gyro_z = (float)gz / 131.0f;
    
    // 温度: 温度换算公式 T = (TEMP_OUT/340) + 36.53
    mpu_data->temp = (float)temp_raw / 340.0f + 36.53f;
}
