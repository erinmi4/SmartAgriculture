#include "beep.h"

// 初始化蜂鸣器
void Beep_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能GPIOF的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    // 配置BEEP (PF8) 为推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    // 默认关闭蜂鸣器
    GPIO_ResetBits(BEEP_GPIO, BEEP_PIN);
}

// 打开蜂鸣器
void Beep_On(void)
{
    GPIO_SetBits(BEEP_GPIO, BEEP_PIN);
}

// 关闭蜂鸣器
void Beep_Off(void)
{
    GPIO_ResetBits(BEEP_GPIO, BEEP_PIN);
}

