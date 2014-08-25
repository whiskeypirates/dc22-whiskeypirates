/*
 * lcd_menu.c: lcd menuing functions
 * 2014 by true
 *
 * ----
 *
 * $Id: lcd_menu.c 214 2014-08-04 05:31:33Z true $
 */


#include "lcd_menu.h"
#include "../display/infopirate.h"

#include "menu_settings.h"
#include "menu_program.h"
#include "menu_sensors.h"
#include "menu_testing.h"
#include "menu_credits.h"

#include "../interface/i2c.h" 		// for communicating with attiny
#include "../device/attiny.h" 		// for temperature offset editing
#include "../device/lightsensor.h" 	// for light gain editing



/* lcd */
MenuItem *menuc; 	// lcd_menu.h

uint8_t menu_mode;

static uint16_t line_idx[2] = {0xffff, 0xffff};
static uint16_t line_time[2] = {0, 0};

static uint8_t lcd_fn_time = 0;
static char lcd_fn_output[64];

char lcd_line[2][9]; 		// lcd.h

void (*lcd_btn_fn[4])(); 	// pirate.h


/* menus */
uint16_t menu_editing;
uint32_t menu_oldval;
uint32_t menu_newval;

const MenuItem menu_runitem; 	// used for run mode


/* functions */
void lcd_menu_init()
{
	// initial menu
	if (settings.autorun & 0x01) {
		menuc = (MenuItem *)&menu_runitem;
		menu_root_run(0);
	} else {
		menuc = (MenuItem *)&menu_ritem[0];
		menu_btntype_menus();
	}
}

void lcd_menu_set(const MenuItem *set)
{
	menuc = (MenuItem *)set;
}

void lcd_menu_linescroll_reset(uint8_t idx)
{
	line_time[idx] = 0;
	line_idx[idx] = 0xffff;
}

static uint8_t lcd_menu_linescroll(uint8_t idx, char *dst, char *src, uint8_t maxlen,
			uint8_t spaces, uint16_t time_wait, uint16_t time_run)
{
	int i;
	int len;

	char *cpy;
	uint16_t copied = 0;

	len = strlen(src);

	// is this line too long?
	if (len > maxlen) {
		// begin scrolling.
		// if timeout occurred, update the index
		if (line_time[idx] == 0) {
			// set the pause time based on the max length plus padding spaces
			// if we are over this limit, we are in "start over" phase and need to use that delay
			if (line_idx[idx] > len + spaces) {
				// this is where we start or start over
				line_time[idx] = time_wait;
				line_idx[idx] = 0;
			} else {
				// we are in normal continuing phase, use the continuing delay and increment index
				line_time[idx] = time_run;
				line_idx[idx]++;
			}
		}
		line_time[idx]--;

		// figure out how much to copy
		cpy = src + line_idx[idx];
		len = ((len - maxlen) > line_idx[idx]) ?
				maxlen :
				len - line_idx[idx];

		// copy up to and including the trailing edge of the text
		if (len > 0) {
			for (i = 0; i < len && copied < maxlen; i++) {
				*dst++ = *cpy++;
				copied++;
			}
		}

		// if we have more left than the length provided, we are done
		if (len >= maxlen) {
			return maxlen;
		}

		// otherwise, we need to add some spaces
		i = spaces;
		if (len < 0) {
			i += len;
		}

		// copy the spaces into the stream
		if (i) {
			for (; i > 0 && (copied < maxlen); i--) {
				*dst++ = 0x20;
				copied++;
			}
		}

		// and, if we have room, copy starting from the beginning of the source
		while (copied < maxlen) {
			*dst++ = *src++;
			copied++;
		}

		return maxlen;
	} else {
		// nope, just copy directly
		strncpy(dst, src, maxlen);
		return len;
	}
}

