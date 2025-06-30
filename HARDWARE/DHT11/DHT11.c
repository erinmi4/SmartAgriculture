/**
 * @file    DHT11.c
 * @brief   DHT11温湿度传感器驱动实现
 * @author  Mika
 * @date    2025-06-30
 * @version 1.0
 * 
 * @note    DHT11数字温湿度传感器驱动实现
 *          数据格式：湿度整数+湿度小数+温度整数+温度小数+校验和
 *          通信协议：单总线协议
 */

#include "DHT11.h"

// 全局变量定义
unsigned char dht11_temp = 0;  // 温度值
unsigned char dht11_humi = 0;  // 湿度值

/**
 * @brief 简单的微秒延时
 * @param us: 延时微秒数
 * @retval 无
 * @note 使用简单循环实现，避免系统冲突
 */
static void dht11_delay_us(uint32_t us)
{
    uint32_t count = us * 168;  // 根据168MHz系统时钟计算
    volatile uint32_t i;
    for(i = 0; i < count; i++)
    {
        __NOP();
    }
}

/**
 * @brief 简单的毫秒延时
 * @param ms: 延时毫秒数
 * @retval 无
 */
static void dht11_delay_ms(uint32_t ms)
{
    uint32_t i;
    for(i = 0; i < ms; i++)
    {
        dht11_delay_us(1000);
    }
}

/**
 * @brief 配置DHT11引脚为输出模式
 * @param 无
 * @retval 无
 */
void dht11_io_out(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = DHT11_IO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       // 输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 输出速度50MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

/**
 * @brief 配置DHT11引脚为输入模式
 * @param 无
 * @retval 无
 */
void dht11_io_in(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = DHT11_IO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;        // 输入模式
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 输入速度50MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

/**
 * @brief DHT11初始化
 * @param 无
 * @retval 0-初始化成功，1-初始化失败
 */
unsigned char dht11_init(void)
{
    // 使能GPIOE时钟
    RCC_AHB1PeriphClockCmd(DHT11_RCC, ENABLE);
    
    // 配置引脚为输出模式并拉高
    dht11_io_out();
    DHT11_DQ_OUT;
    
    // 发送开始信号并检查响应
    dht11_start();
    return dht11_check();
}

/**
 * @brief 发送开始信号
 * @param 无
 * @retval 无
 * @note 主机发送开始信号：拉低至少18ms，然后拉高20-40us
 */
void dht11_start(void)
{
    dht11_io_out();     // 设置为输出模式
    
    DHT11_DQ_LOW;       // 将总线拉低
    dht11_delay_ms(20); // 延时至少18ms，这里用20ms确保稳定
    
    DHT11_DQ_OUT;       // 将总线拉高
    dht11_delay_us(30); // 拉高20-40us，这里用30us
}

/**
 * @brief 等待DHT11响应
 * @param 无
 * @retval 0-DHT11响应正常，1-DHT11响应超时
 * @note DHT11响应：拉低80us，然后拉高80us
 */
unsigned char dht11_check(void)
{
    unsigned char retry = 0;
    
    dht11_io_in();      // 设置为输入模式
    
    // 等待DHT11拉低总线（响应信号）
    while(DHT11_DQ_IN && retry < 100)
    {
        retry++;
        dht11_delay_us(1);
    }
    if(retry >= 100)
        return 1;       // 超时，DHT11无响应
    
    retry = 0;
    // 等待DHT11拉高总线
    while(!DHT11_DQ_IN && retry < 100)
    {
        retry++;
        dht11_delay_us(1);
    }
    if(retry >= 100)
        return 1;       // 超时，DHT11响应异常
    
    return 0;           // DHT11响应正常
}

/**
 * @brief 读取一个位
 * @param 无
 * @retval 读取到的位值（0或1）
 * @note 数据位传输：低电平50us + 高电平26-28us表示0，低电平50us + 高电平70us表示1
 */
unsigned char dht11_read_bit(void)
{
    unsigned char retry = 0;
    
    // 等待低电平结束
    while(DHT11_DQ_IN && retry < 100)
    {
        retry++;
        dht11_delay_us(1);
    }
    
    retry = 0;
    // 等待高电平开始
    while(!DHT11_DQ_IN && retry < 100)
    {
        retry++;
        dht11_delay_us(1);
    }
    
    // 延时40us后判断数据位
    dht11_delay_us(40);
    
    if(DHT11_DQ_IN)
        return 1;       // 高电平时间长，表示数据位1
    else
        return 0;       // 高电平时间短，表示数据位0
}

/**
 * @brief 读取一个字节
 * @param 无
 * @retval 读取到的字节数据
 * @note 从高位到低位依次读取8个位
 */
unsigned char dht11_read_byte(void)
{
    unsigned char i, dat = 0;
    
    for(i = 0; i < 8; i++)
    {
        dat <<= 1;              // 左移一位
        dat |= dht11_read_bit(); // 读取一位数据
    }
    
    return dat;
}

/**
 * @brief 读取DHT11温湿度数据
 * @param 无
 * @retval 0-读取成功，1-读取失败
 * @note 数据格式：湿度整数部分+湿度小数部分+温度整数部分+温度小数部分+校验和
 */
unsigned char dht11_read_data(void)
{
    unsigned char buf[5];
    unsigned char i;
    
    // 发送开始信号
    dht11_start();
    
    // 检查DHT11响应
    if(dht11_check() == 0)
    {
        // 读取5个字节数据
        for(i = 0; i < 5; i++)
        {
            buf[i] = dht11_read_byte();
        }
        
        // 校验数据
        if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            dht11_humi = buf[0];    // 湿度整数部分
            dht11_temp = buf[2];    // 温度整数部分
            return 0;               // 读取成功
        }
    }
    
    return 1;                       // 读取失败
}
