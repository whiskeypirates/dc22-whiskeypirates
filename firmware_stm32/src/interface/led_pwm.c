/*
 * led_pwm.c: led pwm configuration functions
 * 2014 true
 *
 * ----
 *
 * $Id: led_pwm.c 214 2014-08-04 05:31:33Z true $
 */


/* pwm */
#include "led_pwm.h"


/* functions */
void led_pwm_init(TIM_TypeDef *timer, uint16_t polarity, uint8_t channel_mask)
{
	TIM_TimeBaseInitTypeDef tim_tb;
	TIM_OCInitTypeDef tim_oc;

	// set up with 250 levels of brightness
	tim_tb.TIM_Prescaler = 5 - 1; 		// 6.4MHz
	tim_tb.TIM_Period = 256 - 1; 		// 25KHz PWM, 5 updates/LED
	tim_tb.TIM_ClockDivision = 0;
	tim_tb.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(timer, &tim_tb);

	// configure outputs as pwm
	tim_oc.TIM_OCMode = TIM_OCMode_PWM1;
	tim_oc.TIM_OutputState = TIM_OutputState_Enable;
	tim_oc.TIM_Pulse = 0;
	tim_oc.TIM_OCPolarity = polarity;

	// configure pin for PWM output mode, enable preload
	if (channel_mask & 0x01) {
		TIM_OC1Init(timer, &tim_oc);
		TIM_OC1PreloadConfig(timer, TIM_OCPreload_Enable);
	}
	if (channel_mask & 0x02) {
		TIM_OC2Init(timer, &tim_oc);
		TIM_OC2PreloadConfig(timer, TIM_OCPreload_Enable);
	}
	if (channel_mask & 0x04) {
		TIM_OC3Init(timer, &tim_oc);
		TIM_OC3PreloadConfig(timer, TIM_OCPreload_Enable);
	}
	if (channel_mask & 0x08) {
		TIM_OC4Init(timer, &tim_oc);
		TIM_OC4PreloadConfig(timer, TIM_OCPreload_Enable);
	}

	// finally, enable the timer
	TIM_ARRPreloadConfig(timer, ENABLE);
	TIM_Cmd(timer, ENABLE);
}

void led_pwm_init_all()
{
	// enable peripheral clock and configure timers with PWM settings

	// main led matrix
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	led_pwm_init(TIM2, TIM_OCPolarity_High,
			LED_PWM_CH1 | LED_PWM_CH2 | LED_PWM_CH3 | LED_PWM_CH4);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	led_pwm_init(TIM3, TIM_OCPolarity_High,
			LED_PWM_CH1 | LED_PWM_CH2 | LED_PWM_CH3 | LED_PWM_CH4);

	// bone led matrix
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11, ENABLE);
	led_pwm_init(TIM11, TIM_OCPolarity_High, LED_PWM_CH1);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);
	led_pwm_init(TIM9, TIM_OCPolarity_High, LED_PWM_CH1 | LED_PWM_CH2);
}

void led_pwm_set_oc(TIM_TypeDef *timer, uint32_t *oc)
{
	timer->CCR1 = *oc++;

	if (timer == TIM9) {
		timer->CCR2 = *oc++;
	} else if (timer == TIM2 || timer == TIM3 || timer == TIM4) {
		timer->CCR2 = *oc++;
		timer->CCR3 = *oc++;
		timer->CCR4 = *oc;
	}
}
