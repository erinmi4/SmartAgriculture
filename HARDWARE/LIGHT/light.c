#include "light.h"
#include "adc1.h" // <--- 已修改: 包含 adc1.h 而不是 ADC3.h
#include "delay.h"

/**
 * @brief  光敏电阻初始化
 * @param  None
 * @retval None
 * @note   初始化正确的GPIO(PB1)和ADC1
 */
void Light_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能GPIOB时钟
    RCC_AHB1PeriphClockCmd(LIGHT_GPIO_CLK, ENABLE);
    
    // 配置PB1为模拟输入模式
    GPIO_InitStructure.GPIO_Pin = LIGHT_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;        // 模拟输入模式
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;    // 不上拉不下拉
    GPIO_Init(LIGHT_GPIO_PORT, &GPIO_InitStructure);
    
    // 初始化ADC1
    Adc1_Init(); // <--- 已修改: 调用 Adc1_Init()
    
    // 短暂延时等待ADC稳定
    Mdelay_Lib(20);
}

/**
 * @brief  获取光敏电阻原始ADC值(单次采样)
 * @param  None
 * @retval 12位ADC转换结果 (0-4095)
 */
uint16_t Light_GetRawValue(void)
{
    // <--- 已修改: 调用 Get_Adc1()
    return Get_Adc1(LIGHT_ADC_CHANNEL);
}

/**
 * @brief  获取光敏电阻值(多次采样平均，0-100范围)
 * @param  None
 * @retval 光照强度值 (0-100，0最暗，100最亮)
 */
uint8_t Light_GetValue(void)
{
    uint32_t temp_val = 0;
    uint8_t t;
    
    // 多次采样取平均值，提高稳定性
    for(t = 0; t < LSENS_READ_TIMES; t++)
    {
        temp_val += Get_Adc1(LIGHT_ADC_CHANNEL); // <--- 已修改: 调用 Get_Adc1()
        Mdelay_Lib(5);  // 每次采样间隔5ms
    }
    
    temp_val = temp_val / LSENS_READ_TIMES;  // 得到平均值
    
    // --- 已修改: 增加安全判断，防止计算错误 ---
    // 检查ADC值是否在有效范围内
    if(temp_val > 4095) temp_val = 4095;

    // 线性映射公式：light = 100 - (adc * 100 / 4095)
    // 光敏电阻特性：光照强 -> 电阻小 -> ADC值小 -> 光照强度高
    // 光照弱 -> 电阻大 -> ADC值大 -> 光照强度低
    uint32_t mapped_val = temp_val * 100 / 4095;
    
    uint8_t light_percent;
    if (mapped_val >= 100)
    {
        light_percent = 0; // ADC值很大(黑暗)，亮度为0
    }
    else
    {
        light_percent = 100 - mapped_val;
    }
    
    return light_percent;
}

/**
 * @brief  获取光照强度等级
 * @param  None
 * @retval 光照强度等级枚举值
 */
LightLevel_t Light_GetLevel(void)
{
    uint8_t light_value = Light_GetValue();
    
    // 基于0-100范围划分等级
    if(light_value <= 20)
        return LIGHT_LEVEL_DARK;
    else if(light_value <= 40)
        return LIGHT_LEVEL_DIM;
    else if(light_value <= 60)
        return LIGHT_LEVEL_NORMAL;
    else if(light_value <= 80)
        return LIGHT_LEVEL_BRIGHT;
    else
        return LIGHT_LEVEL_VERY_BRIGHT;
}

/**
 * @brief  获取光照强度等级字符串描述
 * @param  level: 光照强度等级
 * @retval 等级描述字符串
 */
const char* Light_GetLevelString(LightLevel_t level)
{
    switch(level)
    {
        case LIGHT_LEVEL_DARK:
            return "Dark";
        case LIGHT_LEVEL_DIM:
            return "Dim";
        case LIGHT_LEVEL_NORMAL:
            return "Normal";
        case LIGHT_LEVEL_BRIGHT:
            return "Bright";
        case LIGHT_LEVEL_VERY_BRIGHT:
            return "V.Bright";
        default:
            return "Unknown";
    }
}
