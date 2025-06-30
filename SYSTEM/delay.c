/**
 * @file    delay.c
 * @brief   ����SysTick��ʱ���ľ�ȷ��ʱ����ʵ��
 * @author  Mika
 * @date    2025-06-30
 * @version 1.0
 * 
 * @note    ���ļ�ʵ���˻���SysTick��ʱ���ľ�ȷ��ʱ����
 *          ֧�ֺ��뼶��΢�뼶��ʱ��ͨ���жϷ�ʽʵ��
 *          ��ʱ�����в������������жϵ���Ӧ
 */

#include "delay.h"

// ȫ����ʱ������������SysTick�ж��еݼ�
static int mdelay_time;

/**
 * @brief SysTick��ʱ���жϷ�����
 * @param ��
 * @retval ��
 * @note ÿ��SysTick��ʱ�����������ô˺���
 *       �����ڲ�����ʱ������mdelay_time�ݼ�
 *       ��mdelay_time����0ʱ����ʱ����
 */
void SysTick_Handler(void)
{
    // �����ʱʱ�����0����ݼ�������
    if(mdelay_time > 0)
    {
        mdelay_time--;
    }
}

/**
 * @brief ���뼶��ȷ��ʱ����
 * @param nms: ��ʱʱ�䣬��λ������(ms)
 * @retval ��
 * @note ���ù̼������SysTick��ʱ��ʵ�ֵĺ��뼶�����ʱ����
 *       ����ԭ��
 *       1. ����SysTick��ʱ����ʹ��ÿ1�������һ���ж�
 *       2. �����ж����ȼ�Ϊ��߼�(0)
 *       3. ���жϷ������еݼ�������
 *       4. �������ȴ�����������0
 */
void Mdelay_Lib(int nms)
{
    // ����SysTick��ʱ������һ�������һ���ж�
    // SystemCoreClock��ϵͳʱ��Ƶ�ʣ�����1000�õ�1ms�ļ���ֵ
    SysTick_Config(SystemCoreClock/1000);

    // ��SysTick�ж����ȼ�Ϊ0(��߼�)
    // ȷ����ʱ�ж��ܹ���ʱ��Ӧ
    NVIC_SetPriority(SysTick_IRQn, 0);

    // ������ʱʱ�䣬��ʱSysTickÿһ�������һ���жϣ�mdelay_time�ͼ�1
    mdelay_time = nms;
    
    // �ȴ���ʱ��ɣ���mdelay_time����0ʱ�˳�ѭ��
    while(mdelay_time > 0)
    {
        // ��ѭ���ȴ����ڼ������Ӧ�����ж�
    }
}

/**
 * @brief ΢�뼶��ȷ��ʱ����
 * @param nms: ��ʱʱ�䣬��λ��΢��(us)  
 * @retval ��
 * @note ���ù̼������SysTick��ʱ��ʵ�ֵ�΢�뼶�����ʱ����
 *       ����ԭ��
 *       1. ����SysTick��ʱ����ʹ��ÿ1΢�����һ���ж�
 *       2. �����ж����ȼ�Ϊ��߼�(0)
 *       3. ���жϷ������еݼ�������
 *       4. �������ȴ�����������0
 */
void Udelay_Lib(int nms)
{
    // ����SysTick��ʱ������һ΢�����һ���ж�
    // SystemCoreClock��ϵͳʱ��Ƶ�ʣ�����1000000�õ�1us�ļ���ֵ
    SysTick_Config(SystemCoreClock/1000000);

    // ��SysTick�ж����ȼ�Ϊ0(��߼�)
    // ȷ����ʱ�ж��ܹ���ʱ��Ӧ
    NVIC_SetPriority(SysTick_IRQn, 0);

    // ������ʱʱ�䣬��ʱSysTickÿһ΢�����һ���жϣ�mdelay_time�ͼ�1
    mdelay_time = nms;
    
    // �ȴ���ʱ��ɣ���mdelay_time����0ʱ�˳�ѭ��  
    while(mdelay_time > 0)
    {
        // ��ѭ���ȴ����ڼ������Ӧ�����ж�
    }
}















