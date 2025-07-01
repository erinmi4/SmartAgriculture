/**
 * @file    pf7_gpio_test.c
 * @brief   PF7引脚电平测试
 * @details 将PF7配置为数字输入，检查引脚电平状态
 */

#include "stm32f4xx.h"
#include "sys.h"
#include "delay.h"
#include "lcd.h"
#include <stdio.h>

int main(void)
{
    // 系统初始化
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    
    // 初始化LCD1602
    lcd_init();
    lcd_clear();
    lcd_print_str(0, 0, "PF7 GPIO Test");
    lcd_print_str(1, 0, "Starting...");
    Mdelay_Lib(2000);
    
    // 配置PF7为数字输入（带内部下拉）
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 测试1：配置为浮空输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    lcd_clear();
    lcd_print_str(0, 0, "PF7 Float Test");
    
    for(int i = 0; i < 10; i++)
    {
        uint8_t pin_state = GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_7);
        char str[16];
        sprintf(str, "State: %d", pin_state);
        lcd_print_str(1, 0, str);
        Mdelay_Lib(1000);
    }
    
    // 测试2：配置为下拉输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    lcd_clear();
    lcd_print_str(0, 0, "PF7 PullDown");
    
    for(int i = 0; i < 10; i++)
    {
        uint8_t pin_state = GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_7);
        char str[16];
        sprintf(str, "State: %d", pin_state);
        lcd_print_str(1, 0, str);
        Mdelay_Lib(1000);
    }
    
    // 测试3：配置为上拉输入
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    
    lcd_clear();
    lcd_print_str(0, 0, "PF7 PullUp");
    
    while(1)
    {
        uint8_t pin_state = GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_7);
        char str[16];
        sprintf(str, "State: %d", pin_state);
        lcd_print_str(1, 0, str);
        Mdelay_Lib(500);
    }
}
