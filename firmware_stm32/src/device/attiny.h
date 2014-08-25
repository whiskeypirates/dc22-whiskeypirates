/*
 * attiny.h: interface to attiny88's RGBLED eyes prototypes
 * 2014 true
 *
 * ----
 *
 * $Id: attiny.h 221 2014-08-07 16:55:36Z true $
 */

#ifndef __PIRATE_DEV_ATTINY_H
#define __PIRATE_DEV_ATTINY_H


#include <stdlib.h>
#include <stdint.h>

#include "stm32l1xx.h"

#include "pirate.h" 		// used for prng


/* interface */
#define ATTINY_I2C_DEV 				I2C1
#define ATTINY_I2C_ADDR 			0x73

  // write register commands, last 4 bits = data, >1 packet
#define ATTINY_CMD_EXT_CMD 			0x10 		// pkt = ext command (bit[7:4] must NOT == 0)
#define ATTINY_CMD_LED_LEVEL 		0x20 		// cmd[3:0] = led, pkt = level (0-250)
#define ATTINY_CMD_TEMP_CAL 		0x30 		// pkt = current temperature
#define ATTINY_CMD_EEPROM_READ 		0x40 		// pkt = address, read 1 byte after this command
#define ATTINY_CMD_EEPROM_WRITE 	0x50 		// pkt[0] = address, pkt[1] = data
  // write register commands, last 4 bits = data, immediate processing 1 packet
#define ATTINY_CMD_READ_TEMP 		0x80 		// read 1 byte after this command
#define ATTINY_CMD_READ_LIGHT 		0x90 		// cmd[3:0] = led (from 0-3), read 1 byte after this command
#define ATTINY_CMD_SLEEP 			0xF0 		// no data
  // write register commands, extended
#define ATTINY_CMD_LIGHTSENSOR_SENS 0x1019 		// 2 bytes data - led, new sensitivity value

/* leds */
#define LED_EYES_LEFT 				0
#define LED_EYES_RIGHT 				1

/* programs */
#define LED_EYES_MODE_OFF 			0
#define LED_EYES_MODE_PROGRAM 		2

#define LED_EYES_PROG_COUNT 		4
#define LED_EYES_PTRN_COUNT 		0
/* prototypes */
void led_eyes_tx();
void led_eyes_set_level(uint8_t idx, uint8_t level);

void led_eyes_mode_update();
void led_eyes_set_mode(uint8_t mode); 	// used for disabling
void led_eyes_set_program(uint8_t program_idx, uint8_t init,
		uint16_t wait, uint16_t level, uint32_t offset, uint32_t settings);

uint8_t attiny_read_temp();
uint8_t attiny_read_light_level(uint8_t led_idx);

void attiny_write_light_sensitivity(uint8_t led, uint8_t sensitivity);

void attiny_sleep();


#endif
