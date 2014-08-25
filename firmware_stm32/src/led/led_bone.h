/*
 * led_bone.h: skull crossbones led matrix handling prototypes
 * 2014 true
 *
 * ----
 *
 * $Id: led_bone.h 211 2014-08-03 21:06:13Z true $
 */

#ifndef __PIRATE_LED_BONE_H
#define __PIRATE_LED_BONE_H


#include <stdint.h>

#include "stm32l1xx.h"
#include "pirate.h"
#include "led_matrix.h"


#define LED_BONE_PROG_COUNT 3
#define LED_BONE_PTRN_COUNT 0


/* led positions */
#define BONE_LED_BR_LOWER	0
#define BONE_LED_TR_LOWER	1
#define BONE_LED_BR_UPPER	3
#define BONE_LED_TL_UPPER	4
#define BONE_LED_TL_LOWER	5
#define BONE_LED_TR_UPPER	6
#define BONE_LED_BL_UPPER	7
#define BONE_LED_BL_LOWER	8


/* struct */
typedef struct LEDBonePattern {
	uint16_t wait;
	uint8_t led[9];
} LEDBonePattern;


/* variables */
extern const char led_bone_prog_name[LED_BONE_PROG_COUNT][16];


/* prototypes */
void led_bone_io_init();
void led_bone_next();

void led_bone_mode_update();
void led_bone_set_mode(uint8_t mode); 	// used for turning off
void led_bone_set_pattern(uint8_t pattern_idx);
void led_bone_set_program(uint8_t program_idx, uint8_t init,
		uint16_t wait, uint16_t level, uint32_t offset, uint32_t settings);

#endif
