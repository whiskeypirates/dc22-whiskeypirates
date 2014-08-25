/*
 * led.h
 *
 * Created: 6/13/2014 1:40:53 AM
 *  Author: true
 */ 


#ifndef LED_H_
#define LED_H_


/* incl */
#include "config.h"

#include <avr/io.h>
#include <util/delay.h>

#include "adc.h"
#include "timer.h"


/* led */
#define		LED_RED					0
#define		LED_GREEN				1
#define		LED_BLUE				2
#define		LED_SPARE				3


/* globals */
extern uint8_t rgbled_pwm_lf[4];		// pwm value for TIMER1 OCA LED outputs
extern uint8_t rgbled_pwm_rt[4];		// pwm value for TIMER1 OCB LED outputs
register uint8_t rgbled_idx asm("r13");	// the currently operated LED


/* prototypes */
void rgbled_io_init();
void rgbled_update();

void rgbled_sensor_sensitivity(uint8_t ledidx, uint8_t sensitivity);
void rgbled_sensor_read_idx(uint8_t ledidx);
void rgbled_sensor_read();


#endif /* LED_H_ */