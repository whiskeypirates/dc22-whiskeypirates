/*
 * led_matrix.c: main skull led matrix handling functions
 * 3414 true
 *
 * ----
 *
 * $Id: led_matrix.c 209 2014-08-03 20:29:17Z true $
 *
 * ----
 *
 * Resources:
 * - Uses TIM2 and TIM3 as PWM output; for frequency, see led_pwm.c
 */

#include "led_matrix.h"
#include "../device/lightsensor.h"


  // pwm devices
static TIM_TypeDef *led_pwm[2] 				= {TIM2, TIM3};

  // pwm pins
  // order is 0=white, 1=purple. row=high, col=low
static GPIO_TypeDef *led_row_port[2] 		= {GPIOA, GPIOC};
static uint16_t led_row_pin_mask[2] 		= {0x000f, 0x03c0};
static const uint8_t led_row_pin_low[2] 	= {0, 6};

static GPIO_TypeDef *led_col_port[2] 		= {GPIOC, GPIOC};
static uint16_t led_col_pin_mask[2] 		= {0xf000, 0x003c};
static const uint8_t led_col_pin_low[2] 	= {12, 2};

  // led mode
static uint8_t led_matrix_mode;
static uint8_t led_matrix_mode_idx;

  // led brightness
static uint8_t led_level[2][16];

  // led patterns
#include "led_matrix_pattern.h"
static LEDFixedPattern *led_pattern;
static const LEDFixedPattern *led_pattern_list[] = {
	led_pattern_01,
	led_pattern_02,
	led_pattern_03,
	led_pattern_04
};

static uint8_t led_pattern_step;
static uint16_t led_pattern_timeout;

  // led programs
#include "led_matrix_prog.h"
static void (*led_program)();
static void (*led_program_list[])() = {
	led_prog_debug,
	led_prog_loops,
	led_prog_loops_mic,
	led_prog_loops_rotate,
	led_prog_rand_on_rand_off,
	led_prog_mic_spl_meter
};
const char led_matrix_prog_name[LED_MATRIX_PROG_COUNT][16] = {
	{"Debug"},
	{"Loops"},
	{"Loops+Mic"},
	{"Loops+Rotate"},
	{"Random"},
	{"SPLMeter"},
};

  // currently lit LED
static uint8_t led_index;


/* functions */
void led_matrix_io_init()
{
	uint8_t i;
	GPIO_InitTypeDef gpio;

	// enable peripheral clocks
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

	// configure LOW as standard outputs
	gpio.GPIO_Mode = GPIO_Mode_OUT;
	gpio.GPIO_OType = GPIO_OType_PP;
	gpio.GPIO_Speed = GPIO_Speed_10MHz;

	gpio.GPIO_Pin = led_col_pin_mask[0];
	GPIO_Init(led_col_port[0], &gpio);

	gpio.GPIO_Pin = led_col_pin_mask[1];
	GPIO_Init(led_col_port[1], &gpio);

	// and set low
	GPIO_ResetBits(led_col_port[0], led_col_pin_mask[0]);
	GPIO_ResetBits(led_col_port[1], led_col_pin_mask[1]);

	// configure HIGH pins as AF
	gpio.GPIO_Mode = GPIO_Mode_AF;

	gpio.GPIO_Pin = led_row_pin_mask[0];
	GPIO_Init(led_row_port[0], &gpio);

	gpio.GPIO_Pin = led_row_pin_mask[1];
	GPIO_Init(led_row_port[1], &gpio);

	// set AF on HIGH pins
	for (i = 0; i < 4; i++) {
		GPIO_PinAFConfig(led_row_port[0], led_row_pin_low[0] + i, GPIO_AF_TIM2);
		GPIO_PinAFConfig(led_row_port[1], led_row_pin_low[1] + i, GPIO_AF_TIM3);
	}

	// MAKE SURE to enable PWM for these to work!
}