void lcd_menu_update()
{
	uint8_t line_len;
	uint8_t i;

	// header line static text
	if (menuc->root == 0) {
		lcd_menu_linescroll(0, lcd_line[0], LCD_ROOT_TEXT, LCD_MAX_LINE_LENGTH,
					LCD_DEF_SPACING, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN);
	} else if ((int)menuc->root > 255) {
		lcd_menu_linescroll(0, lcd_line[0], menuc->root->this->disp, LCD_MAX_LINE_LENGTH,
				menuc->root->this->scroll_spaces, menuc->root->this->scroll_time_wait,
				menuc->root->this->scroll_time_run);
	} else {
		// maybe we can do other special shit here later
		// if set to 255, then external tasks need to update this line
	}

	// TODO: show header line dynamic text? or flag?

	// do we update the function-based string?
	if (!lcd_fn_time) {
		// reset the timeout value
		lcd_fn_time = menuc->this->dispfn_delay;

		// if there was no timeout value, we need to fake one
		if (lcd_fn_time == 0) {
			lcd_fn_time++;
		}

		// if there is a function attached, call it
		if (menuc->this->dispfn != NULL) {
			strncpy(lcd_fn_output, menuc->this->dispfn(menuc->this->disp_id), 63);
			lcd_fn_output[63] = 0;
		} else {
			// no function? no data.
			lcd_fn_output[0] = 0;
		}
	}
	lcd_fn_time--;

	// second line static text
	line_len = lcd_menu_linescroll(1, lcd_line[1], menuc->this->disp, LCD_MAX_LINE_LENGTH,
			menuc->this->scroll_spaces, menuc->this->scroll_time_wait, menuc->this->scroll_time_run);
	// second line dynamic text
	if (line_len < LCD_MAX_LINE_LENGTH) {
		lcd_menu_linescroll(1, lcd_line[1] + line_len, lcd_fn_output, LCD_MAX_LINE_LENGTH - line_len,
				menuc->this->scroll_spaces, menuc->this->scroll_time_wait, menuc->this->scroll_time_run);
	}

	// fix any nulls
	for (i = 0; i < 9; i++) {
		if (!lcd_line[0][i]) lcd_line[0][i] = 0x20;
		if (!lcd_line[1][i]) lcd_line[1][i] = 0x20;
	}
}

/* menu nav functions */
void menu_btn_next()
{
	if (menuc->next != 0) {
		lcd_menu_set(menuc->next);
		lcd_menu_linescroll_reset(1);
		lcd_fn_output[0] = 0;

		if (settings.beeper) {
			beep(settings.beep_type[0], 10);
		}
	}
}

void menu_btn_prev()
{
	if (menuc->prev != 0) {
		lcd_menu_set(menuc->prev);
		lcd_menu_linescroll_reset(1);
		lcd_fn_output[0] = 0;

		if (settings.beeper) {
			beep(settings.beep_type[0], 10);
		}
	}
}

void menu_btn_menu()
{
	if (menuc->root) {
		// call the root menu's entry function
		if (menuc->root->this->entryfn != NULL) {
			menuc->root->this->entryfn(menuc->root->this->entry_id);
		}

		// and set to that menu
		lcd_menu_set(menuc->root);
		lcd_menu_linescroll_reset(0);
		lcd_menu_linescroll_reset(1);
		lcd_fn_output[0] = 0;

		if (settings.beeper) {
			beep(settings.beep_type[0] - 1, 10);
		}
	}

	menu_btntype_menus();
}

void menu_btn_ok()
{
	// call this menu's entry function
	if (menuc->this->entryfn != NULL) {
		menuc->this->entryfn(menuc->this->entry_id);
		if (settings.beeper) {
			beep(settings.beep_type[0] + 1, 10);
		}
	}

	// and set to the new menu if available
	if (menuc->enter) {
		lcd_menu_set(menuc->enter);
		lcd_menu_linescroll_reset(0);
		lcd_menu_linescroll_reset(1);
		lcd_fn_output[0] = 0;
		if (settings.beeper) {
			beep(settings.beep_type[0], 10);
		}
	}
}

