/*
 * led_matrix.h: main skull led matrix handling prototypes
 * 2014 true
 *
 * ----
 *
 * $Id: led_matrix.h 205 2014-08-03 03:24:27Z true $
 */

#ifndef __PIRATE_LED_MATRIX_H
#define __PIRATE_LED_MATRIX_H


#include <stdlib.h>
#include <stdint.h>

#include "stm32l1xx.h"

#include "../interface/gpio.h"
#include "../interface/led_pwm.h"


#define LED_MATRIX_PROG_COUNT 	6
#define LED_MATRIX_PTRN_COUNT 	0


/* interface */
#define LED_MATRIX_WHITE 		0
#define LED_MATRIX_PURPLE 		1

/* programs */
#define LED_MATRIX_MODE_OFF 	0
#define LED_MATRIX_MODE_PATTERN 1
#define LED_MATRIX_MODE_PROGRAM 2


/* struct */
typedef struct LEDFixedPattern {
	uint16_t wait;
	uint32_t led[8];
} LEDFixedPattern;


/* variables */
extern const char led_matrix_prog_name[LED_MATRIX_PROG_COUNT][16];


/* prototypes */
void led_matrix_io_init();
void led_matrix_next();

void led_matrix_mode_update();
void led_matrix_set_mode(uint8_t mode); 	// used for turning off
void led_matrix_set_pattern(uint8_t pattern_idx);
void led_matrix_set_program(uint8_t program_idx, uint8_t init,
		uint16_t wait, uint16_t level, uint32_t offset, uint32_t settings);


#endif
