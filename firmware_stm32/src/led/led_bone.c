/*
 * led_bone.c: skull crossbones led matrix handling functions
 * 2014 true
 *
 * ----
 *
 * $Id: led_bone.c 211 2014-08-03 21:06:13Z true $
 *
 * ----
 *
 * Resources:
 * - Uses TIM11 OC1 and TIM9 OC1 and OC2 for PWM output; for frequency, see led_pwm.c
 *
 * TODO:
 * - use PWM for bone LEDs
 * - integrate with main matrix patterns somehow
 */

#include "led_bone.h"
#include "../device/lightsensor.h"


  // devices
static TIM_TypeDef *led_bone_row_pwm[3] = {TIM11, TIM9, TIM9};
static const tGPIO led_bone_row_gpio[3] = {
	{GPIOA, GPIO_Pin_7, 7},
	{GPIOB, GPIO_Pin_13, 13},
	{GPIOB, GPIO_Pin_14, 14}
};
static const tGPIO led_bone_col_gpio[3] = {
	{GPIOB, GPIO_Pin_12, 12},
	{GPIOB, GPIO_Pin_15, 15},
	{GPIOA, GPIO_Pin_8, 8}
};

const uint8_t led_bone_order[8] = {
	BONE_LED_TR_UPPER,
	BONE_LED_TR_LOWER,
	BONE_LED_BR_UPPER,
	BONE_LED_BR_LOWER,
	BONE_LED_BL_LOWER,
	BONE_LED_BL_UPPER,
	BONE_LED_TL_LOWER,
	BONE_LED_TL_UPPER
};

  // pwm level
static uint8_t led_bone_mode;
static uint8_t led_bone_mode_idx;
static uint8_t led_level[9];

  // patterns
#include "led_bone_pattern.h"
static LEDBonePattern *led_pattern;
static const LEDBonePattern *led_pattern_list[] = {
	led_pattern_01,
	led_pattern_02,
	led_pattern_03
};

static uint8_t led_pattern_step;
static uint16_t led_pattern_timeout;

  // led programs
#include "led_bone_prog.h"
static void (*led_program)();
static void (*led_program_list[])() = {
	led_prog_all_on,
	led_prog_loops,
	led_prog_updown
};
const char led_bone_prog_name[LED_BONE_PROG_COUNT][16] = {
	{"AllOn"},
	{"Loops"},
	{"UpDown"},
};

  // currently lit led
static uint8_t led_index = 0;


/* functions */
void led_bone_io_init()
{
	uint8_t i;
	GPIO_InitTypeDef gpio;

	// enable peripheral clocks
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	// configure LOW as standard outputs
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;

	// init and set low
	for (i = 0; i < 3; i++) {
		gpio.GPIO_Pin = led_bone_col_gpio[i].pin;
		GPIO_Init(led_bone_col_gpio[i].port, &gpio);
		GPIO_ResetBits(led_bone_col_gpio[i].port, led_bone_col_gpio[i].pin);
	}

	// configure HIGH pins as AF
	gpio.GPIO_Mode = GPIO_Mode_AF;

	for (i = 0; i < 3; i++) {
		gpio.GPIO_Pin = led_bone_row_gpio[i].pin;
		GPIO_Init(led_bone_row_gpio[i].port, &gpio);
	}

	// configure AF on these pins
	GPIO_PinAFConfig(led_bone_row_gpio[0].port, led_bone_row_gpio[0].pinsource, GPIO_AF_TIM11);
	GPIO_PinAFConfig(led_bone_row_gpio[1].port, led_bone_row_gpio[1].pinsource, GPIO_AF_TIM9);
	GPIO_PinAFConfig(led_bone_row_gpio[2].port, led_bone_row_gpio[2].pinsource, GPIO_AF_TIM9);
}

