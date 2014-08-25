/*
 * led_matrix_prog.h: programs for the main LED matrix
 * 2014 by true
 *
 * ----
 *
 * $Id: led_matrix_prog.h 223 2014-08-12 17:03:46Z true $
 */


#include "gpio.h"

  // global settings
PirateSettings settings;

  // misc shit
uint16_t mic_peak;

  // program variables
static uint16_t led_prog_wait; 		// used by function to keep track of iteration time
static uint32_t led_prog_state[4]; 	// used by the function to keep track of lit LEDs, state
static uint32_t led_prog_work[4]; 	// used for misc shit

static uint16_t led_prog_set_wait; 	// used by setter function to set iteration time in ms
static uint16_t led_prog_set_level;	// used by setter function to set desired level
static uint32_t led_prog_set_offset;// used by setter function to set desired offset
static uint32_t led_prog_set_option;// used by setter to set options


/* programs */
/********
 * debug light output
 * uses white LEDs as debug output.
 * set the offset to the LED mask for LEDs you want lit.
 * set the wait time to the time to stay lit (15000 is a good value).
*********/
static void led_prog_debug()
{
	uint8_t i;

	if (led_prog_set_wait) {
		led_prog_wait = led_prog_set_wait;
		led_prog_set_wait = 0;
		led_prog_state[0] = led_prog_set_offset;
	}

	if (led_prog_wait) {
		led_prog_wait--;
	}

	for (i = 0; i < 16; i++) {
		if (led_prog_state[0] & (1 << i)) {
			led_level[0][i] = led_prog_wait ? (led_prog_set_level & 0xff) : 0;
		} else {
			led_level[0][i] = 0;
		}
	}
}
/********
 * around the skull in a loop program
 * speed: 	set as desired
 * options: bit 0 = white enable, bit 1 = purple enable,
 * 			bit 2 = white direction (0 = clockwise, 1 = counter-clockwise),
 * 			bit 3 = purple direction (0 = clockwise, 1 = counter-clockwise),
 * 			bit 4 = decay trails faster, bit 5 = decay trails even faster,
 *          bit 6 = trails enable on white, bit 7 = trails enable on purple
 *********/
static void led_prog_loops()
{
	uint8_t i;

	// timeout is done?
	if (!led_prog_wait) {
		// load new timer wait value
		led_prog_wait = led_prog_set_wait * 5;

		// set decay rate (LED PWM active values are right shifted by this value)
		led_prog_work[0] = 1;
		if (led_prog_set_option & BIT_4) led_prog_work[0]++;
		if (led_prog_set_option & BIT_5) led_prog_work[0] += 2;

		// set initial LED offsets
		if (!led_prog_work[3]) {
			led_prog_state[0] = led_prog_set_offset & 0x0f;
			led_prog_state[1] = (led_prog_set_offset >> 4) & 0x0f;
			led_prog_work[3] = 1;
		}

		// set LED PWM levels
		for (i = 0; i < 16; i++) {
			if ((i == led_prog_state[0]) && (led_prog_set_option & BIT_0)) {
				// LED is active and enabled; set to desired level.
				led_level[0][i] = led_prog_set_level & 0xff;
			} else {
				// led is not active - decay if trails on, otherwise turn off
				led_level[0][i] = (led_prog_set_option & BIT_6) ?
					led_level[0][i] >> led_prog_work[0] : 0;
			}

			if ((i == led_prog_state[1]) && (led_prog_set_option & BIT_1)) {
				// LED is active and enabled; set to desired level.
				led_level[1][i] = led_prog_set_level >> 8;
			} else {
				// led is not active - decay if trails on, otherwise turn off
				led_level[1][i] = (led_prog_set_option & BIT_7) ?
						led_level[1][i] >> led_prog_work[0] : 0;
			}
		}

		// update program state with the next LED
		for (i = 0; i < 2; i++) {
			led_prog_state[i] += (led_prog_set_option & (1 << (i + 2))) ? 15 : 1;
			led_prog_state[i] &= 0x0f;
		}
	}

	// bide our time...
	led_prog_wait--;
}

/********
 * around the skull in a loop program modulated by microphone
 * speed: 	set as desired
 * options: see above program
 *********/
static void led_prog_loops_mic()
{
	static uint8_t delay = 0;
	uint16_t low, high;

	low = (led_prog_set_offset >> 8) & 0xff;
	high = (led_prog_set_offset >> 16) & 0xff;

	led_prog_state[3] = pirate_scale(mic_peak, settings.mic_cal[0], settings.mic_cal[1], 255, 0);
	led_prog_state[3] = pirate_scale(led_prog_state[3], low, high, 1, 120);

	// variable decay
	if (!delay) {
		delay = (led_prog_set_offset >> 24);
		if (!delay) delay = 1;
		if (led_prog_set_wait < led_prog_state[3]) {
			led_prog_set_wait++;
		}
	}

	// always have fast attack
	if (led_prog_set_wait > led_prog_state[3]) {
		led_prog_set_wait = led_prog_state[3];
	}

	delay--;

	led_prog_loops();
}

