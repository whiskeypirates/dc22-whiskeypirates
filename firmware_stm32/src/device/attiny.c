/*
 * attiny.c: interface to attiny88's features
 * 2014 true
 *
 * ----
 *
 * $Id: attiny.c 221 2014-08-07 16:55:36Z true $
 *
 * ----
 *
 * attiny88's I2C address is 0x73 - see led_eyes.h for define
 */


#include "attiny.h"
#include "../interface/i2c.h"
#include "../device/lightsensor.h"

  // global settings
PirateSettings settings; 	// pirate.h

  // led mode
static uint8_t led_eyes_mode;

  // led brightness
static uint8_t led_level[2][4];

  // led programs
#include "../led/led_eyes_prog.h"
static void (*led_program)();
static void (*led_program_list[])() = {
	led_prog_candle_flicker,
	led_prog_candle_flicker_favcolor,
	led_prog_randflasher,
	led_prog_mic_spl
};
const char led_eyes_prog_name[LED_EYES_PROG_COUNT][16] = {
	{"CandleFlicker"},
	{"Candle+FavColor"},
	{"RandFlasher"},
	{"MicSPL"}
};


/* attiny functions */
static void attiny_write(uint16_t cmd, uint8_t *data, uint8_t bytes)
{
	uint8_t cmdlen;

	cmdlen = (cmd > 0xff) ? 2 : 1;

	if (bytes) {
		I2C_WriteTransfer(ATTINY_I2C_ADDR, data, bytes, cmd, cmdlen);
	}
}

static uint32_t attiny_read(uint16_t cmd, uint8_t bytes)
{
	uint8_t cmdlen;
	uint8_t fb[4] = {0, 0, 0, 0};

	cmdlen = (cmd > 0xff) ? 2 : 1;

	if (bytes && bytes <= 4) {
		I2C_ReadTransfer(ATTINY_I2C_ADDR, fb, bytes, cmd, cmdlen);
	}

	return fb[0] | fb[1] << 8 | fb[2] << 16 | fb[3] << 24;
}

uint8_t attiny_read_temp()
{
	return (uint8_t)attiny_read(ATTINY_CMD_READ_TEMP, 1);
}

uint8_t attiny_read_light_level(uint8_t led_idx)
{
	return (uint8_t)attiny_read(ATTINY_CMD_READ_LIGHT + (led_idx & 0x03), 1);
}

void attiny_write_light_sensitivity(uint8_t led, uint8_t sensitivity)
{
	uint8_t buf[2];

	buf[0] = led;
	buf[1] = sensitivity;

	attiny_write(ATTINY_CMD_LIGHTSENSOR_SENS, buf, 2);
}

void attiny_sleep()
{
	I2C_WriteTransfer(ATTINY_I2C_ADDR, 0, 0, ATTINY_CMD_SLEEP, 1);
}

/* rgbled-related functions */
void led_eyes_tx()
{
	uint8_t buf[12];
	uint16_t scaler;
	uint8_t i, j;

	// build up the packet
	i = j = 0;
	while (i < 12) {
		buf[i++] = ATTINY_CMD_LED_LEVEL + j;

		scaler = (settings.led_autoadjust & 0x80) ? lightsensor_get_scalerval(LIGHTSENS_SCALED_EYES) : 256;
		buf[i++] = ((led_level[j >> 2][j % 4]) * scaler) >> 8;
		if (i == 6) {
			j = 4;
		} else {
			j++;
		}
	}

	// and send it
	I2C_WriteTransfer(ATTINY_I2C_ADDR, buf, 12, 0, 0);
}

void led_eyes_set_level(uint8_t idx, uint8_t level)
{
	led_level[idx >> 2][idx & 0x03] = level;
}

void led_eyes_set_mode(uint8_t mode)
{
	led_eyes_mode = mode;
}

void led_eyes_set_program(uint8_t program_idx, uint8_t init,
		uint16_t wait, uint16_t level, uint32_t offset, uint32_t settings)
{
	// we need to be in program mode
	led_eyes_set_mode(LED_EYES_MODE_PROGRAM);
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

void led_eyes_mode_update()
{
	uint8_t i;

	switch (led_eyes_mode) {
		case LED_EYES_MODE_PROGRAM: {
			// if we have a valid program loaded, run it
			if (led_program != NULL) {
				led_program();
			}
			break;
		}
		case LED_EYES_MODE_OFF: {
			led_program = NULL;
			for (i = 0; i < 4; i++) {
				led_level[0][i] = 0;
				led_level[1][i] = 0;
			}
			break;
		}
	}
}