void inline led_bone_next()
{
	uint8_t update_col;
	uint8_t update_row;
	uint16_t update_scaler;

	uint32_t update_level[3] = {
		0, 0, 0
	};

	// set active column
	update_col = led_index % 3;

	// set updated row
	if (led_index < 3) {
		update_row = 0;
	} else if (led_index < 6) {
		update_row = 1;
	} else {
		update_row = 2;
	}

	// set new levels
	// update_level[update_row] = *(led_level + led_index);
	update_scaler = (settings.led_autoadjust & 0x80) ? lightsensor_get_scalerval(LIGHTSENS_SCALED_BONES) : 256;
	update_level[update_row] = (led_level[led_index] * update_scaler) >> 8;

	// disable PWM
	TIM_Cmd(led_bone_row_pwm[0], DISABLE);
	TIM_Cmd(led_bone_row_pwm[1], DISABLE);

	// clear all columns (set high)
	GPIO_SetBits(led_bone_col_gpio[0].port, led_bone_col_gpio[0].pin);
	GPIO_SetBits(led_bone_col_gpio[1].port, led_bone_col_gpio[1].pin);
	GPIO_SetBits(led_bone_col_gpio[2].port, led_bone_col_gpio[2].pin);

	// set new row pwm value
	led_pwm_set_oc(led_bone_row_pwm[0], &update_level[0]); 	// TIM11 has pin0
	led_pwm_set_oc(led_bone_row_pwm[1], &update_level[1]); 	// TIM9 has pin1, pin2

	// reset pwm counter (fixes matrix glitches)
	TIM_SetCounter(led_bone_row_pwm[0], 0xff);
	TIM_SetCounter(led_bone_row_pwm[1], 0xff);

	// generate update event to apply pwm values
	TIM_GenerateEvent(led_bone_row_pwm[0], TIM_EventSource_Update);
	TIM_GenerateEvent(led_bone_row_pwm[1], TIM_EventSource_Update);

	// and now set the active column (set low)
	GPIO_ResetBits(led_bone_col_gpio[update_col].port, led_bone_col_gpio[update_col].pin);

	// re-enable PWM
	TIM_Cmd(led_bone_row_pwm[0], ENABLE);
	TIM_Cmd(led_bone_row_pwm[1], ENABLE);

	// next led
	led_index++;
	if (led_index >= 9) {
		led_index = 0;
	}
}

void led_bone_set_mode(uint8_t mode)
{
	led_bone_mode = mode;
}

void led_bone_set_pattern(uint8_t pattern_idx)
{
	// we need to be in pattern mode
	led_bone_set_mode(LED_MATRIX_MODE_PATTERN);
	// update the pattern pointer
	led_pattern = (LEDBonePattern *)led_pattern_list[pattern_idx];
	led_bone_mode_idx = pattern_idx;
	// and reset the pattern settings
	led_pattern_step = led_pattern_size[pattern_idx];
	led_pattern_timeout = 0;
}

void led_bone_set_program(uint8_t program_idx, uint8_t init,
		uint16_t wait, uint16_t level, uint32_t offset, uint32_t settings)
{
	// we need to be in program mode
	led_bone_set_mode(LED_MATRIX_MODE_PROGRAM);
	// update the program pointer
	led_program = led_program_list[program_idx];

	// set initial program variables
	led_prog_set_wait = wait;
	led_prog_set_level = level;
	led_prog_set_offset = offset;
	led_prog_set_option = settings;

	// is this a new set? if so, initialize program parameters
	if (init) {
		uint8_t i;

		led_prog_wait = 0;
		for (i = 0; i < 4; i++) {
			led_prog_state[i] = 0;
			led_prog_work[i] = 0;
		}
	}
}

void led_bone_mode_update()
{
	uint8_t i;

	switch (led_bone_mode) {
		case LED_MATRIX_MODE_PATTERN: {
			if (led_pattern_timeout) {
				// timing out to update next pattern
				led_pattern_timeout--;
			} else {
				// set next pattern
				led_pattern_step++;
				// beyond the end of the pattern? restart it
				if (led_pattern_step >= led_pattern_size[led_bone_mode_idx]) {
					led_pattern_step = 0;
				}

				// load next timeout value
				led_pattern_timeout = led_pattern[led_pattern_step].wait * 5;

				// load pattern into pwm level variables
				for (i = 0; i < 9; i++) {
					led_level[i] = led_pattern[led_pattern_step].led[i];
				}
			}
			break;
		}
		case LED_MATRIX_MODE_PROGRAM: {
			// if we have a valid program loaded, run it
			if (led_program != NULL) {
				led_program();
			}
			break;
		}

		case LED_MATRIX_MODE_OFF: {
			led_program = NULL;
			for (i = 0; i < 9; i++) {
				led_level[i] = 0;
			}
			break;
		}
	}
}
