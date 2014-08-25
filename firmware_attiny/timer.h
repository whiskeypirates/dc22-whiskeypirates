/*
 * timer.h
 *
 * Created: 5/21/2014 10:45:08 PM
 *  Author: true
 *
 * $Id: timer.h 154 2014-06-14 07:22:48Z true $
 */ 


#ifndef TIMER_H_
#define TIMER_H_


#include <avr/io.h>
#include "led.h"


#define TIMER0_COMPARE 		62 - 1
#define TIMER1_COMPARE 		252 - 1 		// divisible by 63 which is set every other round for timer0


uint8_t timer1_mode;


/* prototypes */


/* implementations */
inline static void timer0_init()
{
	// make sure timer is enabled
	PRR &= ~PRTIM0;
	
	// enable timer with prescale divider /64, enable TOP mode
	TCCR0A = _BV(CTC0) | (_BV(CS00) | _BV(CS01) | ~(_BV(CS02)));

	// set TOP level to be 62 to 63 counts; this will result in 2KHz cycle @8MHz
	OCR0A = TIMER0_COMPARE;
	
	// clear timer
	TCNT0 = 0;
	
	// and enable OCA interrupt (we don't use overflow interrupt as we are in TOP mode)
	TIMSK0 = _BV(OCIE0A);
}

#define timer0_set_compare(x)		OCR0A = x

inline static void timer1_init()
{
	// make sure timer is enabled
	PRR &= ~PRTIM1;
	
	// clear PWM OCx
	OCR1AH = 0xff;		// sets temp register to 0xff
	OCR1AL = 0xff;
	OCR1BL = 0xff;
	
	// clear timer
	TCNT1H = 0;			// sets temp register to 0
	TCNT1L = 0;
	
	// enable timer OC1 and OC2 clear on match / set at TOP, set to fast PWM mode 14 (ICR1 compare), prescaler off
	// note: in PWM mode 14, pin PB0 can only be used as an output... =(
	TCCR1A = (_BV(COM1A1)) | (_BV(COM1B1)) | (_BV(WGM11) | ~(_BV(WGM10)));
	TCCR1B = (_BV(WGM12) | _BV(WGM13)) | (_BV(CS10));
	TCCR1C = 0;
	
	// store timer1 mode to allow enabling / disabling timer
	timer1_mode = TCCR1A;

	// set ICR compare to result in a PWM rate of 8MHz / 252, or 32256Hz
	// This is 16 PWM updates per timer0 63-count period (16.25 per 62-count period)
	// In practice we don't get the first PWM output, so we get:
	//   15/16 PWM / (x) LEDs = 31.25% max duty for 3 LED, 23.43% max duty for 4 LED
	ICR1L = TIMER1_COMPARE;
	
	// set OC pins as outputs
	DDRB |= (_BV(DDB2) | _BV(DDB1));
	
	// disable interrupts
	TIMSK1 = 0;
}

inline void timer1_pwm_reinit()
{	
	// enable pwm output on OCx pins
	TCCR1A = (_BV(COM1A1)) | (_BV(COM1B1)) | (_BV(WGM11) | ~(_BV(WGM10)));
	
	// set OC pins as outputs
	DDRB |= (_BV(DDB2) | _BV(DDB1));
}

inline void timer1_set(uint16_t set)
{
	TCNT1 = set;
}


#define		timer1_disable()		TCCR1A = 0
#define		timer1_enable()			TCCR1A = timer1_mode;

#ifdef LED_COMMON_ANODE
	#define		timer1_set_oca(v)	OCR1AL = TIMER1_COMPARE - v
	#define		timer1_set_ocb(v)	OCR1BL = TIMER1_COMPARE - v
#else
	#define		timer1_set_oca(v)	OCR1AL = TIMER1_COMPARE
	#define		timer1_set_ocb(v)	OCR1BL = TIMER1_COMPARE
#endif


#endif /* TIMER_H_ */