#ifndef __LIGHT_H
#define __LIGHT_H

#include "stm32f4xx.h"

/* * 光敏电阻ADC通道定义 (已更正)
 * 根据GEC-M4原理图，光敏传感器连接到 PF7 -> ADC3_IN5
 */
#define LIGHT_ADC_CHANNEL       5 // PF7 -> ADC3_IN5
#define LIGHT_GPIO_PORT         GPIOF               // 使用 GPIOF
#define LIGHT_GPIO_PIN          GPIO_Pin_7          // 使用 Pin 7
#define LIGHT_GPIO_CLK          RCC_AHB1Periph_GPIOF

/* 光敏电阻读取次数定义 */
#define LSENS_READ_TIMES        5              // 多次采样取平均值，增加稳定性

/* 光照强度等级定义 (基于0-100范围) */
typedef enum
{
    LIGHT_LEVEL_DARK = 0,      // 黑暗 (0-20)
    LIGHT_LEVEL_DIM,           // 昏暗 (21-40)
    LIGHT_LEVEL_NORMAL,        // 正常 (41-60)
    LIGHT_LEVEL_BRIGHT,        // 明亮 (61-80)
    LIGHT_LEVEL_VERY_BRIGHT    // 很亮 (81-100)
} LightLevel_t;

/* 函数声明 */
void Light_Init(void);
uint16_t Light_GetRawValue(void);
uint8_t Light_GetValue(void);               // 获取0-100范围的光照强度值
LightLevel_t Light_GetLevel(void);
const char* Light_GetLevelString(LightLevel_t level);

#endif /* __LIGHT_H */
