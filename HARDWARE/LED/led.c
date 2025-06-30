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
            GPIO_ResetBits(LED1_GPIO, LED1_PIN);
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




