/**
 * @file    DHT11.c
 * @brief   DHT11温湿度传感器驱动实现
 * @author  Mika
 * @date    2025-07-01
 * @version 3.0
 * 
 * @note    DHT11数字温湿度传感器驱动实现
 *          数据格式：湿度整数+湿度小数+温度整数+温度小数+校验和
 *          通信协议：单总线协议
 *          使用delay.h中的Mdelay_Lib和Udelay_Lib函数进行延时
 */

#include "DHT11.h"
#include "delay.h"

/**
 * @brief 配置DHT11引脚为输出模式
 * @param 无
 * @retval 无
 */
void DHT11_IO_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = DHT11_IO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       // 输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;  // 高速
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // 无上下拉
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

/**
 * @brief 配置DHT11引脚为浮空输入模式
 * @param 无
 * @retval 无
 */
void DHT11_IO_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = DHT11_IO;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;        // 输入模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // 浮空输入
    GPIO_Init(DHT11_PORT, &GPIO_InitStructure);
}

/**
 * @brief 启动DHT11通信
 * @param 无
 * @retval 无
 * @note 发送开始信号：拉低至少18ms，然后拉高20-40us
 */
void DHT11_Start(void)
{
    DHT11_IO_Output();                      // 设置为输出模式
    GPIO_ResetBits(DHT11_PORT, DHT11_IO);   // 将总线拉低
    Mdelay_Lib(20);                         // 延时至少18ms，这里用20ms
    GPIO_SetBits(DHT11_PORT, DHT11_IO);     // 将总线拉高
    Udelay_Lib(30);                         // 拉高20-40us
}

/**
 * @brief 检查DHT11是否响应
 * @param 无
 * @retval 0-响应成功，1-响应失败
 * @note DHT11响应：拉低80us，然后拉高80us
 */
unsigned char DHT11_Check(void)
{
    unsigned char n = 0;
    
    DHT11_IO_Input();  // 确保是输入模式
    
    // 等待DHT11拉低总线（响应开始）
    while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO) == 1 && n < 100)
    {
        n++;
        Udelay_Lib(1);
    }
    if(n >= 100)
        return 1;  // 超时，未响应
    
    n = 0;
    // 等待DHT11拉高总线（响应结束）
    while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO) == 0 && n < 100)
    {
        n++;
        Udelay_Lib(1);
    }
    if(n >= 100)
        return 1;  // 超时，未响应
    
    return 0;  // 响应成功
}

/**
 * @brief 读取一个数据位
 * @param 无
 * @retval 读取到的位值（0或1）
 * @note DHT11数据位协议：
 *       数据0：50us低电平 + 26-28us高电平
 *       数据1：50us低电平 + 70us高电平
 */
unsigned char DHT11_ReadBit(void)
{
    unsigned char n = 0;
    
    // 等待低电平结束（每个数据位都以50us低电平开始）
    while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO) == 0 && n < 100)
    {
        n++;
        Udelay_Lib(1);
    }
    
    n = 0;
    // 计算高电平持续时间
    while(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO) == 1 && n < 100)
    {
        n++;
        Udelay_Lib(1);
    }
    
    // 根据高电平持续时间判断数据位
    // 高电平持续时间大于30us为1，否则为0
    return n > 30 ? 1 : 0;
}

/**
 * @brief 读取一个字节
 * @param 无
 * @retval 读取到的字节数据
 * @note 从高位到低位依次读取8个位
 */
unsigned char DHT11_ReadByte(void)
{
    unsigned char i, dat = 0;
    
    for(i = 0; i < 8; i++)
    {
        dat <<= 1;              // 左移一位
        dat |= DHT11_ReadBit(); // 读取一位并或运算
    }
    
    return dat;
}

/**
 * @brief DHT11初始化
 * @param 无
 * @retval 无
 */
void DHT11_Init(void)
{
    // 使能GPIOE时钟
    RCC_AHB1PeriphClockCmd(DHT11_RCC, ENABLE);
    
    // 配置为输出模式，拉高数据线
    DHT11_IO_Output();
    GPIO_SetBits(DHT11_PORT, DHT11_IO);
}

/**
 * @brief 读取DHT11温湿度数据
 * @param temp: 温度值指针
 * @param humi: 湿度值指针
 * @retval 0-读取成功，1-读取失败
 * @note 数据格式：湿度整数+湿度小数+温度整数+温度小数+校验和
 */
unsigned char DHT11_ReadData(unsigned char *temp, unsigned char *humi)
{
    unsigned char buf[5];
    int i;
    
    // 发送开始信号
    DHT11_Start();
    
    // 检查DHT11是否响应
    if(DHT11_Check() == 0)  // 响应成功
    {
        // 读取40位数据（5个字节）
        for(i = 0; i < 5; i++)
        {
            buf[i] = DHT11_ReadByte();
        }
        
        // 校验数据
        if((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
        {
            *humi = buf[0];    // 湿度整数部分
            *temp = buf[2];    // 温度整数部分
            
            return 0;  // 读取成功
        }
    }
    
    return 1;  // 读取失败
}
