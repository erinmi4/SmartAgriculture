/**
 * @file    lcd_safe.c
 * @brief   安全的LCD1602驱动实现（修复版本）
 * @author  Mika
 * @date    2025-06-30
 * @version 2.0
 * 
 * @note    使用安全的延时函数，避免SysTick冲突
 */

#include "lcd.h"

/**
 * @brief 简单安全的延时函数
 * @param count: 延时计数
 * @retval 无
 */
static void safe_delay(uint32_t count)
{
    volatile uint32_t i;
    for(i = 0; i < count; i++)
    {
        __NOP();  // 空操作
    }
}

/**
 * @brief 毫秒级安全延时
 * @param ms: 延时毫秒数
 * @retval 无
 */
static void safe_delay_ms(uint32_t ms)
{
    uint32_t i;
    for(i = 0; i < ms; i++)
    {
        safe_delay(168000);  // 假设系统时钟168MHz，调整这个值来校准延时
    }
}

/**
 * @brief 微秒级安全延时
 * @param us: 延时微秒数
 * @retval 无
 */
static void safe_delay_us(uint32_t us)
{
    safe_delay(us * 168);  // 假设系统时钟168MHz
}

/**
 * @brief LCD GPIO引脚初始化
 * @param 无
 * @retval 无
 * @note 初始化所有LCD相关的GPIO引脚为推挽输出模式
 */
