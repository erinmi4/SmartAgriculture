#ifndef __BEEP_H
#define __BEEP_H

#include "stm32f4xx.h"

//Beep PF8
#define BEEP_PIN GPIO_Pin_8
#define BEEP_GPIO GPIOF
#define BEEP_RCC RCC_AHB1Periph_GPIOF

// 音符频率定义 (单位: Hz)
#define DO   262  // Do
#define RE   294  // Re
#define MI   330  // Mi
#define FA   349  // Fa
#define SOL  392  // Sol
#define LA   440  // La
#define SI   494  // Si
#define DO_H 523  // 高音Do

//蜂鸣器初始化
//BEEP -> PF8 -> GPIOF
void Beep_Init(void);

//打开蜂鸣器
void Beep_On(void);

//关闭蜂鸣器
void Beep_Off(void);

// 音乐蜂鸣器功能
void Beep_Tone(uint32_t freq, uint32_t duration_ms);
void Beep_PlayAlarm(void);  // 播放报警音
void Beep_ContinuousAlarm(void);  // 持续报警音

// 音乐开关控制功能
void Beep_SetMusicMode(uint8_t enable);  // 设置音乐模式开关
uint8_t Beep_GetMusicMode(void);         // 获取当前音乐模式状态
void Beep_AlarmSound(void);              // 统一的报警声音接口（根据模式自动选择）

#endif
