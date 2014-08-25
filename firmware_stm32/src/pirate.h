/*
 * pirate.h: half-assed piracy-enabling features for drunken, brazen pirates
 * 2014 by true
 *
 * ----
 *
 * $Id: pirate.h 221 2014-08-07 16:55:36Z true $
 */


#ifndef __PIRATE_PIRATE_H
#define __PIRATE_PIRATE_H


#include <stdlib.h>
#include <stdint.h>
#include <string.h>


/* cmsis req'd */
#include "stm32l1xx.h"


#define PIRATE_PROG_SAVED_MAX 		15


/* helper macros */
#define sizeof_array(x) 		sizeof(x) / sizeof(x[0])


/* pirate settings */
#define PIRATE_PROG_MATRIX 			0
#define PIRATE_PROG_BONES 			1
#define PIRATE_PROG_EYES 			2

typedef struct PirateLEDProg {
	uint8_t type; 		// bit0 = pattern(high)/program(low), bit1 = always init, bit7 = enabled
	uint8_t progidx;
	uint16_t wait;
	uint16_t level;
	uint32_t offset;
	uint32_t option;
	uint16_t dwell;
} PirateLEDProg;

typedef struct PirateSettings {
	char name[32];
	uint8_t fav_color[3];
	uint8_t beeper;
	uint8_t beep_type[8];
	uint8_t contrast;
	uint8_t autorun;
	uint8_t led_autoadjust;
	uint8_t led_autogain_lev_min;
	uint8_t led_autogain_lev_max;
	uint8_t led_autothresh[5];
	uint8_t light_setgain;
	uint8_t lcd_autobrite;
	uint8_t lcd_brightness;
	PirateLEDProg led_prog[3][PIRATE_PROG_SAVED_MAX];
	uint8_t led_prog_mode;
	uint16_t mic_cal[2];
} PirateSettings;

extern PirateSettings settings;
#define PIRATE_SETTINGS_EEPROM_ADDR 		(uint32_t)0x8080000


/* misc extras */
extern uint8_t temperature;
extern int8_t temperature_cal;
extern uint16_t mic_peak;


/* pirate variables */
extern uint32_t pirate_prng_val;


/* pirate functions */
void pirate_shutdown(uint16_t type);

uint8_t pirate_prng();
void pirate_delay(uint16_t ms);
int16_t pirate_scale(int16_t value, int16_t src_min, int16_t src_max, int16_t dest_min, int16_t dest_max);
char * pirate_itoa(uint32_t val, uint8_t base, uint8_t leftpad);
char * pirate_sitoa(int32_t val, uint8_t base, uint8_t leftpad);

uint16_t pirate_batt_voltage();
void pirate_batt_log(uint16_t rawvalue);

uint16_t pirate_thermometer(uint8_t deg_f);
void pirate_thermometer_log(uint8_t temp);

#endif
