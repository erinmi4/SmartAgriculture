#include "dht11.h"
#include "delay.h"

void dht11_io_out()
{
// 配置DHT11的GPIO为输出模式
    GPIO_InitTypeDef g;
    g.GPIO_Pin = DHT11_IO;
    g.GPIO_Mode = GPIO_Mode_OUT;
    g.GPIO_OType = GPIO_OType_PP;
    g.GPIO_Speed = GPIO_Speed_100MHz;
    g.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(DHT11_PORT, &g);
}

void dht11_io_in()
{
// 配置DHT11的GPIO为浮空输入模式
    GPIO_InitTypeDef g;
    g.GPIO_Pin = DHT11_IO;
    g.GPIO_Mode = GPIO_Mode_IN;
    g.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(DHT11_PORT, &g);
}

// 初始化DHT11
void dht11_init()
{
    RCC_AHB1PeriphClockCmd(DHT11_RCC, ENABLE);
    dht11_start();
    dht11_check();
}

// 启动DHT11
void dht11_start()
{
    dht11_io_out();
    // 将总线拉低
    GPIO_ResetBits(DHT11_PORT, DHT11_IO);
    // 延时至少18ms
    Mdelay_Lib(20);
    // 将总线拉高
    GPIO_SetBits(DHT11_PORT, DHT11_IO);
    // 拉高20~40us
    Udelay_Lib(30);
}

// 检查DHT11是否响应
unsigned char dht11_check()
{
    unsigned char n = 0;
    dht11_io_in();
    while((GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO) == 1) && n < 100)
    {
        n++;
        Udelay_Lib(1);
    }
    if(n >= 100)
        return 1;
    else 
        n = 0;
    while((GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO) == 0) && n < 100)
    {
        n++;
        Udelay_Lib(1);
    }
    if(n >= 100)
        return 1;
    else 
        return 0;
}

// 读取一个数据位
unsigned char dht11_read_bit()
{
    unsigned char n = 0;
    while((GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO) == 1) && n < 100)
    {
        n++;
        Udelay_Lib(1);
    }
    n = 0;
    while((GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO) == 0) && n < 100)
    {
        n++;
        Udelay_Lib(1);
    }
    Udelay_Lib(40);
    if(GPIO_ReadInputDataBit(DHT11_PORT, DHT11_IO) == 1)
        return 1;
    else
        return 0;
}

// 读取一个字节
unsigned char dht11_read_byte()
{
    unsigned char i, dat;
    dat = 0;
    for(i = 0; i < 8; i++)
    {
        dat = dat << 1;
        dat = dat | dht11_read_bit();
    }
    return dat;
}

// 读取温湿度数据
unsigned char dht11_read_dat(unsigned char *temp, unsigned char *humi)
{
    unsigned char buf[5];
    dht11_start();
    if(dht11_check() == 0)
    {
        for(int i = 0; i < 5; i++)
        {
            buf[i] = dht11_read_byte();
        }
        if((buf[0]+buf[1]+buf[2]+buf[3]) == buf[4])
        {
            *humi = buf[0];// 湿度整数部分
            *temp = buf[2];// 温度整数部分
        }
    }
    else
        return 1;
    return 0;
	}