void lcd_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 1. 使能所需的GPIO时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | 
                           RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | 
                           RCC_AHB1Periph_GPIOG, ENABLE);
    
    // 2. 配置控制引脚：RS(PB7)、RW(PD6)、EN(PA4)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        
    
    // RS引脚 - PB7
    GPIO_InitStructure.GPIO_Pin = LCD_RS_PIN;
    GPIO_Init(LCD_RS_PORT, &GPIO_InitStructure);
    
    // RW引脚 - PD6
    GPIO_InitStructure.GPIO_Pin = LCD_RW_PIN;
    GPIO_Init(LCD_RW_PORT, &GPIO_InitStructure);
    
    // EN引脚 - PA4
    GPIO_InitStructure.GPIO_Pin = LCD_EN_PIN;
    GPIO_Init(LCD_EN_PORT, &GPIO_InitStructure);
    
    // 3. 配置数据引脚D0-D7
    GPIO_InitStructure.GPIO_Pin = LCD_D0_PIN;
    GPIO_Init(LCD_D0_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LCD_D1_PIN;
    GPIO_Init(LCD_D1_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LCD_D2_PIN;
    GPIO_Init(LCD_D2_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LCD_D3_PIN;
    GPIO_Init(LCD_D3_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LCD_D4_PIN;
    GPIO_Init(LCD_D4_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LCD_D5_PIN;
    GPIO_Init(LCD_D5_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LCD_D6_PIN;
    GPIO_Init(LCD_D6_PORT, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = LCD_D7_PIN;
    GPIO_Init(LCD_D7_PORT, &GPIO_InitStructure);
    
    // 4. 初始化引脚状态
    GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN, Bit_RESET);  
    GPIO_WriteBit(LCD_RW_PORT, LCD_RW_PIN, Bit_RESET);  
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_RESET);  
}

/**
 * @brief 向LCD写入指令
 * @param cmd: 要写入的指令
 * @retval 无
 */
void lcd_write_cmd(unsigned char cmd)
{
    // 1. 设置控制信号
    GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN, Bit_RESET);  // RS = 0
    GPIO_WriteBit(LCD_RW_PORT, LCD_RW_PIN, Bit_RESET);  // RW = 0
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_RESET);  // EN = 0
    
    // 2. 将指令数据送到D0~D7数据线上
    GPIO_WriteBit(LCD_D0_PORT, LCD_D0_PIN, ((cmd >> 0) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D1_PORT, LCD_D1_PIN, ((cmd >> 1) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D2_PORT, LCD_D2_PIN, ((cmd >> 2) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D3_PORT, LCD_D3_PIN, ((cmd >> 3) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D4_PORT, LCD_D4_PIN, ((cmd >> 4) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D5_PORT, LCD_D5_PIN, ((cmd >> 5) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D6_PORT, LCD_D6_PIN, ((cmd >> 6) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D7_PORT, LCD_D7_PIN, ((cmd >> 7) & 0x01) ? Bit_SET : Bit_RESET);
    
    // 3. 延时
    safe_delay_us(2);
    
    // 4. 产生使能脉冲
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_SET);    // EN = 1
    safe_delay_us(2);
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_RESET);  // EN = 0
    
    // 5. 等待执行完成
    safe_delay_us(50);
}

/**
 * @brief 向LCD写入数据
 * @param dat: 要写入的数据
 * @retval 无
 */
void lcd_write_dat(unsigned char dat)
{
    // 1. 设置控制信号
    GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN, Bit_SET);    // RS = 1
    GPIO_WriteBit(LCD_RW_PORT, LCD_RW_PIN, Bit_RESET);  // RW = 0
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_RESET);  // EN = 0
    
    // 2. 将数据送到D0~D7数据线上
    GPIO_WriteBit(LCD_D0_PORT, LCD_D0_PIN, ((dat >> 0) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D1_PORT, LCD_D1_PIN, ((dat >> 1) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D2_PORT, LCD_D2_PIN, ((dat >> 2) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D3_PORT, LCD_D3_PIN, ((dat >> 3) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D4_PORT, LCD_D4_PIN, ((dat >> 4) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D5_PORT, LCD_D5_PIN, ((dat >> 5) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D6_PORT, LCD_D6_PIN, ((dat >> 6) & 0x01) ? Bit_SET : Bit_RESET);
    GPIO_WriteBit(LCD_D7_PORT, LCD_D7_PIN, ((dat >> 7) & 0x01) ? Bit_SET : Bit_RESET);
    
    // 3. 延时
    safe_delay_us(2);
    
    // 4. 产生使能脉冲
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_SET);    // EN = 1
    safe_delay_us(2);
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_RESET);  // EN = 0
    
    // 5. 等待执行完成
    safe_delay_us(50);
}

/**
 * @brief LCD安全初始化
 * @param 无
 * @retval 无
 * @note 使用安全的延时，避免系统冲突
 */
void lcd_init(void)
{
    // 1. GPIO初始化
    lcd_gpio_init();
    
    // 2. 等待LCD上电稳定
    safe_delay_ms(50);  // 增加延时确保稳定
    
    // 3. LCD初始化序列（标准HD44780初始化）
    lcd_write_cmd(0x38);    // 功能设置：8位数据，2行显示，5x7字符
    safe_delay_ms(5);
    
    lcd_write_cmd(0x38);    // 再次发送
    safe_delay_ms(1);
    
    lcd_write_cmd(0x38);    // 第三次发送
    safe_delay_ms(1);
    
    lcd_write_cmd(0x08);    // 显示关闭
    safe_delay_ms(1);
    
    lcd_write_cmd(0x01);    // 清屏
    safe_delay_ms(2);
    
    lcd_write_cmd(0x06);    // 输入模式设置
    safe_delay_ms(1);
    
    lcd_write_cmd(0x0C);    // 显示开，光标关，闪烁关
    safe_delay_ms(1);
}

/**
 * @brief 设置LCD光标位置
 * @param line: 行号 (0-第一行, 1-第二行)
 * @param column: 列号 (0-15)
 * @retval 无
 */
void lcd_gotoxy(unsigned char line, unsigned char column)
{
    unsigned char address;
    
    if (line == 0) {
        address = 0x80 + column;  // 第一行
    } else if (line == 1) {
        address = 0xC0 + column;  // 第二行
    } else {
        return;
    }
    
    if (column > 15) {
        return;
    }
    
    lcd_write_cmd(address);
}

/**
 * @brief 在LCD上显示字符串
 * @param str: 要显示的字符串指针
 * @retval 无
 */
void lcd_print_str(unsigned char *str)
{
    while (*str != '\0') {
        lcd_write_dat(*str);
        str++;
    }
}

/**
 * @brief 在LCD上显示单个字符
 * @param ch: 要显示的字符
 * @retval 无
 */
void lcd_print_char(unsigned char ch)
{
    lcd_write_dat(ch);
}

/**
 * @brief 清除LCD显示内容
 * @param 无
 * @retval 无
 */
void lcd_clear(void)
{
    lcd_write_cmd(0x01);  // 清屏指令
    safe_delay_ms(2);     // 等待清屏完成
}
