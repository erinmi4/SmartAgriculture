#include "key.h"

// 初始化KEY0
void Key0_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能KEY0对应GPIO的时钟
    RCC_AHB1PeriphClockCmd(KEY0_RCC, ENABLE);

    // 配置KEY0为输入模式，带上拉
    GPIO_InitStructure.GPIO_Pin = KEY0_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(KEY0_GPIO, &GPIO_InitStructure);
}

// 初始化KEY1
void Key1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能KEY1对应GPIO的时钟
    RCC_AHB1PeriphClockCmd(KEY1_RCC, ENABLE);

    // 配置KEY1为输入模式，带上拉
    GPIO_InitStructure.GPIO_Pin = KEY1_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(KEY1_GPIO, &GPIO_InitStructure);
}

// 初始化KEY2
void Key2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能KEY2对应GPIO的时钟
    RCC_AHB1PeriphClockCmd(KEY2_RCC, ENABLE);

    // 配置KEY2为输入模式，带上拉
    GPIO_InitStructure.GPIO_Pin = KEY2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(KEY2_GPIO, &GPIO_InitStructure);
}

// 初始化KEY3
void Key3_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能KEY3对应GPIO的时钟
    RCC_AHB1PeriphClockCmd(KEY3_RCC, ENABLE);

    // 配置KEY3为输入模式，带上拉
    GPIO_InitStructure.GPIO_Pin = KEY3_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(KEY3_GPIO, &GPIO_InitStructure);
}

// 初始化所有按键
void Key_Init(void)
{
    // 调用各个按键的初始化函数
    Key0_Init();
    Key1_Init();
    Key2_Init();
    Key3_Init();
}

// 按键防抖函数 - 非阻塞版本
uint8_t Key_Debounce(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    // 简单的按键检测，不使用阻塞延时
    if (GPIO_ReadInputDataBit(GPIOx, GPIO_Pin) == Bit_RESET) // 判断是否按下
    {
        return 1; // 按下
    }
    return 0; // 未按下
}

