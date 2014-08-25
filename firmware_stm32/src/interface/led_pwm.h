/*
 * led_pwm.h: led pwm configuration prototypes
 * 2014 true
 *
 * ----
 *
 * $Id: led_pwm.h 214 2014-08-04 05:31:33Z true $
 */

#ifndef __PIRATE_LED_PWM_H
#define __PIRATE_LED_PWM_H


#include "pirate.h"


#define LED_PWM_CH1 	0x01
#define LED_PWM_CH2 	0x02
#define LED_PWM_CH3 	0x04
#define LED_PWM_CH4 	0x08


/* prototypes */
void led_pwm_init_all();
void led_pwm_set_oc(TIM_TypeDef *timer, uint32_t *oc);


#endif
