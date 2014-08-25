/*
 * led_matrix_prog.h: programs for the RGBLED eyes
 * 2014 by true
 *
 * ----
 *
 * $Id: led_eyes_prog.h 222 2014-08-07 18:15:09Z true $
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
 * candle flicker
 * option sets the color mix; each 8 bits set the mix level in RGBx
 *     (so 0xff0000?? would be red, etc)
 * xx option bits determine if independent (1 = yes, 0 = no)
*********/
static void led_prog_candle_flicker()
{
	static const uint8_t lookup[] = {20, 52, 84, 116, 148, 184, 220, 255};
	uint8_t rand = 0;
	uint8_t new_val[3];

	uint8_t i;

	// ideally we want to update flicker at about 454.54hz (as close to 440hz as we can get) = 11
	// I don't know where I read this so let's just update LEDs less often because it looks nicer

	if (!led_prog_wait) {
		led_prog_wait = 45;

		// set which LED to update if in alternating mode
		led_prog_state[0] = led_prog_state[0] ? 0 : 1;

		rand = pirate_prng() & 0x1f;
		rand = (rand > 7) ? lookup[7] : lookup[rand];

		// create scaled value
		new_val[0] = ((led_prog_set_option >> 24));
		new_val[1] = ((led_prog_set_option >> 16) & 0xff);
		new_val[2] = ((led_prog_set_option >>  8) & 0xff);

		for (i = 0; i < 3; i++) {
			new_val[i] = (rand * new_val[i]) >> 8;
		}

		// update LED values
		if (led_prog_set_option & 1) {
			led_level[led_prog_state[0]][0] = new_val[0];
			led_level[led_prog_state[0]][1] = new_val[1];
			led_level[led_prog_state[0]][2] = new_val[2];
		} else {
			led_level[0][0] = led_level[1][0] = new_val[0];
			led_level[0][1] = led_level[1][1] = new_val[1];
			led_level[0][2] = led_level[1][2] = new_val[2];
		}
	}

	led_prog_wait--;
}

static void led_prog_candle_flicker_favcolor()
{
	led_prog_set_option = settings.fav_color[0] << 24 | settings.fav_color[1] << 16 |
			settings.fav_color[2] << 8 | (led_prog_set_option & 0x01);

	led_prog_candle_flicker();
}

/********
 * static led or random led flasher
 * option sets the color mix; each 8 bits set the mix level in RGBx
 *     (so 0xff0000?? would be red, etc)
 * low bit of option sets LEDs in independent mode (1 = yes, 0 = no)
 * offset sets the flash color mix
 * low 8 bits of offset set the threshold of the flasher (0x00=always, 0xff=never)
 * having eyes on while set to always activate will simply alternate the LEDs
*********/
static void led_prog_randflasher()
{
	uint8_t i;

	if (!led_prog_wait) {
		led_prog_wait = led_prog_set_wait * 5;

		// set which LED to operate
		led_prog_state[0] = led_prog_state[0] ? 0 : 1;

		// set background levels on all LEDs
		for (i = 0; i < 2; i++) {
			led_level[i][0] = ((led_prog_set_option >> 24));
			led_level[i][1] = ((led_prog_set_option >> 16) & 0xff);
			led_level[i][2] = ((led_prog_set_option >>  8) & 0xff);
		}

		// set flash
		if (pirate_prng() > (led_prog_set_offset & 0xff)) {
			if (!(led_prog_set_option & BIT_0) || (led_prog_state[0] == 0)) {
				led_level[0][0] = ((led_prog_set_offset >> 24));
				led_level[0][1] = ((led_prog_set_offset >> 16) & 0xff);
				led_level[0][2] = ((led_prog_set_offset >>  8) & 0xff);
			}
			if (!(led_prog_set_option & BIT_0) || (led_prog_state[0] == 1)) {
				led_level[1][0] = ((led_prog_set_offset >> 24));
				led_level[1][1] = ((led_prog_set_offset >> 16) & 0xff);
				led_level[1][2] = ((led_prog_set_offset >>  8) & 0xff);
			}
		}
	}

	led_prog_wait--;
}

/********
 * mic spl meter
 * level: sets the threshold scaler. 0xff = 100 is max, 0x80 = 50 is max, and so on
 * offset: sets the cold (0%) color, see above for format
 * option: sets the hot (100%) color, see above for format
 * last 7 bits of option sets how fast the ramping up is
 * last 7 bits of offset sets how fast the ramping down is
 *********/
static void led_prog_mic_spl()
{
	uint8_t i;

	if (!led_prog_wait) {
		led_prog_wait = led_prog_set_wait * 5;

		// first run?
		if (led_prog_set_option & 0x80) {
			led_prog_work[0] = 0;
			led_prog_set_option |= 0x80;
		}

		// speed is set?
		if (!(led_prog_set_option & 0x7f)) {
			led_prog_set_option |= 0x0f;
		}
		if (!(led_prog_set_offset & 0x7f)) {
			led_prog_set_offset |= 0x01;
		}

		// set our value
		if (mic_peak > led_prog_work[0]) {
			led_prog_work[0] += led_prog_set_option & 0xff;
			if (led_prog_work[0] > mic_peak) led_prog_work[0] = mic_peak;
		} else {
			led_prog_work[0] -= led_prog_set_offset & 0xff;
			if (led_prog_work[0] < mic_peak) led_prog_work[0] = mic_peak;
		}

		// make it a percentage
		led_prog_work[1] = pirate_scale(led_prog_work[0], settings.mic_cal[0], settings.mic_cal[1], 0, 255);

		// scale value
		led_prog_state[0] = pirate_scale(led_prog_work[1], (led_prog_set_level >> 8), (led_prog_set_level & 0xff), 0, 248);

		// scale hot/cold levels on LEDs
		for (i = 0; i < 3; i++) {
			led_level[0][i] = led_level[1][i] = pirate_scale(led_prog_state[0], 0, 248,
					((led_prog_set_offset >> ((3 - i) << 3)) & 0xff),
					((led_prog_set_option >> ((3 - i) << 3)) & 0xff));
		}
	}

	led_prog_wait--;
}
