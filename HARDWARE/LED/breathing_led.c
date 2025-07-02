#include "breathing_led.h"
#include "delay.h"

// ��ʱ�� TIM1 ��ʼ��
void TIM1_PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // ʹ�� GPIOE �� TIM1 ʱ��
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    // ���� PE13 Ϊ���ù���
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // ���� PE14 Ϊ���ù���
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // �� PE13 ���ӵ� TIM1_CH3
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1);
    // �� PE14 ���ӵ� TIM1_CH4
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);

    // ��ʱ�� TIM1 ��������
    TIM_TimeBaseStructure.TIM_Period = 999; // ��ʱ������
    TIM_TimeBaseStructure.TIM_Prescaler = 83; // Ԥ��Ƶ��
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // ���� TIM1_CH3 Ϊ PWM ģʽ
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0; // ��ʼռ�ձ�Ϊ 0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);

    // ���� TIM1_CH4 Ϊ PWM ģʽ
    TIM_OCInitStructure.TIM_Pulse = 0; // ��ʼռ�ձ�Ϊ 0
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);

    // ʹ�� TIM1 �����
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // ʹ�� TIM1
    TIM_Cmd(TIM1, ENABLE);
}

// ������Ч��������ʵ�ֽ�����˸
void Breathing_LED_Effect(void)
{
    uint16_t duty_cycle;
    uint8_t channel = 3; // ��ʼѡ�� TIM1_CH3

    while (1)
    {
        if (channel == 3)
        {
            // TIM1_CH3 ��������
            for (duty_cycle = 0; duty_cycle <= 999; duty_cycle++)
            {
                TIM_SetCompare3(TIM1, duty_cycle);
                TIM_SetCompare4(TIM1, 0); // TIM1_CH4 Ϩ��
                Mdelay_Lib(1); // ʹ�ú��뼶��ʱ�������ɸ���ʵ�����������ʱʱ��
            }
            // TIM1_CH3 ���ȼ�С
            for (duty_cycle = 999; duty_cycle > 0; duty_cycle--)
            {
                TIM_SetCompare3(TIM1, duty_cycle);
                TIM_SetCompare4(TIM1, 0); // TIM1_CH4 Ϩ��
                Mdelay_Lib(1); // ʹ�ú��뼶��ʱ�������ɸ���ʵ�����������ʱʱ��
            }
            channel = 4; // �л��� TIM1_CH4
        }
        else
        {
            // TIM1_CH4 ��������
            for (duty_cycle = 0; duty_cycle <= 999; duty_cycle++)
            {
                TIM_SetCompare4(TIM1, duty_cycle);
                TIM_SetCompare3(TIM1, 0); // TIM1_CH3 Ϩ��
                Mdelay_Lib(2); // ʹ�ú��뼶��ʱ�������ɸ���ʵ�����������ʱʱ��
            }
            // TIM1_CH4 ���ȼ�С
            for (duty_cycle = 999; duty_cycle > 0; duty_cycle--)
            {
                TIM_SetCompare4(TIM1, duty_cycle);
                TIM_SetCompare3(TIM1, 0); // TIM1_CH3 Ϩ��
                Mdelay_Lib(2); // ʹ�ú��뼶��ʱ�������ɸ���ʵ�����������ʱʱ��
            }
            channel = 3; // �л��� TIM1_CH3
        }
    }
}


