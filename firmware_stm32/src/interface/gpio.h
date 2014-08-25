/*
 * gpio.h: pirate gpio interface
 * 2014 true
 *
 * ----
 *
 * $Id: gpio.h 214 2014-08-04 05:31:33Z true $
 */

#ifndef __PIRATE_IF_GPIO_H
#define __PIRATE_IF_GPIO_H


#include "pirate.h"


#define BIT_0 		1
#define BIT_1 		(1 << 1)
#define BIT_2 		(1 << 2)
#define BIT_3 		(1 << 3)
#define BIT_4 		(1 << 4)
#define BIT_5 		(1 << 5)
#define BIT_6 		(1 << 6)
#define BIT_7 		(1 << 7)

#define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))

typedef struct tGPIO {
	GPIO_TypeDef *port;
	uint16_t pin;
	uint8_t pinsource;
} tGPIO;


#endif
