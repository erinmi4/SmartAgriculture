#include "beep.h"
#include "delay.h"

// 初始化蜂鸣器
void Beep_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 使能GPIOF的时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    // 配置BEEP (PF8) 为推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    // 默认关闭蜂鸣器
    GPIO_ResetBits(BEEP_GPIO, BEEP_PIN);
}

// 打开蜂鸣器
void Beep_On(void)
{
    GPIO_SetBits(BEEP_GPIO, BEEP_PIN);
}

// 关闭蜂鸣器
void Beep_Off(void)
{
    GPIO_ResetBits(BEEP_GPIO, BEEP_PIN);
}

/**
 * @brief 蜂鸣器发出指定频率和持续时间的音调
 * @param freq: 频率 (Hz), 0表示静音
 * @param duration_ms: 持续时间 (毫秒)
 */
void Beep_Tone(uint32_t freq, uint32_t duration_ms) 
{
    if (freq == 0) {
        // 静音
        GPIO_ResetBits(BEEP_GPIO, BEEP_PIN);
        Mdelay_Lib(duration_ms);
        return;
    }
    
    // 计算半周期时间 (单位: 微秒)
    uint32_t half_period_us = 500000 / freq;
    uint32_t cycles = (duration_ms * 1000) / (2 * half_period_us);
    
    for (uint32_t i = 0; i < cycles; i++) {
        GPIO_SetBits(BEEP_GPIO, BEEP_PIN);
        Udelay_Lib(half_period_us);
        GPIO_ResetBits(BEEP_GPIO, BEEP_PIN);
        Udelay_Lib(half_period_us);
    }
}

/**
 * @brief 播放报警音乐 - 简单的警示音
 */
void Beep_PlayAlarm(void)
{
    // 播放一个简单的报警音调: Do-Sol-Do-Sol
    Beep_Tone(DO, 200);   // Do 0.2秒
    Beep_Tone(0, 50);     // 静音 0.05秒

    Beep_Tone(DO, 200);   // Do 0.2秒
    Beep_Tone(0, 50);     // 静音 0.05秒
    
    Beep_Tone(SOL, 200);  // Sol 0.2秒
    Beep_Tone(0, 50);     // 静音 0.05秒

    Beep_Tone(SOL, 200);  // Sol 0.2秒
    Beep_Tone(0, 50);     // 静音 0.05秒

    Beep_Tone(LA, 300); // 高音Do 0.3秒
    Beep_Tone(0, 100);    // 静音 0.1秒

    Beep_Tone(LA, 300); // 高音Do 0.3秒
    Beep_Tone(0, 100);    // 静音 0.1秒

    Beep_Tone(SOL, 200);  // Sol 0.2秒
    Beep_Tone(0, 50);     // 静音 0.05秒

}

/**
 * @brief 持续报警音 - 新增函数，专门用于持续报警
 */
void Beep_ContinuousAlarm(void)
{
    static uint32_t alarm_step = 0;
    static uint32_t last_step_time = 0;
    extern uint32_t system_tick;  // 使用系统时钟
    
    // 每200ms切换一次状态，形成连续的报警音
    if(system_tick - last_step_time >= 200)
    {
        last_step_time = system_tick;
        
        switch(alarm_step % 4)
        {
            case 0:
            case 2:
                GPIO_SetBits(BEEP_GPIO, BEEP_PIN);  // 蜂鸣器响
                break;
            case 1:
            case 3:
                GPIO_ResetBits(BEEP_GPIO, BEEP_PIN); // 蜂鸣器停
                break;
        }
        alarm_step++;
    }
}

