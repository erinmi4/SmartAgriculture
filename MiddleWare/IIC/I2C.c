#include "i2c.h"
#include <stddef.h> // 添加头文件定义NULL

// 初始化I2C接口
void My_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    // 使能GPIOB和I2C1时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // 配置PB8和PB9为复用功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 将PB8和PB9连接到I2C1
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);

    // 配置I2C1
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;
    I2C_Init(I2C1, &I2C_InitStructure);

    // 使能I2C1
    I2C_Cmd(I2C1, ENABLE);
}

// 向I2C设备写入一个字节
void I2C_Write_Byte(uint8_t addr, uint8_t reg, uint8_t data)
{
    uint32_t timeout = 10000; // 设置超时值
    
    // 等待I2C总线空闲，带超时
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) && --timeout);
    if (timeout == 0) return;
    
    timeout = 10000;
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return;
    }
    
    timeout = 10000;
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return;
    }
    
    timeout = 10000;
    I2C_SendData(I2C1, reg);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return;
    }
    
    timeout = 10000;
    I2C_SendData(I2C1, data);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --timeout);
    
    I2C_GenerateSTOP(I2C1, ENABLE);
}

// 从I2C设备读取一个字节
uint8_t I2C_Read_Byte(uint8_t addr, uint8_t reg)
{
    uint8_t data = 0; // 默认返回0
    uint32_t timeout = 10000; // 设置超时值
    
    // 等待I2C总线空闲，带超时
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) && --timeout);
    if (timeout == 0) return data;
    
    timeout = 10000;
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return data;
    }
    
    timeout = 10000;
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return data;
    }
    
    timeout = 10000;
    I2C_SendData(I2C1, reg);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return data;
    }
    
    timeout = 10000;
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return data;
    }
    
    timeout = 10000;
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return data;
    }

    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_GenerateSTOP(I2C1, ENABLE);

    timeout = 10000;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && --timeout);
    if (timeout == 0) return data;
    
    data = I2C_ReceiveData(I2C1);
    return data;
}

// 从I2C设备读取多个字节
void I2C_Read_Multiple(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint32_t timeout = 10000; // 设置超时值
    
    // 如果buf为空或len为0，则直接返回
    if (buf == NULL || len == 0) return;
    
    // 等待I2C总线空闲，带超时
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) && --timeout);
    if (timeout == 0) return;
    
    timeout = 10000;
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return;
    }
    
    timeout = 10000;
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return;
    }
    
    timeout = 10000;
    I2C_SendData(I2C1, reg);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return;
    }

    timeout = 10000;
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return;
    }

    timeout = 10000;
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && --timeout);
    if (timeout == 0) {
        I2C_GenerateSTOP(I2C1, ENABLE);
        return;
    }

    I2C_AcknowledgeConfig(I2C1, ENABLE);

    for (uint8_t i = 0; i < len - 1; i++)
    {
        timeout = 10000;
        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && --timeout);
        if (timeout == 0) {
            I2C_GenerateSTOP(I2C1, ENABLE);
            return;
        }
        buf[i] = I2C_ReceiveData(I2C1);
    }

    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_GenerateSTOP(I2C1, ENABLE);

    timeout = 10000;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && --timeout);
    if (timeout > 0) {
        buf[len - 1] = I2C_ReceiveData(I2C1);
    }
}


