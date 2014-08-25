/*
 * i2c.h
 *
 * Created: 5/21/2014 11:41:32 PM
 *  Author: true
 *
 * $Id: i2c.h 163 2014-07-06 01:58:45Z true $
 */ 


#ifndef I2C_H_
#define I2C_H_


#include <avr/io.h>


/* macros */
#define	i2c_clear_int_flag()		TWCR |= _BV(TWINT)

#define i2c_disable_interrupt()		TWCR &= ~(_BV(TWIE))
#define i2c_enable_interrupt()		TWCR |= _BV(TWIE)

#define i2c_disable_slave()			TWCR &= ~(_BV(TWEA))
#define i2c_enable_slave()			TWCR |= _BV(TWEA)


/* configuration */
static inline void i2c_slave_init(uint8_t address, uint8_t enable_general_addr)
{
	// set bitrate register, clear prescaler
	// should get us 400KHz (fast mode) max rate
	TWBR = 2;
	TWSR = ~(_BV(TWPS0) | _BV(TWPS1));
	
	// set slave address and clear address mask
	TWAR = ((address << 1) | (enable_general_addr & 1));
	TWAMR = 0;
	
	// enable auto-ack, enable TWI, and enable the TWI interrupt
	TWCR = _BV(TWEA) | _BV(TWEN) | _BV(TWIE);
}

/* sending */
static uint8_t i2c_slave_tx(uint8_t byte, uint8_t is_last_byte)
{
	// load data
	TWDR = byte;
	
	// start sending
	TWCR |= _BV(TWINT);
	
	if (is_last_byte) {
		// this is all the data we have to send; send a nack after this byte
		i2c_disable_slave();	
	}
	
	return 0;
}


#endif /* I2C_H_ */