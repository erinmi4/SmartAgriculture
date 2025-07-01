#include "stm32f4xx.h"
#include "ADC3.h"
#include "sys.h"
 
void Adc3_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3,ENABLE); //使能ADC3时钟
	
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3,ENABLE);  //ADC3复位
	RCC_APB2PeriphResetCmd(RCC_APB2Periph_ADC3,DISABLE);   //复位结束
	
	//初始化CCR寄存器
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	ADC_CommonInitStructure.ADC_DMAAccessMode=ADC_DMAAccessMode_Disabled; //DMA不使能，DMA通常用于多通道的转移
	ADC_CommonInitStructure.ADC_Mode=ADC_Mode_Independent;  //独立模式
	ADC_CommonInitStructure.ADC_Prescaler=ADC_Prescaler_Div4;  //预分频4分频
	ADC_CommonInitStructure.ADC_TwoSamplingDelay=ADC_TwoSamplingDelay_5Cycles;  //两个采样阶段之间延迟5个时钟
	ADC_CommonInit(&ADC_CommonInitStructure);
	
	//初始化ADC
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;  //关闭连续转换
	ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;  //右对齐
	ADC_InitStructure.ADC_ExternalTrigConvEdge=ADC_ExternalTrigConvEdge_None;//禁止触发检测，使用软件触发
	ADC_InitStructure.ADC_NbrOfConversion=1; //1个转换在规则序列中
	ADC_InitStructure.ADC_Resolution=ADC_Resolution_12b;  //12位模式分辨率
	ADC_InitStructure.ADC_ScanConvMode=DISABLE;  //非扫描模式
	ADC_Init(ADC3,&ADC_InitStructure);
	
	ADC_Cmd(ADC3,ENABLE);  //开启AD转换器
}
//获得ADC的值
//ch：通道值0~16，直接传入通道号 
//返回值：转换的结果
u16 Get_Adc3(u8 ch)
{
    u32 adc_channel;
    
    // 将通道号转换为ADC_Channel_x格式
    switch(ch)
    {
        case 0: adc_channel = ADC_Channel_0; break;
        case 1: adc_channel = ADC_Channel_1; break;
        case 2: adc_channel = ADC_Channel_2; break;
        case 3: adc_channel = ADC_Channel_3; break;
        case 4: adc_channel = ADC_Channel_4; break;
        case 5: adc_channel = ADC_Channel_5; break;
        case 6: adc_channel = ADC_Channel_6; break;
        case 7: adc_channel = ADC_Channel_7; break;
        case 8: adc_channel = ADC_Channel_8; break;
        case 9: adc_channel = ADC_Channel_9; break;
        case 10: adc_channel = ADC_Channel_10; break;
        case 11: adc_channel = ADC_Channel_11; break;
        case 12: adc_channel = ADC_Channel_12; break;
        case 13: adc_channel = ADC_Channel_13; break;
        case 14: adc_channel = ADC_Channel_14; break;
        case 15: adc_channel = ADC_Channel_15; break;
        case 16: adc_channel = ADC_Channel_16; break;
        default: return 0; // 无效通道返回0
    }
    
	ADC_RegularChannelConfig(ADC3, adc_channel, 1, ADC_SampleTime_480Cycles); //设置ADC规则组通道，1个序列 采样时间
	ADC_SoftwareStartConv(ADC3);//使能指定的ADC3的软件转换启动功能
	while(!ADC_GetFlagStatus(ADC3,ADC_FLAG_EOC));//等待状态寄存器转换标志位结束
	return ADC_GetConversionValue(ADC3);   //返回转换的结果
}








