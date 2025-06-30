#include "lcd.h"
#include "delay.h"

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
    // GPIOA、GPIOB、GPIOC、GPIOD、GPIOG
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | 
                           RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | 
                           RCC_AHB1Periph_GPIOG, ENABLE);
    
    // 2. 配置控制引脚：RS(PB7)、RW(PD6)、EN(PA4)
    // 配置RS引脚 - PB7
    GPIO_InitStructure.GPIO_Pin = LCD_RS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;       // 输出模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // 输出速度50MHz
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        // 上拉
    GPIO_Init(LCD_RS_PORT, &GPIO_InitStructure);
    
    // 配置RW引脚 - PD6
    GPIO_InitStructure.GPIO_Pin = LCD_RW_PIN;
    GPIO_Init(LCD_RW_PORT, &GPIO_InitStructure);
    
    // 配置EN引脚 - PA4
    GPIO_InitStructure.GPIO_Pin = LCD_EN_PIN;
    GPIO_Init(LCD_EN_PORT, &GPIO_InitStructure);
    
    // 3. 配置数据引脚D0-D7
    // D0 - PD7
    GPIO_InitStructure.GPIO_Pin = LCD_D0_PIN;
    GPIO_Init(LCD_D0_PORT, &GPIO_InitStructure);
    
    // D1 - PG15
    GPIO_InitStructure.GPIO_Pin = LCD_D1_PIN;
    GPIO_Init(LCD_D1_PORT, &GPIO_InitStructure);
    
    // D2 - PC6
    GPIO_InitStructure.GPIO_Pin = LCD_D2_PIN;
    GPIO_Init(LCD_D2_PORT, &GPIO_InitStructure);
    
    // D3 - PC7
    GPIO_InitStructure.GPIO_Pin = LCD_D3_PIN;
    GPIO_Init(LCD_D3_PORT, &GPIO_InitStructure);
    
    // D4 - PC8
    GPIO_InitStructure.GPIO_Pin = LCD_D4_PIN;
    GPIO_Init(LCD_D4_PORT, &GPIO_InitStructure);
    
    // D5 - PC9
    GPIO_InitStructure.GPIO_Pin = LCD_D5_PIN;
    GPIO_Init(LCD_D5_PORT, &GPIO_InitStructure);
    
    // D6 - PC11
    GPIO_InitStructure.GPIO_Pin = LCD_D6_PIN;
    GPIO_Init(LCD_D6_PORT, &GPIO_InitStructure);
    
    // D7 - PB6
    GPIO_InitStructure.GPIO_Pin = LCD_D7_PIN;
    GPIO_Init(LCD_D7_PORT, &GPIO_InitStructure);
    
    // 4. 初始化引脚状态
    GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN, Bit_RESET);  // RS = 0
    GPIO_WriteBit(LCD_RW_PORT, LCD_RW_PIN, Bit_RESET);  // RW = 0 (写模式)
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_RESET);  // EN = 0
}

/**
 * @brief 向LCD写入指令
 * @param cmd: 要写入的指令
 * @retval 无
 * @note RS=0表示写指令，RW=0表示写操作
 */
void lcd_write_cmd(unsigned char cmd)
{
    // 1. 设置控制信号：RS=0(指令), RW=0(写), EN=0
    GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN, Bit_RESET);  // RS = 0，选择指令寄存器
    GPIO_WriteBit(LCD_RW_PORT, LCD_RW_PIN, Bit_RESET);  // RW = 0，写操作
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_RESET);  // EN = 0
    
    // 2. 将指令数据送到D0~D7数据线上
    GPIO_WriteBit(LCD_D0_PORT, LCD_D0_PIN, ((cmd >> 0) & 0x01) ? Bit_SET : Bit_RESET);  // D0
    GPIO_WriteBit(LCD_D1_PORT, LCD_D1_PIN, ((cmd >> 1) & 0x01) ? Bit_SET : Bit_RESET);  // D1
    GPIO_WriteBit(LCD_D2_PORT, LCD_D2_PIN, ((cmd >> 2) & 0x01) ? Bit_SET : Bit_RESET);  // D2
    GPIO_WriteBit(LCD_D3_PORT, LCD_D3_PIN, ((cmd >> 3) & 0x01) ? Bit_SET : Bit_RESET);  // D3
    GPIO_WriteBit(LCD_D4_PORT, LCD_D4_PIN, ((cmd >> 4) & 0x01) ? Bit_SET : Bit_RESET);  // D4
    GPIO_WriteBit(LCD_D5_PORT, LCD_D5_PIN, ((cmd >> 5) & 0x01) ? Bit_SET : Bit_RESET);  // D5
    GPIO_WriteBit(LCD_D6_PORT, LCD_D6_PIN, ((cmd >> 6) & 0x01) ? Bit_SET : Bit_RESET);  // D6
    GPIO_WriteBit(LCD_D7_PORT, LCD_D7_PIN, ((cmd >> 7) & 0x01) ? Bit_SET : Bit_RESET);  // D7
    
    // 3. 延时，让LCD准备接收数据
    Udelay_Lib(2);
    
    // 4. 将EN拉高，产生上升沿，LCD开始读取指令
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_SET);   // EN = 1
    
    // 5. 延时，保持EN高电平一段时间
    Udelay_Lib(2);
    
    // 6. 将EN拉低，产生下降沿，LCD完成指令读取
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_RESET); // EN = 0
    
    // 7. 延时，等待LCD执行指令
    Udelay_Lib(50);  // 大部分指令需要37us执行时间
}

