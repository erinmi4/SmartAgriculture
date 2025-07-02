#include "i2c.h"
#include "delay.h"

/**
 * @brief I2C初始化
 */
void MyI2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef I2C_InitStructure;

    // 使能I2C和GPIO时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    // 配置I2C引脚
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 配置引脚复用功能
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_I2C1);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_I2C1);

    // 复位I2C
    I2C_DeInit(I2C1);

    // 配置I2C
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000; // 100kHz，稳定可靠

    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_Cmd(I2C1, ENABLE);
}

/**
 * @brief 等待I2C事件
 * @param event: 要等待的事件
 * @retval 0-成功，1-超时
 */
static uint8_t MyI2C_WaitEvent(uint32_t event)
{
    uint32_t timeout = 10000;
    
    while(!I2C_CheckEvent(I2C1, event))
    {
        if(--timeout == 0)
            return 1; // 超时
    }
    return 0; // 成功
}

/**
 * @brief I2C写寄存器
 * @param addr: 设备地址
 * @param reg: 寄存器地址
 * @param data: 要写入的数据
 * @retval 0-成功，其他-失败
 */
uint8_t MyI2C_WriteReg(uint8_t addr, uint8_t reg, uint8_t data)
{
    // 发送开始信号
    I2C_GenerateSTART(I2C1, ENABLE);
    if(MyI2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
        return 1;

    // 发送设备地址
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
    if(MyI2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        return 2;

    // 发送寄存器地址
    I2C_SendData(I2C1, reg);
    if(MyI2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        return 3;

    // 发送数据
    I2C_SendData(I2C1, data);
    if(MyI2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        return 4;

    // 发送停止信号
    I2C_GenerateSTOP(I2C1, ENABLE);

    return 0;
}

/**
 * @brief I2C读寄存器
 * @param addr: 设备地址
 * @param reg: 寄存器地址
 * @retval 读取到的数据
 */
uint8_t MyI2C_ReadReg(uint8_t addr, uint8_t reg)
{
    uint8_t data = 0;
    
    // 发送开始信号
    I2C_GenerateSTART(I2C1, ENABLE);
    if(MyI2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
        return 0;

    // 发送设备地址（写模式）
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
    if(MyI2C_WaitEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        return 0;

    // 发送寄存器地址
    I2C_SendData(I2C1, reg);
    if(MyI2C_WaitEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        return 0;

    // 重新发送开始信号
    I2C_GenerateSTART(I2C1, ENABLE);
    if(MyI2C_WaitEvent(I2C_EVENT_MASTER_MODE_SELECT))
        return 0;

    // 发送设备地址（读模式）
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Receiver);
    if(MyI2C_WaitEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        return 0;

    // 禁用ACK
    I2C_AcknowledgeConfig(I2C1, DISABLE);

    // 发送停止信号
    I2C_GenerateSTOP(I2C1, ENABLE);

    // 等待数据接收
    if(MyI2C_WaitEvent(I2C_EVENT_MASTER_BYTE_RECEIVED))
        return 0;

    // 读取数据
    data = I2C_ReceiveData(I2C1);

    // 重新使能ACK
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return data;
}

/**
 * @brief I2C连续读取多个寄存器
 * @param addr: 设备地址
 * @param reg: 起始寄存器地址
 * @param buf: 数据缓冲区
 * @param len: 读取长度
 * @retval None
 */
void MyI2C_ReadMultiReg(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    for(i = 0; i < len; i++)
    {
        buf[i] = MyI2C_ReadReg(addr, reg + i);
    }
}
