#include "breathing_led.h"
#include "delay.h"

// 定时器 TIM1 初始化
void TIM1_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 使能 GPIOE 和 TIM1 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    // 配置 PE13 为复用功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 配置 PE14 为复用功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 将 PE13 连接到 TIM1_CH3
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1);
    // 将 PE14 连接到 TIM1_CH4
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);

    // 定时器 TIM1 基本配置
    TIM_TimeBaseStructure.TIM_Period = 999; // 定时器周期
    TIM_TimeBaseStructure.TIM_Prescaler = 83; // 预分频器
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // 配置 TIM1_CH3 为 PWM 模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比为 0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);

    // 配置 TIM1_CH4 为 PWM 模式
    TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比为 0
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);

    // 使能 TIM1 主输出
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // 使能 TIM1
    TIM_Cmd(TIM1, ENABLE);
}

// 呼吸灯效果函数，实现交替闪烁
void Breathing_LED_Effect(void)
{
    uint16_t duty_cycle;
    uint8_t channel = 3; // 初始选择 TIM1_CH3

    while (1)
    {
        if (channel == 3)
        {
            // TIM1_CH3 亮度增加
            for (duty_cycle = 0; duty_cycle <= 999; duty_cycle++)
            {
                TIM_SetCompare3(TIM1, duty_cycle);
                TIM_SetCompare4(TIM1, 0); // TIM1_CH4 熄灭
                Mdelay_Lib(1); // 使用毫秒级延时函数，可根据实际情况调整延时时间
            }
            // TIM1_CH3 亮度减小
            for (duty_cycle = 999; duty_cycle > 0; duty_cycle--)
            {
                TIM_SetCompare3(TIM1, duty_cycle);
                TIM_SetCompare4(TIM1, 0); // TIM1_CH4 熄灭
                Mdelay_Lib(1); // 使用毫秒级延时函数，可根据实际情况调整延时时间
            }
            channel = 4; // 切换到 TIM1_CH4
        }
        else
        {
            // TIM1_CH4 亮度增加
            for (duty_cycle = 0; duty_cycle <= 999; duty_cycle++)
            {
                TIM_SetCompare4(TIM1, duty_cycle);
                TIM_SetCompare3(TIM1, 0); // TIM1_CH3 熄灭
                Mdelay_Lib(2); // 使用毫秒级延时函数，可根据实际情况调整延时时间
            }
            // TIM1_CH4 亮度减小
            for (duty_cycle = 999; duty_cycle > 0; duty_cycle--)
            {
                TIM_SetCompare4(TIM1, duty_cycle);
                TIM_SetCompare3(TIM1, 0); // TIM1_CH3 熄灭
                Mdelay_Lib(2); // 使用毫秒级延时函数，可根据实际情况调整延时时间
            }
            channel = 3; // 切换到 TIM1_CH3
        }
    }
}


