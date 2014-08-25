/*
 * spi.h
 *
 * Created: 5/21/2014 9:56:06 PM
 *  Author: true
 *
 * $Id: spi.h 83 2014-05-22 07:43:24Z true $
 */ 


#ifndef SPI_H_
#define SPI_H_


#include <avr/io.h>


/* prototypes */


/* implementations */
inline static void spi_slave_init(uint8_t phase, uint8_t polarity, uint8_t order)
{
	// set MISO as output
	DDRB |= (_BV(DDB4));
	// enable spi interrupts, enable spi, set order, polarity and phase
	SPCR = _BV(SPIE) | _BV(SPE) | (order << DORD) | (polarity << CPOL) | (phase << CPHA);
}


#endif /* SPI_H_ */