/*
 * adc.h
 *
 * Created: 6/9/2014 11:07:51 PM
 *  Author: true
 */ 


#ifndef ADC_H_
#define ADC_H_


#include "config.h"


#define ADC_CHAN_0			0
#define ADC_CHAN_1			1
#define ADC_CHAN_2			2
#define ADC_CHAN_3			3
#define ADC_CHAN_4			4
#define ADC_CHAN_5			5
#define ADC_CHAN_6			6
#define ADC_CHAN_7			7
#define ADC_CHAN_TEMP		8
#define ADC_CHAN_BANDGAP	14
#define ADC_CHAN_GND		15
#define ADC_CHAN_INVALID	ADC_CHAN_GND + 1 		// one above last valid channel

#define ADC_REF_AVCC		1
#define ADC_REF_BANDGAP		2
#define ADC_REF_NO_SET		255

#define ADC_MAX_FEEDBACK	ADC_CHAN_TEMP

// adc read interrupt
extern volatile uint8_t adc_busy;
extern volatile uint16_t adc_result[ADC_MAX_FEEDBACK + 1];

// main timer adc handler
extern volatile uint8_t adc_read_mode;
register uint8_t adc_read_step asm("r14");


/* application specific */
#define ADC_READ_LED_RED	ADC_CHAN_0
#define ADC_READ_LED_GREEN	ADC_CHAN_1
#define ADC_READ_LED_BLUE	ADC_CHAN_2


/* prototypes */
void adc_init();
void adc_channel(uint8_t channel, uint8_t bandgap_ref);
uint8_t adc_start(uint8_t reread, uint8_t enable_averaging);


#endif /* ADC_H_ */