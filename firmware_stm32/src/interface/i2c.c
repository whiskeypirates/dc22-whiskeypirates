/*
 * i2c.h: pirate i2c interface, command and data buffering, and data receive callbacks
 * 2014 true
 *
 * ----
 *
 * $Id: i2c.c 214 2014-08-04 05:31:33Z true $
 */


#include "i2c.h"

#include "gpio.h"


#define I2C_GPIO_SCL 		0
#define I2C_GPIO_SDA 		1

static const tGPIO i2c_pins[2][2] = {
	{ 	// i2c1
		{GPIOB, GPIO_Pin_8, 8}, 	// scl
		{GPIOB, GPIO_Pin_9, 9} 		// sda
	},
	{ 	// i2c2
		{GPIOB, GPIO_Pin_10, 10}, 	// scl
		{GPIOB, GPIO_Pin_11, 11} 	// sda
	}
};


/* functions */
void i2c_init(I2C_TypeDef *i2cdev, uint32_t speed)
{
	GPIO_InitTypeDef gpio;
	I2C_InitTypeDef i2c;

	uint8_t i2c_idx;

	if (i2cdev == I2C1) {
		i2c_idx = 0;
	} else {
		i2c_idx = 1;
	}

	// enable peripheral clocks
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	if (i2cdev == I2C1) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
	} else if (i2cdev == I2C2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
		// and reset it (silicon bug?)
		RCC_AHBPeriphResetCmd(RCC_APB1Periph_I2C2, ENABLE);
		RCC_AHBPeriphResetCmd(RCC_APB1Periph_I2C2, DISABLE);
	}

	// set up peripheral
	I2C_StructInit(&i2c);
	i2c.I2C_ClockSpeed = speed;
	I2C_Init(i2cdev, &i2c);

	// set up pins
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_OType = GPIO_OType_OD;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;

	gpio.GPIO_Pin = i2c_pins[i2c_idx][I2C_GPIO_SCL].pin | i2c_pins[i2c_idx][I2C_GPIO_SDA].pin;
	GPIO_Init(i2c_pins[i2c_idx][I2C_GPIO_SCL].port, &gpio);

	// set pin alternate function
	GPIO_PinAFConfig(i2c_pins[i2c_idx][I2C_GPIO_SCL].port,
			i2c_pins[i2c_idx][I2C_GPIO_SCL].pinsource, GPIO_AF_I2C1);
	GPIO_PinAFConfig(i2c_pins[i2c_idx][I2C_GPIO_SDA].port,
			i2c_pins[i2c_idx][I2C_GPIO_SDA].pinsource, GPIO_AF_I2C1);

	// enable the i2c interrupts

	// enable peripheral
	I2C_Cmd(i2cdev, ENABLE);
}