/**
 * @brief 向LCD写入数据
 * @param dat: 要写入的数据
 * @retval 无
 * @note RS=1表示写数据，RW=0表示写操作
 */
void lcd_write_dat(unsigned char dat)
{
    // 1. 设置控制信号：RS=1(数据), RW=0(写), EN=0
    GPIO_WriteBit(LCD_RS_PORT, LCD_RS_PIN, Bit_SET);    // RS = 1，选择数据寄存器
    GPIO_WriteBit(LCD_RW_PORT, LCD_RW_PIN, Bit_RESET);  // RW = 0，写操作
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_RESET);  // EN = 0
    
    // 2. 将数据送到D0~D7数据线上
    GPIO_WriteBit(LCD_D0_PORT, LCD_D0_PIN, ((dat >> 0) & 0x01) ? Bit_SET : Bit_RESET);  // D0
    GPIO_WriteBit(LCD_D1_PORT, LCD_D1_PIN, ((dat >> 1) & 0x01) ? Bit_SET : Bit_RESET);  // D1
    GPIO_WriteBit(LCD_D2_PORT, LCD_D2_PIN, ((dat >> 2) & 0x01) ? Bit_SET : Bit_RESET);  // D2
    GPIO_WriteBit(LCD_D3_PORT, LCD_D3_PIN, ((dat >> 3) & 0x01) ? Bit_SET : Bit_RESET);  // D3
    GPIO_WriteBit(LCD_D4_PORT, LCD_D4_PIN, ((dat >> 4) & 0x01) ? Bit_SET : Bit_RESET);  // D4
    GPIO_WriteBit(LCD_D5_PORT, LCD_D5_PIN, ((dat >> 5) & 0x01) ? Bit_SET : Bit_RESET);  // D5
    GPIO_WriteBit(LCD_D6_PORT, LCD_D6_PIN, ((dat >> 6) & 0x01) ? Bit_SET : Bit_RESET);  // D6
    GPIO_WriteBit(LCD_D7_PORT, LCD_D7_PIN, ((dat >> 7) & 0x01) ? Bit_SET : Bit_RESET);  // D7
    
    // 3. 延时，让LCD准备接收数据
    Udelay_Lib(2);
    
    // 4. 将EN拉高，产生上升沿，LCD开始读取数据
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_SET);   // EN = 1
    
    // 5. 延时，保持EN高电平一段时间
    Udelay_Lib(2);
    
    // 6. 将EN拉低，产生下降沿，LCD完成数据读取
    GPIO_WriteBit(LCD_EN_PORT, LCD_EN_PIN, Bit_RESET); // EN = 0
    
    // 7. 延时，等待LCD处理数据
    Udelay_Lib(50);
}

/**
 * @brief LCD初始化
 * @param 无
 * @retval 无
 * @note 按照HD44780标准初始化序列进行初始化
 */
void lcd_init(void)
{
    // 1. GPIO初始化
    lcd_gpio_init();
    
    // 2. 等待LCD上电稳定
    Mdelay_Lib(15);  // 等待LCD上电稳定，至少15ms
    
    // 3. LCD初始化序列
    lcd_write_cmd(LCD_CMD_FUNCTION_SET);    // 0x38: 8位数据线，2行显示，5x7字符
    Mdelay_Lib(5);                            // 等待4.1ms以上
    
    lcd_write_cmd(LCD_CMD_FUNCTION_SET);    // 再次发送功能设置指令
    Udelay_Lib(100);                          // 等待100us以上
    
    lcd_write_cmd(LCD_CMD_FUNCTION_SET);    // 第三次发送功能设置指令
    
    lcd_write_cmd(LCD_CMD_DISPLAY_CTRL);    // 0x0C: 显示开，光标关，闪烁关
    lcd_write_cmd(LCD_CMD_CLEAR);           // 0x01: 清屏
    Mdelay_Lib(2);                            // 清屏指令需要1.64ms执行时间
    
    lcd_write_cmd(LCD_CMD_ENTRY_MODE);      // 0x06: 输入模式设置，光标右移，显示器不移动
}

/**
 * @brief 设置LCD光标位置
 * @param line: 行号 (0-第一行, 1-第二行)
 * @param column: 列号 (0-15)
 * @retval 无
 * @note LCD1602有2行，每行16个字符
 */
void lcd_gotoxy(unsigned char line, unsigned char column)
{
    unsigned char address;
    
    if (line == 0) {
        // 第一行，起始地址为0x00
        address = LCD_LINE1_ADDR + column;  // 0x80 + column
    } else if (line == 1) {
        // 第二行，起始地址为0x40
        address = LCD_LINE2_ADDR + column;  // 0xC0 + column
    } else {
        return; // 无效行号
    }
    
    // 限制列号范围
    if (column > 15) {
        return; // 无效列号
    }
    
    lcd_write_cmd(address);  // 设置DDRAM地址
}

/**
 * @brief 在LCD上显示字符串
 * @param str: 要显示的字符串指针
 * @retval 无
 * @note 字符串以'\0'结尾
 */
void lcd_print_str(unsigned char *str)
{
    while (*str != '\0') {
        lcd_write_dat(*str);  // 发送字符数据
        str++;                // 指向下一个字符
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
    lcd_write_cmd(LCD_CMD_CLEAR);  // 发送清屏指令
    Mdelay_Lib(2);                   // 等待清屏完成
}
