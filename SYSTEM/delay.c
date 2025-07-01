#include "delay.h"

static volatile int mdelay_time;  // ���volatile�ؼ��ַ�ֹ�������Ż�

void SysTick_Handler(void)
{
	
	if(mdelay_time > 0)
	{
		mdelay_time--;
	}

}
/*
    ���ù̼������SysTick��ʱ��ʵ�ֵĺ��뼶�����ʱ����
*/
void Mdelay_Lib(int nms)
{
    //����SysTick��ʱ������һ�������һ���ж�
    SysTick_Config(SystemCoreClock/1000);

    //��SysTick�ж����ȼ�Ϊ0(��߼�)
    NVIC_SetPriority (SysTick_IRQn,0);

    //���úú�,��ʱSysTickÿһ�������һ���ж�mdelay_time�ͼ�1
    mdelay_time = nms;
    while(mdelay_time > 0) {
        // �ȴ��жϵݼ�mdelay_time����ӿղ�����ֹ�������Ż�
        __NOP();
    }
}

// �򵥵������ʱ�������������ж�
void Simple_Delay(volatile uint32_t count)
{
    while(count--) {
        __NOP();  // �ղ�������ֹ�������Ż�
    }
}

// ���뼶�����ʱ����Լ��ʱ����168MHzʱ���£�
void Simple_Delay_Ms(uint32_t ms)
{
    volatile uint32_t i, j;
    for(i = 0; i < ms; i++) {
        for(j = 0; j < 42000; j++) {  // ��Լ1ms����ʱѭ��
            __NOP();
        }
    }
}

/*
    ���ù̼������SysTick��ʱ��ʵ�ֵ�΢�뼶�����ʱ����
*/
void Udelay_Lib(int nms)
{
    //����SysTick��ʱ������һ΢�����һ���ж�
    SysTick_Config(SystemCoreClock/1000000);

    //��SysTick�ж����ȼ�Ϊ0(��߼�)
    NVIC_SetPriority (SysTick_IRQn,0);

    //���úú�,��ʱSysTickÿһ΢�����һ���ж�mdelay_time�ͼ�1
    mdelay_time = nms;
    while(mdelay_time)
;
}















