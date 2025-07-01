#ifndef _DHT11_H_
#define _DHT11_H_
#include "stm32f4xx.h"


//PE5
#define DHT11_PORT	GPIOE
#define DHT11_IO	GPIO_Pin_5
#define DHT11_RCC	RCC_AHB1Periph_GPIOE

void dht11_io_out(void);
void dht11_io_in(void);
void dht11_init(void);
void dht11_start(void);
unsigned char dht11_check(void);
unsigned char dht11_read_bit(void);
unsigned char dht11_read_byte(void);
unsigned char dht11_read_dat(unsigned char *temp, unsigned char *humi);


#endif


