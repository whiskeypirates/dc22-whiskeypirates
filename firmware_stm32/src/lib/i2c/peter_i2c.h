#ifndef _I2C_H
#define _I2C_H


#include <stdlib.h>
#include <stdint.h>

#include "stm32l1xx.h"


//#define I2C1_OPEN
#define I2C2_OPEN

#define I2C_SCL_SPEED       (200000)    // SCL in Hz
#define I2C_OWN_ADDR        (0x00)      // use 0x00 for master

/**
 * @brief Definition for I2C1
 */  
#if defined	I2C1_OPEN
	#define PeterI2C                        I2C1
	#define PeterI2C_CLK                    RCC_APB1Periph_I2C1
	
	#define PeterI2C_SCL_PIN                GPIO_Pin_6
	#define PeterI2C_SCL_GPIO_PORT          GPIOB
	#define PeterI2C_SCL_GPIO_CLK           RCC_AHB1Periph_GPIOB
	#define PeterI2C_SCL_SOURCE             GPIO_PinSource6
	#define PeterI2C_SCL_AF                 GPIO_AF_I2C1
	
	#define PeterI2C_SDA_PIN                GPIO_Pin_9
	#define PeterI2C_SDA_GPIO_PORT          GPIOB
	#define PeterI2C_SDA_GPIO_CLK           RCC_AHB1Periph_GPIOB
	#define PeterI2C_SDA_SOURCE             GPIO_PinSource9
	#define PeterI2C_SDA_AF                 GPIO_AF_I2C1
	
	#define PeterI2C_IRQn                   I2C1_EV_IRQn
	#define I2Cx_IRQHANDLER                 I2C1_EV_IRQHandler

#elif defined I2C2_OPEN
  #define PeterI2C                        I2C2
  #define PeterI2C_CLK                    RCC_APB1Periph_I2C2

  #define PeterI2C_SCL_PIN                GPIO_Pin_10
  #define PeterI2C_SCL_GPIO_PORT          GPIOB
  #define PeterI2C_SCL_GPIO_CLK           RCC_AHB1Periph_GPIOB
  #define PeterI2C_SCL_SOURCE             GPIO_PinSource10
  #define PeterI2C_SCL_AF                 GPIO_AF_I2C2

  #define PeterI2C_SDA_PIN                GPIO_Pin_11
  #define PeterI2C_SDA_GPIO_PORT          GPIOB
  #define PeterI2C_SDA_GPIO_CLK           RCC_AHB1Periph_GPIOB
  #define PeterI2C_SDA_SOURCE             GPIO_PinSource11
  #define PeterI2C_SDA_AF                 GPIO_AF_I2C2

  #define PeterI2C_IRQn                   I2C2_EV_IRQn
  #define I2Cx_IRQHANDLER                 I2C2_EV_IRQHandler

#else
	#error "Please select the I2C-Device to be used (in i2c.h)"
#endif


#define I2C_TFLAG_BUS_BUSY 				0x01
#define I2C_TFLAG_START_FAILED			0x02
#define I2C_TFLAG_NO_ACK 				0x04
#define I2C_TFLAG_WRITE_FAILED 			0x08
#define I2C_TFLAG_READ_FAILED			0x10

int  I2C_ReadTransfer(uint8_t dev_addr, uint8_t *buffer, int cnt, uint32_t ptr, uint8_t ptrlen);
int  I2C_WriteTransfer(uint8_t dev_addr, uint8_t *buffer, int cnt, uint32_t ptr, uint8_t ptrlen);

#endif /*_I2C_H*/
