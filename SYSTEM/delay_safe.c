/**
 * @file    delay_safe.c
 * @brief   安全的延时函数实现（替代原有的delay.c）
 * @author  Mika
 * @date    2025-06-30
 * @version 2.0
 * 
 * @note    使用简单的循环延时，避免SysTick配置冲突
 *          适用于LCD驱动等对延时精度要求不是特别高的场合
 */

#include "delay.h"

// 延时校准参数（根据系统时钟调整）
static uint32_t fac_us = 0;    // us延时倍乘数
static uint32_t fac_ms = 0;    // ms延时倍乘数

/**
 * @brief 延时初始化
 * @param 无
 * @retval 无
 * @note 计算延时校准参数，避免使用SysTick
 */
void delay_init(void)
{
    // 假设系统时钟为168MHz，根据实际情况调整
    fac_us = SystemCoreClock / 8000000;  // 大约每us需要的循环次数
    fac_ms = fac_us * 1000;              // 大约每ms需要的循环次数
}

/**
 * @brief 简单的微秒延时
 * @param nus: 延时微秒数
 * @retval 无
 * @note 使用简单循环实现，避免SysTick冲突
 */
void Udelay_Lib(int nus)
{
    uint32_t temp;
    uint32_t count = nus * fac_us;
    
    for(temp = 0; temp < count; temp++)
    {
        __NOP();  // 空操作，消耗时间
    }
}

/**
 * @brief 简单的毫秒延时
 * @param nms: 延时毫秒数
 * @retval 无
 * @note 使用简单循环实现，避免SysTick冲突
 */
void Mdelay_Lib(int nms)
{
    uint32_t temp;
    
    for(temp = 0; temp < nms; temp++)
    {
        Udelay_Lib(1000);  // 延时1ms
    }
}

/**
 * @brief 超简单延时（用于LCD初始化）
 * @param count: 延时计数
 * @retval 无
 * @note 最简单的延时，确保不会有任何冲突
 */
void simple_delay(uint32_t count)
{
    volatile uint32_t i;
    for(i = 0; i < count; i++)
    {
        __NOP();
    }
}
