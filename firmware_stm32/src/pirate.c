/*
 * pirate.c: yarrrgh mateys
 * 2014 by true
 *
 * ----
 *
 * 0xId: pirate.c 179 2014-07-25 00:55:53Z true 0x
 */


#include "pirate.h"

#include "device/attiny.h"
#include "device/lcd.h"

#include "display/cgram/cgram.h"
const uint8_t cgram_crown[6][8];


/********
 * CEASE ALL OPERATIONS
 ********/
void pirate_shutdown(uint16_t type)
{
	int i;

	// turn off eyes LEDs
	for (i = 0; i < 8; i++) {
		led_eyes_set_level(i, 0);
	}
	led_eyes_tx();

	// put attiny to sleep
	attiny_sleep();

	// enable cgram
	lcd_cmd(LCD_CMD_SINGLEHEIGHT & 0xfe);

	// send crown data
	for (i = 1; i <= 6; i++) {
		lcd_print(0x40 + (i << 3), (uint8_t *)cgram_crown[i - 1], 8);
	}

	// print shutdown message
	static const uint8_t line1[8] = {0x01, 0x02, 0x03, 'T', 'h', 'r', 'e', 'e'};
	static const uint8_t line2[8] = {0x04, 0x05, 0x06, 'K', 'i', 'n', 'g', 's'};

	lcd_print(LCD_LINE_1, (uint8_t *)line1, 8);
	lcd_print(LCD_LINE_2, (uint8_t *)line2, 8);

	// turn off CPU and all peripherals
	PWR_EnterSTANDBYMode();
}

/********
 * update the pirate quick pseudorng
 ********/
uint32_t pirate_prng_val;
uint8_t pirate_prng()
{
	if (pirate_prng_val & 1) {
		pirate_prng_val = (pirate_prng_val >> 1);
	} else {
		pirate_prng_val = (pirate_prng_val >> 1) ^ 0x7FFFF159;
	}

	return (uint8_t)pirate_prng_val;
}

/********
 * a really shitty hackish delay function
 ********/
void pirate_delay(uint16_t ms)
{
	uint32_t wait;

	wait = ((SystemCoreClock / 2000) * ms) - 20;
	while (wait--) asm volatile("nop");
}

/********
 * scale a number from one range to another
 ********/
int16_t pirate_scale(int16_t value, int16_t src_min, int16_t src_max, int16_t dest_min, int16_t dest_max)
{
	int w;

	if (dest_min == dest_max) return dest_min;
	if (value < src_min) return dest_min;
	if (value > src_max) return dest_max;

	w = (dest_max - dest_min) * (value - src_min);
	w /= (src_max - src_min);
	w += dest_min;

	return (int16_t)w;
}

/********
 * itoa for pirates
 ********/
char * pirate_itoa(uint32_t val, uint8_t base, uint8_t leftpad)
{
	static char buf[11] = {0};
	int i;
	int j;

	buf[10] = 0;

	if (val == 0) {
		buf[9] = '0';
		i = 8;
	} else {
		for (i = 9; val && i; --i, val /= base)
			buf[i] = "0123456789ABCDEF"[val % base];
	}

	if (leftpad) {
		j = 9 - leftpad;
		while (j < i) {
			buf[i] = 0x20;
			i--;
		}
	}

	return &buf[i + 1];
}

char * pirate_sitoa(int32_t val, uint8_t base, uint8_t leftpad)
{
	static char buf[11] = {0};
	int neg;
	int i;
	int j;

	buf[10] = 0;

	neg = val < 0 ? 1 : 0;
	val = abs(val);

	if (val == 0) {
		buf[9] = '0';
		i = 8;
	} else {
		for (i = 9; val && i; --i, val /= base)
			buf[i] = "0123456789ABCDEF"[val % base];
	}

	if (neg) {
		buf[i] = '-';
		i--;
	}

	if (leftpad) {
		j = 9 - leftpad;
		while (j < i) {
			buf[i] = 0x20;
			i--;
		}
	}

	return &buf[i + 1];
}

/********
 * battery voltage
 ********/
static uint8_t pirate_batt_histlog = 0;
static uint16_t pirate_batt_history[32];
uint16_t pirate_batt_voltage()
{
	int i;
	uint32_t t = 0;

	for (i = 0; i < 32; i++) {
		t += pirate_batt_history[i];
	}

	return t >> 5;
}

void pirate_batt_log(uint16_t rawvalue)
{
	int t;

	t = (rawvalue << 4) / 195;

	pirate_batt_history[pirate_batt_histlog] = t;

	pirate_batt_histlog++;
	pirate_batt_histlog &= 0x1f;
}

/********
 * temperature averaging
 ********/
static uint8_t pirate_temp_histlog = 0;
static uint8_t pirate_temp_history[20];
uint16_t pirate_thermometer(uint8_t deg_f) {
	int i;
	uint32_t t = 0;

	for (i = 0; i < 20; i++) {
		t += pirate_temp_history[i];
	}

	t >>= 1;

	if (deg_f) {
		return (((t * 9) / 5) / 10) + 32;
	} else {
		return t;
	}
}

void pirate_thermometer_log(uint8_t temp)
{
	pirate_temp_history[pirate_temp_histlog] = temp;

	pirate_temp_histlog++;
	if (pirate_temp_histlog > 20) pirate_temp_histlog = 0;
}
