/*
 * adc.h: manager for anything adc-related
 * 2014 by true
 *
 * ----
 *
 * $Id: adc.c 214 2014-08-04 05:31:33Z true $
 */


#include <stdlib.h>
#include <stdint.h>

#include "adc.h"

#include "gpio.h"


static const tGPIO adc_gpio[] = {
	{GPIOA, GPIO_Pin_5, 5}, 	// mic sig detect
	{GPIOB, GPIO_Pin_0, 0}, 	// mic peak detect
	{GPIOB, GPIO_Pin_0, 1}, 	// battery voltage
};
static uint8_t adc_chan;
uint16_t adc_result[ADC_MAX_RESULT_COUNT];


/* functions */
void adc_init()
{
	GPIO_InitTypeDef gpio;
	ADC_CommonInitTypeDef adc_com;
	ADC_InitTypeDef adc_init;
	NVIC_InitTypeDef nvic;

	int i;


	// make sure the HSI is turned on - STM32L1xx uses HSI only for ADC
	RCC_HSICmd(ENABLE);

	// enable ADC clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	// set up pins as analog inputs
	// NOTE: clocks were likely enabled earlier. fix if they weren't.
	gpio.GPIO_Mode = GPIO_Mode_AN;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;

	for (i = 0; i < 3; i++) {
		gpio.GPIO_Pin = adc_gpio[i].pin;
		GPIO_Init(adc_gpio[i].port, &gpio);
	}

	// configure ADC initial values
	ADC_CommonStructInit(&adc_com);
	ADC_StructInit(&adc_init);
	adc_init.ADC_ExternalTrigConv = 0;

	// set up channels to read
	ADC_RegularChannelConfig(ADC1, ADC_CHAN_MIC_SIG, ADC_READ_MIC_SIG + 1, ADC_SampleTime_4Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_CHAN_MIC_PEAK, ADC_READ_MIC_PEAK + 1, ADC_SampleTime_4Cycles);
	ADC_RegularChannelConfig(ADC1, ADC_CHAN_BATT_VOLTAGE, ADC_READ_BATT_VOLTAGE + 1, ADC_SampleTime_4Cycles);
	adc_init.ADC_ScanConvMode = ENABLE;
	adc_init.ADC_NbrOfConversion = 3;

	// enable end of conversion on each channel read
	ADC_EOCOnEachRegularChannelCmd(ADC1, ENABLE);

	// freeze ADC until data has been read
	ADC_DelaySelectionConfig(ADC1, ADC_DelayLength_Freeze);

	// initialize the ADC
	ADC_CommonInit(&adc_com);
	ADC_Init(ADC1, &adc_init);

	// it doesn't look like STM32L100 has auto-calibrate feature,
	// but if it did, we'd set it up here. baw.

	// set up ADC power saving
	ADC_PowerDownCmd(ADC1, ADC_PowerDown_Idle_Delay, ENABLE);

	// enable interrupt
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

	// set interrupt priority
	nvic.NVIC_IRQChannel = ADC1_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 3;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	// finally, turn on the ADC
	ADC_Cmd(ADC1, ENABLE);
}

void adc_deinit()
{
	// reset ADC
	ADC_DeInit(ADC1);

	// and disable ADC clock
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
}

void adc_start()
{
	uint8_t adc_timeout = 200;

	adc_chan = 0;

	// verify ADC is powered up
	while (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET) {
		ADC_Cmd(ADC1, ENABLE);
		if (--adc_timeout == 0) {
			adc_result[2] |= 0xf000;
			break;
		}
	}

	// then start conversion if ADC is on
	if (ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == SET) {
		ADC_SoftwareStartConv(ADC1);
	}
}

void ADC1_IRQHandler()
{
	if (adc_chan < ADC_MAX_RESULT_COUNT) {
		adc_result[adc_chan] = ADC_GetConversionValue(ADC1);
		adc_chan++;
	}

	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
}
