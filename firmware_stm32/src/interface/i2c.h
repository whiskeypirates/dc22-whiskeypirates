/*
 * i2c.h: pirate i2c interface, command and data buffering, and data receive callbacks
 * 2014 true
 *
 * ----
 *
 * $Id: i2c.h 214 2014-08-04 05:31:33Z true $
 */

#ifndef __PIRATE_I2C_H
#define __PIRATE_I2C_H


#include "pirate.h"

// we're using this guy's library because it seems to work
#include "lib/i2c/peter_i2c.h"


#define I2C_BUF_SIZE 			16  	// power of 2

#define I2C_READ 				I2C_Direction_Receiver
#define I2C_WRITE 				I2C_Direction_Transmitter


/* prototypes */
void i2c_init(I2C_TypeDef *i2c, uint32_t speed);


#endif
