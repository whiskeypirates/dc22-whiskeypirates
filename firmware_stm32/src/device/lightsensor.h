/*
 * lightsensor.h: control of the attiny's light sensor output plus helper functions
 * 2014 true
 *
 * ----
 *
 * $Id$
 */


#ifndef __PIRATE_DEV_LIGHTSENSOR_H
#define __PIRATE_DEV_LIGHTSENSOR_H


#include "pirate.h"


extern uint8_t light_level;
extern uint8_t light_gain;


#define LIGHTSENS_GAIN_MIN 				1 		// brightest
#define LIGHTSENS_GAIN_MAX  			64 		// darkest

#define LIGHTSENS_AUTOGAIN_LEVEL_MIN 	108		// threshold levels for gain
#define LIGHTSENS_AUTOGAIN_LEVEL_MAX 	114 	// should be a range of ~6-10

#define LIGHTSENS_THRESH_DAY			1
#define LIGHTSENS_THRESH_TOP_BRIGHT		4
#define LIGHTSENS_THRESH_TOP_NORM		30
#define LIGHTSENS_THRESH_TOP_DIM 		48
#define LIGHTSENS_THRESH_TOP_DARK 		64

#define LIGHTSENS_SCALED_SKULL_WHT 		0
#define LIGHTSENS_SCALED_SKULL_PUR 		1
#define LIGHTSENS_SCALED_BONES	 		2
#define LIGHTSENS_SCALED_EYES 			3
#define LIGHTSENS_SCALED_BACKLIGHT 		4


/* prototypes */
uint8_t lightsensor_gainvalue_update();

void lightsensor_scaler_update();
uint8_t lightsensor_get_scalerval(uint8_t which);


#endif
