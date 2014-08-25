/*
 * adc.h: prototypes for adc functions
 * 2014 by true
 *
 * ----
 *
 * $Id: adc.h 214 2014-08-04 05:31:33Z true $
 */

#ifndef __PIRATE_IF_ADC_H
#define __PIRATE_IF_ADC_H


#include "pirate.h"


#define ADC_MAX_RESULT_COUNT 	3 		// channels in use

#define ADC_READ_MIC_SIG 		0
#define ADC_READ_MIC_PEAK 		1
#define ADC_READ_BATT_VOLTAGE 	2

#define ADC_CHAN_MIC_SIG 		ADC_Channel_5
#define ADC_CHAN_MIC_PEAK 		ADC_Channel_8
#define ADC_CHAN_BATT_VOLTAGE 	ADC_Channel_9


extern uint16_t adc_result[ADC_MAX_RESULT_COUNT];


void adc_init();
void adc_deinit();

void adc_channel(uint8_t rank);
void adc_start();


#endif
