#include "delay.h"

static volatile int mdelay_time;  // 添加volatile关键字防止编译器优化
extern volatile uint32_t system_tick;  // 引用main.c中的系统时钟

void SysTick_Handler(void)
{
    // 更新延时计数器
    if(mdelay_time > 0)
    {
        mdelay_time--;
    }
    
    // 同时更新系统时钟
    system_tick++;
}
/*
    毫秒级延时函数 - 修改版，避免干扰现有SysTick配置
*/
void Mdelay_Lib(int nms)
{
    // 检查SysTick是否已经配置为1ms
    if(SysTick->LOAD == (SystemCoreClock/1000 - 1))
    {
        // SysTick已经正确配置为1ms，直接使用system_tick
        uint32_t start_tick = system_tick;
        while((system_tick - start_tick) < nms)
        {
            __NOP();
        }
    }
    else
    {
        // 如果SysTick未配置或配置错误，使用软件延时
        Simple_Delay_Ms(nms);
    }
}

// 简单的软件延时函数，不依赖中断
void Simple_Delay(volatile uint32_t count)
{
    while(count--) {
        __NOP();  // 空操作，防止编译器优化
    }
}

// 毫秒级软件延时，大约延时（在168MHz时钟下）
void Simple_Delay_Ms(uint32_t ms)
{
    volatile uint32_t i, j;
    for(i = 0; i < ms; i++) {
        for(j = 0; j < 42000; j++) {  // 大约1ms的延时循环
            __NOP();
        }
    }
}

/*
    微秒级延时函数 - 非阻塞版本，不干扰SysTick配置
*/
void Udelay_Lib(int nms)
{
    // 使用软件延时，避免重新配置SysTick
    volatile uint32_t count = nms * 42;  // 大约1微秒的循环计数
    while(count--)
    {
        __NOP();
    }
}















