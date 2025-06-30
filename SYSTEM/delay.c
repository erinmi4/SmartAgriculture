/**
 * @file    delay.c
 * @brief   基于SysTick定时器的精确延时函数实现
 * @author  Mika
 * @date    2025-06-30
 * @version 1.0
 * 
 * @note    本文件实现了基于SysTick定时器的精确延时功能
 *          支持毫秒级和微秒级延时，通过中断方式实现
 *          延时过程中不会阻塞其他中断的响应
 */

#include "delay.h"

// 全局延时计数变量，在SysTick中断中递减
static int mdelay_time;

/**
 * @brief SysTick定时器中断服务函数
 * @param 无
 * @retval 无
 * @note 每次SysTick定时器溢出都会调用此函数
 *       函数内部将延时计数器mdelay_time递减
 *       当mdelay_time减到0时，延时结束
 */
void SysTick_Handler(void)
{
    // 如果延时时间大于0，则递减计数器
    if(mdelay_time > 0)
    {
        mdelay_time--;
    }
}

/**
 * @brief 毫秒级精确延时函数
 * @param nms: 延时时间，单位：毫秒(ms)
 * @retval 无
 * @note 利用固件库控制SysTick定时器实现的毫秒级别的延时函数
 *       工作原理：
 *       1. 配置SysTick定时器，使其每1毫秒产生一次中断
 *       2. 设置中断优先级为最高级(0)
 *       3. 在中断服务函数中递减计数器
 *       4. 主函数等待计数器减到0
 */
void Mdelay_Lib(int nms)
{
    // 配置SysTick定时器让其一毫秒产生一次中断
    // SystemCoreClock是系统时钟频率，除以1000得到1ms的计数值
    SysTick_Config(SystemCoreClock/1000);

    // 让SysTick中断优先级为0(最高级)
    // 确保延时中断能够及时响应
    NVIC_SetPriority(SysTick_IRQn, 0);

    // 设置延时时间，此时SysTick每一毫秒产生一次中断，mdelay_time就减1
    mdelay_time = nms;
    
    // 等待延时完成，当mdelay_time减到0时退出循环
    while(mdelay_time > 0)
    {
        // 空循环等待，期间可以响应其他中断
    }
}

/**
 * @brief 微秒级精确延时函数
 * @param nms: 延时时间，单位：微秒(us)  
 * @retval 无
 * @note 利用固件库控制SysTick定时器实现的微秒级别的延时函数
 *       工作原理：
 *       1. 配置SysTick定时器，使其每1微秒产生一次中断
 *       2. 设置中断优先级为最高级(0)
 *       3. 在中断服务函数中递减计数器
 *       4. 主函数等待计数器减到0
 */
void Udelay_Lib(int nms)
{
    // 配置SysTick定时器让其一微秒产生一次中断
    // SystemCoreClock是系统时钟频率，除以1000000得到1us的计数值
    SysTick_Config(SystemCoreClock/1000000);

    // 让SysTick中断优先级为0(最高级)
    // 确保延时中断能够及时响应
    NVIC_SetPriority(SysTick_IRQn, 0);

    // 设置延时时间，此时SysTick每一微秒产生一次中断，mdelay_time就减1
    mdelay_time = nms;
    
    // 等待延时完成，当mdelay_time减到0时退出循环  
    while(mdelay_time > 0)
    {
        // 空循环等待，期间可以响应其他中断
    }
}















