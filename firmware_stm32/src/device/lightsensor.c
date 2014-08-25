/*
 * lightsensor.c: control of the attiny's light sensor output plus helper functions
 * 2014 true
 *
 * ----
 *
 * $Id: lightsensor.c 217 2014-08-05 03:38:42Z true $
 */


#include "lightsensor.h"
#include "attiny.h"


  // light variables
uint8_t light_level;
uint8_t light_gain;

static uint8_t wait = 0;

  // target brightness
  // TODO: make this adjustable?
static uint8_t lev_target[5][5] = {
		{0x00, 0xff, 0xe0, 0x55, 0x0d}, 	// skull (white)
		{0x00, 0x38, 0xff, 0xff, 0xff}, 	// skull (purple)
		{0x00, 0xff, 0xe0, 0x55, 0x0d}, 	// bones
		{0xff, 0xff, 0xf0, 0xe0, 0xc0}, 	// eyes
		{0x00, 0xff, 0xe0, 0x55, 0x14} 		// backlight
};

static uint8_t lev_set[5];


/* functions */
  // gain
uint8_t lightsensor_gainvalue_update()
{
	if (light_level <= settings.led_autogain_lev_min) {
		if (light_gain > LIGHTSENS_GAIN_MIN) {
			light_gain--;
			return 1;
		}
	} else if (light_level >= settings.led_autogain_lev_max) {
		if (light_gain < LIGHTSENS_GAIN_MAX) {
			light_gain++;
			return 2;
		}
	}

	return 0;
}

  // scaling
void lightsensor_scale_level(uint8_t idx)
{
	int16_t scaled = 0;
	uint8_t i = 5;

	if (light_gain) {
		if (light_gain == settings.led_autothresh[0]) {
			scaled = lev_target[idx][0];
			i = 1;
		} else {
			for (i = 1; i < 5; i++) {
				if (light_gain <= settings.led_autothresh[i]) {
					scaled = pirate_scale(light_gain,
							settings.led_autothresh[i - 1],
							settings.led_autothresh[i],
							lev_target[idx][i - 1],
							lev_target[idx][i]);

					break;
				}
			}
		}

		if (i < 5) {
			if (lev_set[idx] > scaled) {
				lev_set[idx]--;
			} else if (lev_set[idx] < scaled) {
				lev_set[idx]++;
			}
		}
	}
}

void lightsensor_scaler_update()
{
	int i;

	if (!wait) {
		wait = settings.led_autoadjust & 0x7f;

		for (i = 0; i < 5; i++) {
			lightsensor_scale_level(i);
		}

		// if the scaling speed isn't set, set it fast
		if (!wait) wait = 1;
	}

	wait--;
}

uint8_t lightsensor_get_scalerval(uint8_t which)
{
	if (which <= LIGHTSENS_SCALED_BACKLIGHT) return lev_set[which]; else return 0;
}