void inline led_matrix_next()
{
	uint8_t update_col;
	uint8_t update_row;
	uint16_t update_scaler;
	uint32_t update_level[2][4] = {
		//{0xff, 0xff, 0xff, 0xff},
		//{0xff, 0xff, 0xff, 0xff}
		{0x100, 0x100, 0x100, 0x100},
		{0x100, 0x100, 0x100, 0x100}
	};

	// set next index
	led_index++;
	led_index &= 0x0f;

	// set active column
	update_col = led_index % 4;
	update_row = led_index >> 2;

	// set new levels
	update_scaler = (settings.led_autoadjust & 0x80) ? lightsensor_get_scalerval(LIGHTSENS_SCALED_SKULL_WHT) : 256;
	update_level[0][update_row] = 0x100 - ((led_level[0][led_index] * update_scaler) >> 8);
	update_scaler = (settings.led_autoadjust & 0x80) ? lightsensor_get_scalerval(LIGHTSENS_SCALED_SKULL_PUR) : 256;
	update_level[1][update_row] = 0x100 - ((led_level[1][led_index] * update_scaler) >> 8);

	// disable PWM
	TIM_Cmd(led_pwm[0], DISABLE);
	TIM_Cmd(led_pwm[1], DISABLE);

	// set new PWM value for the active row
	led_pwm_set_oc(led_pwm[0], update_level[0]);
	led_pwm_set_oc(led_pwm[1], update_level[1]);

	// reset pwm counter (fixes matrix glitches)
	TIM_SetCounter(led_pwm[0], 0xff);
	TIM_SetCounter(led_pwm[1], 0xff);

	// generate update event to apply pwm values
	TIM_GenerateEvent(led_pwm[0], TIM_EventSource_Update);
	TIM_GenerateEvent(led_pwm[1], TIM_EventSource_Update);

	// clear all columns
	// we do this after setting PWM to fix glitchy other LEDs lighting
	GPIO_ResetBits(led_col_port[0], led_col_pin_mask[0]);
	GPIO_ResetBits(led_col_port[1], led_col_pin_mask[1]);

	// and now set the active column
	GPIO_SetBits(led_col_port[0], (1 << (led_col_pin_low[0] + update_col)));
	GPIO_SetBits(led_col_port[1], (1 << (led_col_pin_low[1] + update_col)));

	// re-enable PWM
	TIM_Cmd(led_pwm[0], ENABLE);
	TIM_Cmd(led_pwm[1], ENABLE);
}

void led_matrix_set_mode(uint8_t mode)
{
	led_matrix_mode = mode;
}

void led_matrix_set_pattern(uint8_t pattern_idx)
{
	// we need to be in pattern mode
	led_matrix_set_mode(LED_MATRIX_MODE_PATTERN);
	// update the pattern pointer
	led_pattern = (LEDFixedPattern *)led_pattern_list[pattern_idx];
	led_matrix_mode_idx = pattern_idx;
	// and reset the pattern settings
	led_pattern_step = led_pattern_size[pattern_idx];
	led_pattern_timeout = 0;
}

void led_matrix_set_program(uint8_t program_idx, uint8_t init,
		uint16_t wait, uint16_t level, uint32_t offset, uint32_t settings)
{
	// we need to be in program mode
	led_matrix_set_mode(LED_MATRIX_MODE_PROGRAM);
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

uint8_t led_matrix_get_mode()
{
	return led_matrix_mode;
}

void led_matrix_mode_update()
{
	uint8_t i;
	uint32_t *t;

	switch (led_matrix_mode) {
		case LED_MATRIX_MODE_PATTERN: {
			if (led_pattern_timeout) {
				// timing out to update next pattern
				led_pattern_timeout--;
			} else {
				// set next pattern
				led_pattern_step++;
				// beyond the end of the pattern? restart it
				if (led_pattern_step >= led_pattern_size[led_matrix_mode_idx]) {
					led_pattern_step = 0;
				}

				// load next timeout value
				led_pattern_timeout = led_pattern[led_pattern_step].wait * 5;

				// load pattern into pwm level variables
				for (i = 0; i < 4; i++) {
					t = (uint32_t *)&led_level[LED_MATRIX_WHITE][i << 2];
					*t = led_pattern[led_pattern_step].led[i];

					t = (uint32_t *)&led_level[LED_MATRIX_PURPLE][i << 2];
					*t = led_pattern[led_pattern_step].led[i + 4];
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
			for (i = 0; i < 16; i++) {
				led_level[LED_MATRIX_WHITE][i] = 0;
				led_level[LED_MATRIX_PURPLE][i] = 0;
			}
			break;
		}
	}
}
