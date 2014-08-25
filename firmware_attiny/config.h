/*
 * config.h
 *
 * Created: 5/24/2014 9:22:14 AM
 *  Author: true
 *
 * $Id: config.h 164 2014-07-06 04:34:29Z true $
 */ 


#ifndef CONFIG_H_
#define CONFIG_H_


// system
#define F_CPU					8000000UL


// LEDs
#define LED_COMMON_ANODE 		1			// defined = common anode, undef = common cathode
//#define LED_RGBX_4LED			1			// defined = use 4 LEDs (RGBX), undef = use 3 LEDs (RGB)


// i2c config
#define I2C_SLAVE_ADDRESS		0x73		// 7-bit address (0b1110011)
#define	I2C_ENABLE_ADDR_ZERO	0			// 1 = enable response on 0x00, 0 = disable


#endif /* CONFIG_H_ */