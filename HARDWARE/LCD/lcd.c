#include "lcd.h"
#include "delay.h"     

// 初始化 LCD1602 的 GPIO 引脚
void lcd_gpio_init() {
    // 使能时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG, ENABLE);

    // 将对应引脚配置为推挽输出
    GPIO_InitTypeDef GPIO_InitStructure;

    // 初始化控制引脚
    GPIO_InitStructure.GPIO_Pin = RS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(RS_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = RW_PIN;
    GPIO_Init(RW_GPIO_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = EN_PIN;
    GPIO_Init(EN_GPIO_PORT, &GPIO_InitStructure);

    // 初始化数据总线引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_11;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// 写指令到 LCD1602
void lcd_write_cmd(unsigned char cmd) {
    // RS = 0, RW = 0, EN = 0
    GPIO_WriteBit(RS_GPIO_PORT, RS_PIN, Bit_RESET);
    GPIO_WriteBit(RW_GPIO_PORT, RW_PIN, Bit_RESET);
    GPIO_WriteBit(EN_GPIO_PORT, EN_PIN, Bit_RESET);

    // 稍微延时确保控制信号稳定
    Mdelay_Lib(1);

    // 将指令送到 D0 ~ D7
		GPIO_WriteBit(GPIOD, GPIO_Pin_7, ((cmd >> 0) & 1) ? Bit_SET : Bit_RESET);  // D0
		GPIO_WriteBit(GPIOG, GPIO_Pin_15, ((cmd >> 1) & 1) ? Bit_SET : Bit_RESET); // D1
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, ((cmd >> 2) & 1) ? Bit_SET : Bit_RESET);  // D2
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, ((cmd >> 3) & 1) ? Bit_SET : Bit_RESET);  // D3
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, ((cmd >> 4) & 1) ? Bit_SET : Bit_RESET);  // D4
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, ((cmd >> 5) & 1) ? Bit_SET : Bit_RESET);  // D5
		GPIO_WriteBit(GPIOC, GPIO_Pin_11, ((cmd >> 6) & 1) ? Bit_SET : Bit_RESET); // D6
		GPIO_WriteBit(GPIOB, GPIO_Pin_6, ((cmd >> 7) & 1) ? Bit_SET : Bit_RESET);  // D7

    // 延时确保数据线稳定
    Mdelay_Lib(1);

    // 将 EN 拉高
    GPIO_WriteBit(EN_GPIO_PORT, EN_PIN, Bit_SET);

    // 延时一段时间 - EN高电平持续时间
    Mdelay_Lib(1);

    // 将 EN 拉低
    GPIO_WriteBit(EN_GPIO_PORT, EN_PIN, Bit_RESET);
    
    // 命令执行需要时间
    Mdelay_Lib(2);
}

// 写数据到 LCD1602
void lcd_write_dat(unsigned char dat) {
    // RS = 1, RW = 0, EN = 0
    GPIO_WriteBit(RS_GPIO_PORT, RS_PIN, Bit_SET);
    GPIO_WriteBit(RW_GPIO_PORT, RW_PIN, Bit_RESET);
    GPIO_WriteBit(EN_GPIO_PORT, EN_PIN, Bit_RESET);

    // 稍微延时确保控制信号稳定
    Mdelay_Lib(1);

    // 将数据送到 D0 ~ D7
		GPIO_WriteBit(GPIOD, GPIO_Pin_7, ((dat >> 0) & 1) ? Bit_SET : Bit_RESET);  // D0
		GPIO_WriteBit(GPIOG, GPIO_Pin_15, ((dat >> 1) & 1) ? Bit_SET : Bit_RESET); // D1
		GPIO_WriteBit(GPIOC, GPIO_Pin_6, ((dat >> 2) & 1) ? Bit_SET : Bit_RESET);  // D2
		GPIO_WriteBit(GPIOC, GPIO_Pin_7, ((dat >> 3) & 1) ? Bit_SET : Bit_RESET);  // D3
		GPIO_WriteBit(GPIOC, GPIO_Pin_8, ((dat >> 4) & 1) ? Bit_SET : Bit_RESET);  // D4
		GPIO_WriteBit(GPIOC, GPIO_Pin_9, ((dat >> 5) & 1) ? Bit_SET : Bit_RESET);  // D5
		GPIO_WriteBit(GPIOC, GPIO_Pin_11, ((dat >> 6) & 1) ? Bit_SET : Bit_RESET); // D6
		GPIO_WriteBit(GPIOB, GPIO_Pin_6, ((dat >> 7) & 1) ? Bit_SET : Bit_RESET);  // D7
		
    // 延时确保数据线稳定
    Mdelay_Lib(1);

    // 将 EN 拉高
    GPIO_WriteBit(EN_GPIO_PORT, EN_PIN, Bit_SET);

    // 延时一段时间 - EN高电平持续时间
    Mdelay_Lib(1);

    // 将 EN 拉低
    GPIO_WriteBit(EN_GPIO_PORT, EN_PIN, Bit_RESET);
    
    // 数据写入需要时间
    Mdelay_Lib(1);
}

// 初始化 LCD1602
void lcd_init() {
    lcd_gpio_init();
    
    // 给LCD一些时间稳定电源
    Mdelay_Lib(50);
    
    // LCD1602初始化序列
    lcd_write_cmd(0x38); // 8 位数据总线，两行显示，5x7 点阵字符
    Mdelay_Lib(5);
    
    lcd_write_cmd(0x38); // 重复发送初始化命令
    Mdelay_Lib(5);
    
    lcd_write_cmd(0x38); // 第三次发送确保初始化
    Mdelay_Lib(5);
    
    lcd_write_cmd(0x0C); // 显示开，光标关，光标不闪烁
    Mdelay_Lib(5);
    
    lcd_write_cmd(0x06); // 写一个数据后，显示位置右移一位
    Mdelay_Lib(5);
    
    lcd_write_cmd(0x01); // 显示清屏
    Mdelay_Lib(10);      // 清屏命令需要更长的延时
}

// 设置光标位置
void lcd_gotoxy(unsigned char line, unsigned char column) {
    if (line == 0) { // 第一行
        lcd_write_cmd(0x80 + column);
    } else if (line == 1) { // 第二行
        lcd_write_cmd(0x80 + 0x40 + column);
    }
}

// 在 LCD1602 上显示字符串
void lcd_print_str(unsigned char line, unsigned char column, const char *str) {
    lcd_gotoxy(line, column); // 设置光标位置
    while (*str != '\0') {
        lcd_write_dat(*str++);
        Mdelay_Lib(1); // 每个字符间稍微延时
    }
}

// 清屏函数
void lcd_clear(void) {
    lcd_write_cmd(0x01); // 清屏命令
    Mdelay_Lib(10);      // 清屏需要较长延时
}

// LCD调试测试函数
void lcd_debug_test(void) {
    int i;
    
    // 测试清屏
    lcd_clear();
    
    // 测试第一行显示
    lcd_print_str(0, 0, "TEST LINE 1");
    Mdelay_Lib(1000);
    
    // 测试第二行显示
    lcd_print_str(1, 0, "TEST LINE 2");
    Mdelay_Lib(1000);
    
    // 测试全屏填充
    lcd_clear();
    for (i = 0; i < 16; i++) {
        lcd_write_dat('A' + (i % 26));
        Mdelay_Lib(100);
    }
    
    // 切换到第二行
    lcd_gotoxy(1, 0);
    for (i = 0; i < 16; i++) {
        lcd_write_dat('0' + (i % 10));
        Mdelay_Lib(100);
    }
    
    Mdelay_Lib(2000);
    lcd_clear();
}

