#ifndef __DELAY_H__
#define __DELAY_H__ 

#include "stm32f4xx.h"

/**
 * @file    delay.h
 * @brief   基于SysTick定时器的精确延时函数头文件
 * @author  Mika
 * @date    2025-06-30
 * @version 1.0
 * 
 * @note    使用SysTick定时器实现精确的毫秒和微秒级延时
 *          通过中断方式实现，不会阻塞其他中断处理
 */

// SysTick寄存器直接访问定义
#define MYCTL     *((volatile unsigned long *)0xE000E010)   // SysTick控制和状态寄存器
#define MYLOAD    *((volatile unsigned long *)0xE000E014)   // SysTick重装载数值寄存器  
#define MYCURVAL  *((volatile unsigned long *)0xE000E018)   // SysTick当前数值寄存器

/**
 * @brief 毫秒级精确延时函数
 * @param nms: 延时时间，单位：毫秒(ms)
 * @retval 无
 * @note 利用SysTick定时器实现精确的毫秒级延时
 *       函数内部会配置SysTick定时器产生1ms间隔的中断
 *       延时期间CPU可以响应其他中断
 */
void Mdelay_Lib(int nms);

/**
 * @brief 微秒级精确延时函数  
 * @param nms: 延时时间，单位：微秒(us)
 * @retval 无
 * @note 利用SysTick定时器实现精确的微秒级延时
 *       函数内部会配置SysTick定时器产生1us间隔的中断
 *       延时期间CPU可以响应其他中断
 */
void Udelay_Lib(int nms);

#endif 

