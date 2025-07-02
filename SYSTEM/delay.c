#include "delay.h"

static volatile int mdelay_time;  // ���volatile�ؼ��ַ�ֹ�������Ż�
extern volatile uint32_t system_tick;  // ����main.c�е�ϵͳʱ��

void SysTick_Handler(void)
{
    // ������ʱ������
    if(mdelay_time > 0)
    {
        mdelay_time--;
    }
    
    // ͬʱ����ϵͳʱ��
    system_tick++;
}
/*
    ���뼶��ʱ���� - �޸İ棬�����������SysTick����
*/
void Mdelay_Lib(int nms)
{
    // ���SysTick�Ƿ��Ѿ�����Ϊ1ms
    if(SysTick->LOAD == (SystemCoreClock/1000 - 1))
    {
        // SysTick�Ѿ���ȷ����Ϊ1ms��ֱ��ʹ��system_tick
        uint32_t start_tick = system_tick;
        while((system_tick - start_tick) < nms)
        {
            __NOP();
        }
    }
    else
    {
        // ���SysTickδ���û����ô���ʹ�������ʱ
        Simple_Delay_Ms(nms);
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
    ΢�뼶��ʱ���� - �������汾��������SysTick����
*/
void Udelay_Lib(int nms)
{
    // ʹ�������ʱ��������������SysTick
    volatile uint32_t count = nms * 42;  // ��Լ1΢���ѭ������
    while(count--)
    {
        __NOP();
    }
}