/********
 * around the skull in a loop program that rotates
 * most useful for oppositing direction loops so that the
 * point of contact changes
 * speed: 	set as desired
 * options: see above program, with the following:
 *          high 12 option bits: time between rotating in ms
 *          bits[19:16]: specify direction (1-15 = move pos cw)
 *********/
static void led_prog_loops_rotate()
{
	static uint16_t wait = 0;
	uint8_t t;

	if (!wait) {
		wait = (led_prog_set_option >> 20) * 5;
		if (!wait) wait = 1;

		t = (led_prog_set_option >> 16 & 0x0f);

		if (t >= 1 && t <= 15) {
			led_prog_state[0] += t;
			led_prog_state[0] &= 0x0f;

			led_prog_state[1] += t;
			led_prog_state[1] &= 0x0f;
		}
	}

	wait--;
	led_prog_loops();
}


/********
 * random on, random off
 * speed: 	set as desired
 * options: bit 7 = fade off instead of turning off, bit 6 = fade faster
 *          bit 5 = fade even faster, bit 4 = fade faster still
 *          bit 1 = enable purple, bit 0 = enable white
 *********/
static void led_prog_rand_on_rand_off()
{
	int i;
	uint8_t fade;

	if (!led_prog_wait) {
		// load new timer wait value
		led_prog_wait = led_prog_set_wait * 5;

		// is this a new run?
		if (!led_prog_state[3]) {
			led_prog_state[3] = 1;

			for (i = 0; i < 16; i++) {
				led_level[LED_MATRIX_WHITE][i] = 0;
				led_level[LED_MATRIX_PURPLE][i] = 0;
			}
		}

		led_prog_state[0] = led_prog_state[0] ? 0 : 1;

		fade = 0;
		if (led_prog_set_option & 0x80) fade = 2;
		if (led_prog_set_option & 0x40) fade += 6;
		if (led_prog_set_option & 0x20) fade += 12;
		if (led_prog_set_option & 0x10) fade += 24;

		for (i = 0; i < 16; i++) {
			if (fade) {
				if (led_level[LED_MATRIX_WHITE][i] > fade) {
					led_level[LED_MATRIX_WHITE][i] -= fade;
				} else {
					led_level[LED_MATRIX_WHITE][i] = 0;
				}

				if (led_level[LED_MATRIX_PURPLE][i] > fade) {
					led_level[LED_MATRIX_PURPLE][i] -= fade;
				} else {
					led_level[LED_MATRIX_PURPLE][i] = 0;
				}
			} else {
				led_level[LED_MATRIX_WHITE][i] = 0;
				led_level[LED_MATRIX_PURPLE][i] = 0;
			}
		}

		if (led_prog_set_option & (1 << led_prog_state[0])) {
			led_level[led_prog_state[0]][pirate_prng() % 16] = led_prog_set_level >> (led_prog_state[0] << 3);
		}
	}

	led_prog_wait--;
}

/********
 * impulse spl meter
 * speed: 	ignored
 * options:	bits7-0 = sensitivity
 *********/
static void led_prog_mic_spl_meter()
{
	int i;
	uint8_t set[2];

	if (!led_prog_wait) {
		led_prog_wait = led_prog_set_wait * 5;

		if (led_prog_work[0] > mic_peak) {
			led_prog_work[0] -= 4;
		} else {
			led_prog_work[0] = mic_peak;
		}

		// scale peak value to calculated value and invert it
		led_prog_state[3] = pirate_scale(led_prog_work[0], settings.mic_cal[0], settings.mic_cal[1], 0, 255);

		// and invert it
		led_prog_state[3] ^= 0xff;

		// set defaults high level if not set
		if (!(led_prog_set_option >> 24)) led_prog_set_option |= (0xff << 24);

		// stretch value with constrained bounds to original bounds
		led_prog_state[3] = pirate_scale(led_prog_state[3],
				(led_prog_set_option >> 16) & 0xff, (led_prog_set_option >> 24), 0, 255);

		// finally determine which LEDs to light
		led_prog_state[3] = pirate_scale(led_prog_state[3], 0, 255, 0, 8);

		// then set them
		for (i = 0; i < 8; i++) {
			if (led_prog_state[3] < i) {
				set[0] = led_prog_set_level & 0xff;
				set[1] = led_prog_set_level >> 8;
			} else if (led_prog_state[3] == i) {
				set[0] = (led_prog_set_level & 0xff) >> 1;
				set[1] = led_prog_set_level >> 9;
			} else {
				set[0] = 0;
				set[1] = 0;
			}

			led_level[LED_MATRIX_WHITE][i] = set[0];
			led_level[LED_MATRIX_WHITE][15 - i] = set[0];

			led_level[LED_MATRIX_PURPLE][i] = set[1];
			led_level[LED_MATRIX_PURPLE][15 - i] = set[1];
		}
	}

	led_prog_wait--;
}
