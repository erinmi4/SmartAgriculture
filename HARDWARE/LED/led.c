#include "stm32f4xx.h"
#include "led.h"
#include "key.h"
#include "beep.h"

// 全局变量记录呼吸灯状态
static uint8_t breathing_enabled = 0;

// 初始化LED灯
void Led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能GPIOF和GPIOE的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOE, ENABLE);

    // 配置PF9为推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    // 配置PF10为推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    // 配置PE13为推挽输出模式 (普通GPIO模式)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 配置PE14为推挽输出模式 (普通GPIO模式)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 默认关闭所有LED灯
    GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
    GPIO_SetBits(GPIOE, GPIO_Pin_13 | GPIO_Pin_14);
}

/**
 * @brief 初始化呼吸灯PWM功能 (TIM1_CH3和TIM1_CH4)
 */
void Led_BreathingInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    // 使能 GPIOE 和 TIM1 时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    // 配置 PE13 为复用功能 (TIM1_CH3)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 配置 PE14 为复用功能 (TIM1_CH4)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 将 PE13 连接到 TIM1_CH3
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1);
    // 将 PE14 连接到 TIM1_CH4
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);

    // 定时器 TIM1 基本配置
    TIM_TimeBaseStructure.TIM_Period = 999; // 定时器周期 (1000级亮度)
    TIM_TimeBaseStructure.TIM_Prescaler = 83; // 预分频器 (84MHz/84=1MHz)
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    // 配置 TIM1_CH3 为 PWM 模式 (LED2)
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比为 0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; // 低电平有效(LED接地控制)
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);

    // 配置 TIM1_CH4 为 PWM 模式 (LED3)
    TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比为 0
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);

    // 使能 TIM1 主输出
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    // 使能 TIM1
    TIM_Cmd(TIM1, ENABLE);
    
    breathing_enabled = 1;
}

/**
 * @brief 设置呼吸灯亮度
 * @param led_num: LED编号 (2或3)
 * @param brightness: 亮度值 (0-999, 0最暗, 999最亮)
 */
void Led_SetBreathing(uint16_t led_num, uint16_t brightness)
{
    if(!breathing_enabled) return;
    
    if(brightness > 999) brightness = 999;
    
    if(led_num == LED2) {
        TIM_SetCompare3(TIM1, brightness);
    }
    else if(led_num == LED3) {
        TIM_SetCompare4(TIM1, brightness);
    }
}

/**
 * @brief 呼吸灯效果 (单次呼吸循环)
 * @param led_num: LED编号 (2或3)
 */
void Led_BreathingEffect(uint16_t led_num)
{
    if(!breathing_enabled) return;
    
    static uint16_t brightness[2] = {0, 0}; // LED2和LED3的亮度
    static int8_t direction[2] = {1, 1};    // 亮度变化方向
    static uint32_t last_update[2] = {0, 0}; // 上次更新时间
    
    extern uint32_t system_tick;
    
    uint8_t idx = (led_num == LED2) ? 0 : 1;
    
    // 每10ms更新一次亮度
    if(system_tick - last_update[idx] >= 10) {
        last_update[idx] = system_tick;
        
        // 更新亮度
        brightness[idx] += direction[idx] * 5; // 每次增减5
        
        // 检查边界并改变方向
        if(brightness[idx] >= 999) {
            brightness[idx] = 999;
            direction[idx] = -1;
        }
        else if(brightness[idx] <= 0) {
            brightness[idx] = 0;
            direction[idx] = 1;
        }
        
        // 设置新亮度
        Led_SetBreathing(led_num, brightness[idx]);
    }
}

/**
 * @brief 停止呼吸灯，恢复普通GPIO模式
 * @param led_num: LED编号 (2或3)
 */
void Led_StopBreathing(uint16_t led_num)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    if(led_num == LED2) {
        // 停止PWM输出
        TIM_SetCompare3(TIM1, 0);
        
        // 重新配置为普通GPIO输出
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
        
        // 关闭LED
        GPIO_SetBits(GPIOE, GPIO_Pin_13);
    }
    else if(led_num == LED3) {
        // 停止PWM输出
        TIM_SetCompare4(TIM1, 0);
        
        // 重新配置为普通GPIO输出
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOE, &GPIO_InitStructure);
        
        // 关闭LED
        GPIO_SetBits(GPIOE, GPIO_Pin_14);
    }
}

// 打开指定的LED
void Led_On(uint16_t Led_Num)
{
    switch(Led_Num)
    {
        case 0: // LED0 (PF9) - 烟雾
            GPIO_ResetBits(LED0_GPIO, LED0_PIN);
            break;
        
        case 1: // LED1 (PF10) - 温度
            GPIO_ResetBits(LED1_GPIO, LED1_PIN);
            break;
        
        case 2: // LED2 (PE13) - 湿度
            if(breathing_enabled) {
                Led_SetBreathing(LED2, 999); // 最亮
            } else {
                GPIO_ResetBits(LED2_GPIO, LED2_PIN);
            }
            break;
        
        case 3: // LED3 (PE14) - 光照
            if(breathing_enabled) {
                Led_SetBreathing(LED3, 999); // 最亮
            } else {
                GPIO_ResetBits(LED3_GPIO, LED3_PIN);
            }
            break;
        
        default:
            break;
    }
}

// 关闭指定的LED
void Led_Off(uint16_t Led_Num)
{
    switch(Led_Num)
    {
        case 0: // LED0 (PF9) - 烟雾
            GPIO_SetBits(LED0_GPIO, LED0_PIN);
            break;
        
        case 1: // LED1 (PF10) - 温度
            GPIO_SetBits(LED1_GPIO, LED1_PIN);
            break;

        case 2: // LED2 (PE13) - 湿度
            if(breathing_enabled) {
                Led_SetBreathing(LED2, 0); // 最暗
            } else {
                GPIO_SetBits(LED2_GPIO, LED2_PIN);
            }
            break;
        
        case 3: // LED3 (PE14) - 光照
            if(breathing_enabled) {
                Led_SetBreathing(LED3, 0); // 最暗
            } else {
                GPIO_SetBits(LED3_GPIO, LED3_PIN);
            }
            break;
        
        default:
            break;
    }
}

// LED翻转函数
void Led_Toggle(uint16_t Led_Num)
{
    static uint8_t led_state[4] = {0};
    
    led_state[Led_Num] = !led_state[Led_Num];
    
    if(led_state[Led_Num]) {
        Led_On(Led_Num);
    } else {
        Led_Off(Led_Num);
    }
}