/* menu edit functions */
void menu_edit_next()
{
	int work;
	uint8_t do_beep = 0;

	switch (menu_editing) {
		case SETTING_NAME: {
			do_beep = 0;
			break;
		}

		case SETTING_CONTRAST: {
			if (menu_newval < 63) {
				menu_newval++;
				lcd_set_contrast(menu_newval);
				do_beep = settings.beeper;
			}
			break;
		}

		case SETTING_FAVCOLOR_RED:
		case SETTING_FAVCOLOR_GREEN:
		case SETTING_FAVCOLOR_BLUE: {
			if (menu_newval < 0xff) {
				menu_newval++;
				settings.fav_color[menu_editing - SETTING_FAVCOLOR_RED] = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}

		case SETTING_BEEP_TYPE_BUTTON:
		case SETTING_BEEP_TYPE_PAGING:
		case SETTING_BEEP_TYPE_ALARM: {
			if (menu_newval < 30) {
				menu_newval++;
				settings.beep_type[menu_editing - SETTING_BEEP_TYPE_BUTTON] = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}

		case SENSOR_LIGHT_SETTINGSGAIN: {
			if (menu_newval == 0) {
				menu_newval = LIGHTSENS_GAIN_MIN;
				do_beep = settings.beeper;
			} else if (menu_newval < LIGHTSENS_GAIN_MAX) {
				menu_newval++;
				do_beep = settings.beeper;
			}

			settings.light_setgain = menu_newval;
			break;
		}

		case SETTING_BRITE_LCD_VALUE: {
			if (menu_newval < 200) {
				menu_newval++;
				settings.lcd_brightness = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}
		case SETTING_BRITE_AUTOGAIN_LO: {
			if (menu_newval < 160) {
				menu_newval++;
				settings.led_autogain_lev_min = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}
		case SETTING_BRITE_AUTOGAIN_HI: {
			if (menu_newval < 160) {
				menu_newval++;
				settings.led_autogain_lev_max = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}
		case SETTING_BRITE_THRESH0:
		case SETTING_BRITE_THRESH1:
		case SETTING_BRITE_THRESH2:
		case SETTING_BRITE_THRESH3:
		case SETTING_BRITE_THRESH4: {
			work = menu_editing - SETTING_BRITE_THRESH0;

			if (menu_editing == SETTING_BRITE_THRESH4) {
				work = LIGHTSENS_GAIN_MAX;
			} else {
				work = settings.led_autothresh[work + 1] - 1;
			}

			if (menu_newval < work) {
				menu_newval++;
				settings.led_autothresh[menu_editing - SETTING_BRITE_THRESH0] = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}

		case SENSOR_TEMP_CALVALUE: {
			if (temperature_cal < 60) {
				temperature_cal++;
				do_beep = settings.beeper;
			}
			break;
		}
	}

	if (do_beep) {
		beep(settings.beep_type[0], 10);
	}
}

void menu_edit_prev()
{
	int work;
	uint8_t do_beep = 0;

	switch (menu_editing) {
		case SETTING_NAME: {
			break;
		}

		case SETTING_CONTRAST: {
			if (settings.contrast) {
				menu_newval--;
				lcd_set_contrast(menu_newval);
				do_beep = settings.beeper;
			}
			break;
		}

		case SETTING_FAVCOLOR_RED:
		case SETTING_FAVCOLOR_GREEN:
		case SETTING_FAVCOLOR_BLUE: {
			if (menu_newval) {
				menu_newval--;
				settings.fav_color[menu_editing - SETTING_FAVCOLOR_RED] = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}

		case SETTING_BEEP_TYPE_BUTTON:
		case SETTING_BEEP_TYPE_PAGING:
		case SETTING_BEEP_TYPE_ALARM: {
			if (menu_newval > 1) {
				menu_newval--;
				settings.beep_type[menu_editing - SETTING_BEEP_TYPE_BUTTON] = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}

		case SENSOR_LIGHT_SETTINGSGAIN: {
			if (menu_newval > LIGHTSENS_GAIN_MIN) {
				menu_newval--;
				do_beep = settings.beeper;
			} else if (menu_newval == LIGHTSENS_GAIN_MIN) {
				menu_newval = 0;
				do_beep = settings.beeper;
			}

			settings.light_setgain = menu_newval;
			break;
		}

		case SETTING_BRITE_LCD_VALUE: {
			if (menu_newval) {
				menu_newval--;
				settings.lcd_brightness = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}
		case SETTING_BRITE_AUTOGAIN_LO: {
			if (menu_newval > 0) {
				menu_newval--;
				settings.led_autogain_lev_min = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}
		case SETTING_BRITE_AUTOGAIN_HI: {
			if (menu_newval > 0) {
				menu_newval--;
				settings.led_autogain_lev_max = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}
		case SETTING_BRITE_THRESH0:
		case SETTING_BRITE_THRESH1:
		case SETTING_BRITE_THRESH2:
		case SETTING_BRITE_THRESH3:
		case SETTING_BRITE_THRESH4: {
			work = menu_editing - SETTING_BRITE_THRESH0;

			if (!work) {
				work = LIGHTSENS_GAIN_MIN;
			} else {
				work = settings.led_autothresh[work - 1] + 1;
			}

			if (menu_newval > work) {
				menu_newval--;
				settings.led_autothresh[menu_editing - SETTING_BRITE_THRESH0] = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}
		case SENSOR_TEMP_CALVALUE: {
			if (temperature_cal > -20) {
				temperature_cal--;
				do_beep = settings.beeper;
			}
			break;
		}
	}

	if (do_beep) {
		beep(settings.beep_type[0], 10);
	}
}

void menu_edit_menu()
{
	// values in this switch need to REVERT previous settings
	switch (menu_editing) {
		case SETTING_NAME: {
			break;
		}

		case SETTING_CONTRAST: {
			lcd_set_contrast(menu_oldval);
			break;
		}

		case SETTING_FAVCOLOR_RED:
		case SETTING_FAVCOLOR_GREEN:
		case SETTING_FAVCOLOR_BLUE: {
			settings.fav_color[menu_editing - SETTING_FAVCOLOR_RED] = menu_oldval;
			break;
		}

		case SETTING_BEEP_TYPE_BUTTON:
		case SETTING_BEEP_TYPE_PAGING:
		case SETTING_BEEP_TYPE_ALARM: {
			settings.beep_type[menu_editing - SETTING_BEEP_TYPE_BUTTON] = menu_oldval;
			break;
		}

		case SENSOR_LIGHT_SETTINGSGAIN: {
			settings.light_setgain = menu_oldval;
			break;
		}

		case SETTING_BRITE_LCD_VALUE: {
			settings.lcd_brightness = menu_oldval;
			break;
		}
		case SETTING_BRITE_AUTOGAIN_LO: {
			settings.led_autogain_lev_min = menu_oldval;
			break;
		}
		case SETTING_BRITE_AUTOGAIN_HI: {
			settings.led_autogain_lev_max = menu_oldval;
			break;
		}
		case SETTING_BRITE_THRESH0:
		case SETTING_BRITE_THRESH1:
		case SETTING_BRITE_THRESH2:
		case SETTING_BRITE_THRESH3:
		case SETTING_BRITE_THRESH4: {
			settings.led_autothresh[menu_editing - SETTING_BRITE_THRESH0] = menu_oldval;
			break;
		}


		case SENSOR_TEMP_CALVALUE: {
			temperature_cal = 0;
			break;
		}
	}

	// stop editing and return to menu control
	menu_editing = 0;
	menu_btntype_menus();

	if (settings.beeper) {
		beep(settings.beep_type[0], 10);
	}
}

void menu_edit_ok()
{
	// values in this switch need to COMMIT edited settings
	switch (menu_editing) {
		case SENSOR_TEMP_CALVALUE: {
			I2C_WriteTransfer(ATTINY_I2C_ADDR, (uint8_t *)&temperature_cal, 1, ATTINY_CMD_TEMP_CAL, 1);
			temperature_cal = 0;
			break;
		}

		default: {break;}
	}

	// stop editing and return to menu control
	menu_editing = 0;
	menu_btntype_menus();

	if (settings.beeper) {
		beep(settings.beep_type[0] + 1, 10);
	}
}

void menu_edit_start(uint16_t id)
{
	menu_editing = id;

	// set edited value
	switch (id) {
		case SETTING_CONTRAST: {
			menu_oldval = menu_newval = settings.contrast;
			break;
		}

		case SETTING_FAVCOLOR_RED:
		case SETTING_FAVCOLOR_GREEN:
		case SETTING_FAVCOLOR_BLUE: {
			menu_newval = menu_oldval = settings.fav_color[menu_editing - SETTING_FAVCOLOR_RED];
			break;
		}

		case SETTING_BEEP_TYPE_BUTTON:
		case SETTING_BEEP_TYPE_PAGING:
		case SETTING_BEEP_TYPE_ALARM: {
			menu_newval = menu_oldval = settings.beep_type[menu_editing - SETTING_BEEP_TYPE_BUTTON];
			break;
		}

		case SENSOR_LIGHT_SETTINGSGAIN: {
			menu_newval = menu_oldval = settings.light_setgain;
			break;
		}

		case SETTING_BRITE_LCD_VALUE: {
			menu_newval = menu_oldval = settings.lcd_brightness;
			break;
		}
		case SETTING_BRITE_AUTOGAIN_LO: {
			menu_newval = menu_oldval = settings.led_autogain_lev_min;
			break;
		}
		case SETTING_BRITE_AUTOGAIN_HI: {
			menu_newval = menu_oldval = settings.led_autogain_lev_max;
			break;
		}
		case SETTING_BRITE_THRESH0:
		case SETTING_BRITE_THRESH1:
		case SETTING_BRITE_THRESH2:
		case SETTING_BRITE_THRESH3:
		case SETTING_BRITE_THRESH4: {
			menu_newval = menu_oldval = settings.led_autothresh[menu_editing - SETTING_BRITE_THRESH0];
			break;
		}

		case SENSOR_TEMP_CALVALUE: {
			temperature_cal = 0;
			break;
		}

	}

	// set buttons to editing control
	menu_btntype_editing();
}

/* menu fn pointer set functions */
void menu_btntype_menus()
{
	menu_mode = MENU_MODE_NAVIGATING_MENUS;

	lcd_btn_fn[BTN_UP] = menu_btn_prev;
	lcd_btn_fn[BTN_MENU] = menu_btn_menu;
	lcd_btn_fn[BTN_OK] = menu_btn_ok;
	lcd_btn_fn[BTN_DOWN] = menu_btn_next;

	lcd_set_cursor(LCD_LINE_1, LCD_CMD_NO_CURSOR_FLASH);
	lcd_set_height(LCD_CMD_SINGLEHEIGHT);
}

void menu_btntype_run()
{
	menu_mode = MENU_MODE_RUNNING_PROGRAM;

	lcd_btn_fn[BTN_UP] = menu_infopirate_btn_prev;
	lcd_btn_fn[BTN_MENU] = menu_btn_menu;
	lcd_btn_fn[BTN_OK] = menu_infopirate_btn_ok;
	lcd_btn_fn[BTN_DOWN] = menu_infopirate_btn_next;

	lcd_set_cursor(LCD_LINE_1, LCD_CMD_NO_CURSOR_FLASH);
	lcd_set_height(LCD_CMD_DOUBLEHEIGHT);
}

void menu_btntype_editing()
{
	menu_mode = MENU_MODE_EDITING;

	lcd_btn_fn[BTN_UP] = menu_edit_prev;
	lcd_btn_fn[BTN_MENU] = menu_edit_menu;
	lcd_btn_fn[BTN_OK] = menu_edit_ok;
	lcd_btn_fn[BTN_DOWN] = menu_edit_next;

	lcd_set_cursor(LCD_LINE_2 + 7, LCD_CMD_CURSOR_FLASH);
}

  // run mode
void menu_root_run(uint16_t id)
{
	prog_init();
	infopirate_init();
	menu_btntype_run();
}


/* menu construction */
  // root menu
const MenuData menu_rdata[8] = {
	{"Run!",     NULL, 0, 0, 0, 0, 0, menu_root_run,      0},
	{"Settings", NULL, 0, 0, 0, 0, 0, NULL,               0},
	{"Programs", NULL, 0, 0, 0, 0, 0, NULL,               0},
	{"Save",     NULL, 0, 0, 0, 0, 0, menu_settings_save, 0},
	{"Sensors",  NULL, 0, 0, 0, 0, 0, NULL,               0},
	{"Testing",  NULL, 0, 0, 0, 0, 0, NULL,               0},
	{"Credits",  NULL, 0, 0, 0, 0, 0, NULL,               0},
	{"Shutdown", NULL, 0, 0, 0, 0, 0, pirate_shutdown,    0}
};
const MenuItem menu_ritem[8] = {
	{0,              0, &menu_ritem[1],       &menu_runitem, &menu_rdata[0]},
	{0, &menu_ritem[0], &menu_ritem[2], &menu_pref_ritem[0], &menu_rdata[1]},
	{0, &menu_ritem[1], &menu_ritem[3], &menu_prog_ritem[0], &menu_rdata[2]},
	{0, &menu_ritem[2], &menu_ritem[4],                   0, &menu_rdata[3]},
	{0, &menu_ritem[3], &menu_ritem[5], &menu_sens_ritem[0], &menu_rdata[4]},
	{0, &menu_ritem[4], &menu_ritem[6], &menu_test_ritem[0], &menu_rdata[5]},
	{0, &menu_ritem[5], &menu_ritem[7], &menu_cred_ritem[0], &menu_rdata[6]},
	{0, &menu_ritem[6],              0,                   0, &menu_rdata[7]},
};

  // run entry
const MenuData menu_rundata = {"", NULL, 0, 4, 0, 0, 0, NULL, 0};
const MenuItem menu_runitem = {&menu_ritem[0], 0, 0, 0, &menu_rundata};
