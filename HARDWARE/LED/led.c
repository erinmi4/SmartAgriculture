#include "stm32f4xx.h"
#include "led.h"
#include "key.h"
#include "beep.h"

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

    // 配置PE13为推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 配置PE14为推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    // 点亮所有LED灯
    GPIO_SetBits(GPIOF, GPIO_Pin_9 | GPIO_Pin_10);
    GPIO_SetBits(GPIOE, GPIO_Pin_13 | GPIO_Pin_14);
}

// 打开指定的LED
void Led_On(uint16_t Led_Num)
{
    switch(Led_Num)
    {
        case 0: // LED1/LED0 (PF9)
            GPIO_ResetBits(LED0_GPIO, LED0_PIN);
            break;
        
        case 1: // LED2 (PF10)
            GPIO_ResetBits(LED2_GPIO, LED2_PIN);
            break;
        
        case 2: // LED3 (PE13)
            GPIO_ResetBits(LED3_GPIO, LED3_PIN);
            break;
        
        case 3: // LED3 (PE14)
            GPIO_ResetBits(LED3_GPIO, LED3_PIN);
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
        case 0: // LED0 (PF9)
            GPIO_SetBits(LED0_GPIO, LED0_PIN);
            break;
        
        case 1: // LED1 (PF10)
            GPIO_SetBits(LED1_GPIO, LED1_PIN);
            break;

        case 2: // LED2 (PE13)
            GPIO_SetBits(LED2_GPIO, LED2_PIN);
            break;
        
        case 3: // LED3 (PE14)
            GPIO_SetBits(LED3_GPIO, LED3_PIN);
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
    
    if(led_state[Led_Num])
        GPIO_ResetBits(Led_Num == 0 || Led_Num == 1 ? GPIOF : GPIOE, 
                      Led_Num == 0 ? GPIO_Pin_9 : 
                      Led_Num == 1 ? GPIO_Pin_10 : 
                      Led_Num == 2 ? GPIO_Pin_13 : GPIO_Pin_14);
    else
        GPIO_SetBits(Led_Num == 0 || Led_Num == 1 ? GPIOF : GPIOE, 
                    Led_Num == 0 ? GPIO_Pin_9 : 
                    Led_Num == 1 ? GPIO_Pin_10 : 
                    Led_Num == 2 ? GPIO_Pin_13 : GPIO_Pin_14);
}

/* =================== 呼吸灯功能实现 =================== */

// 定义PWM相关参数
#define PWM_PERIOD 1000    // PWM周期
static TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
static TIM_OCInitTypeDef  TIM_OCInitStructure;
static uint8_t breathEnabled[2] = {0, 0}; // LED2和LED3的呼吸灯启用状态
static uint16_t breathValue[2] = {0, 0};  // 当前亮度值
static uint8_t breathDir[2] = {1, 1};     // 方向：1-变亮，0-变暗

// 初始化TIM1用于PWM输出
void LED_Breath_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    // 配置GPIO为复用功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    
    // 连接TIM1 Channel3和Channel4到PE13和PE14
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13, GPIO_AF_TIM1);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14, GPIO_AF_TIM1);
    
    // 配置TIM1基本参数
    TIM_TimeBaseStructure.TIM_Period = PWM_PERIOD - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 84-1; // 1MHz计数频率
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
    
    // 配置TIM1输出通道
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_Pulse = 0; // 初始占空比为0
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; // 低电平有效
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
    
    // 配置通道3（PE13/LED2）
    TIM_OC3Init(TIM1, &TIM_OCInitStructure);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    
    // 配置通道4（PE14/LED3）
    TIM_OC4Init(TIM1, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
    
    // 使能TIM1
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
    
    // 必须使能主输出，因为TIM1是高级定时器
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    
    // 初始状态：禁用呼吸灯效果
    breathEnabled[0] = breathEnabled[1] = 0;
}

// 启用指定LED的呼吸灯效果
void LED_Breath_Enable(uint16_t Led_Num)
{
    // 只支持LED2和LED3的呼吸灯功能
    if(Led_Num == LED2)
        breathEnabled[0] = 1;
    else if(Led_Num == LED3)
        breathEnabled[1] = 1;
}

// 禁用指定LED的呼吸灯效果
void LED_Breath_Disable(uint16_t Led_Num)
{
    if(Led_Num == LED2) {
        breathEnabled[0] = 0;
        TIM_SetCompare3(TIM1, 0); // 关闭LED
    } else if(Led_Num == LED3) {
        breathEnabled[1] = 0;
        TIM_SetCompare4(TIM1, 0); // 关闭LED
    }
}

// 处理呼吸灯效果，应在主循环中定期调用
void LED_Breath_Process(void)
{
    static uint32_t lastBreathTime = 0;
    extern uint32_t system_tick; // 从main.c引入系统时钟计数器
    
    // 每10ms更新一次亮度
    if(system_tick - lastBreathTime >= 10) {
        lastBreathTime = system_tick;
        
        // 处理LED2呼吸效果
        if(breathEnabled[0]) {
            if(breathDir[0]) { // 变亮
                breathValue[0] += 5;
                if(breathValue[0] >= PWM_PERIOD) {
                    breathValue[0] = PWM_PERIOD;
                    breathDir[0] = 0; // 变暗
                }
            } else { // 变暗
                if(breathValue[0] > 5)
                    breathValue[0] -= 5;
                else {
                    breathValue[0] = 0;
                    breathDir[0] = 1; // 变亮
                }
            }
            TIM_SetCompare3(TIM1, breathValue[0]);
        }
        
        // 处理LED3呼吸效果
        if(breathEnabled[1]) {
            if(breathDir[1]) { // 变亮
                breathValue[1] += 5;
                if(breathValue[1] >= PWM_PERIOD) {
                    breathValue[1] = PWM_PERIOD;
                    breathDir[1] = 0; // 变暗
                }
            } else { // 变暗
                if(breathValue[1] > 5)
                    breathValue[1] -= 5;
                else {
                    breathValue[1] = 0;
                    breathDir[1] = 1; // 变亮
                }
            }
            TIM_SetCompare4(TIM1, breathValue[1]);
        }
    }
}
