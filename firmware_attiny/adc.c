/*
 * adc.c
 *
 * Created: 6/9/2014 11:07:35 PM
 *  Author: true
 */


#include <avr/io.h>
#include <avr/interrupt.h>

#include "adc.h"


volatile uint8_t adc_busy;
volatile uint8_t adc_reread;

volatile uint16_t adc_result[ADC_MAX_FEEDBACK + 1];
static uint8_t adc_averages;

volatile uint8_t adc_read_mode;


/* functions */
void adc_init()
{
	// make sure ADC power is enabled
	PRR &= ~_BV(PRADC);
	
	// clear all settings; set the ADC prescaler bits to get 500KHz ADC clock (SYSCLK/16), disable auto trigger
	ADCSRA = _BV(ADPS2) | _BV(ADPS0) | ~(_BV(ADATE));
	
	// set to right-aligned mode
	ADMUX &= ~(_BV(ADLAR));
	
	// enable the ADC, enable interrupts
	ADCSRA |= _BV(ADEN) | _BV(ADIE);
}

void adc_channel(uint8_t channel, uint8_t vref)
{
	// can we set channel?
	if (channel < ADC_CHAN_INVALID) {
		// set MUX[3:0] to the specified channel
		ADMUX = (ADMUX & 0xf0) | (channel & 0x0f);
		// also, if this is an actual pin, disable digital input on this pin
		DIDR0 = (channel < 8) ? _BV(channel) : 0;
	}
	
	// set the voltage source
	if (vref == ADC_REF_AVCC) {
		ADMUX |= (_BV(REFS0));
	} else if (vref == ADC_REF_BANDGAP) {
		ADMUX &= ~(_BV(REFS0));
	}
}

uint8_t adc_start(uint8_t reread, uint8_t enable_averaging)
{
	// is a conversion already running?
	if (ADCSRA & _BV(ADSC)) {
		return 1;
	}
	
	// set up amount of times to re-read, and mark the amount of values to average
	adc_reread = reread;
	adc_averages = enable_averaging ? reread : 0;
	
	// start conversion
	adc_busy = 0x80 | (ADMUX & 0x0f);
	ADCSRA |= _BV(ADSC);
	return 0;
}


/* ISR */
ISR(ADC_vect)
{
	uint8_t channel;
	
	channel = ADMUX & 0x0f;
	
	// mark our result in the average table
	if (adc_averages) {
		// except for the first read
		if (adc_averages != adc_reread) {
			// we do successive averaging; it's faster, smaller code, less RAM, and good enough for our purpose
			// update our feedback
			if (channel <= ADC_MAX_FEEDBACK) {
				adc_result[channel] = (adc_result[channel] + ADC) >> 1;
			}
		}
	}
	
	if (adc_reread) {
		adc_reread--;
		// start another conversion
		ADCSRA |= _BV(ADSC);
	} else {
		// determine if we need to update feedback
		if (!adc_averages && channel <= ADC_MAX_FEEDBACK) {
			// not averaging, so just store last result
			adc_result[channel] = ADC;
		}
	
		adc_busy = 0;
	}
}