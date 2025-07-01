#include "stm32f4xx.h"
#include "adc1.h"
#include "sys.h"
 
/**
 * @brief  初始化ADC1
 * @param  None
 * @retval None
 * @note   配置ADC1为单次转换、软件触发模式
 */
void Adc1_Init(void)
{
	// 定义相关结构体
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_InitTypeDef       ADC_InitStructure;

	// 使能ADC1时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	// 复位ADC1
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, ENABLE);
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC1, DISABLE);
	
	// 初始化ADC通用寄存器
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;                  // 独立模式
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div4;               // APB2时钟4分频 (PCLK2/4)
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;   // 禁止DMA
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // 两个采样阶段之间延迟5个时钟
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	// 初始化ADC1参数
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;                // 12位分辨率
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;                         // 禁止扫描模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                   // 禁止连续转换
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // 禁止外部触发，使用软件触发
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;                // 数据右对齐
	ADC_InitStructure.ADC_NbrOfConversion = 1;                            // 1个转换在规则序列中
	ADC_Init(ADC1, &ADC_InitStructure);
	
	// 使能ADC1
	ADC_Cmd(ADC1, ENABLE);
}

/**
 * @brief  获取ADC1在指定通道上的转换值
 * @param  ch: 通道号 (0-16)
 * @retval 12位ADC转换结果 (0-4095)
 */
u16 Get_Adc1(u8 ch)
{
    // 设置规则组通道、采样顺序和采样时间
	// ADC_SampleTime_480Cycles 提供了较长的采样时间，有助于提高稳定性
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_480Cycles);
	
	// 启动软件转换
	ADC_SoftwareStartConv(ADC1);
	
	// 等待转换结束标志位 (EOC)
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	
	// 返回转换结果
	return ADC_GetConversionValue(ADC1);
}
