/*
 * menu_settings.c: setting shit up for pirates since
 * 2014 by true
 *
 * ----
 *
 * $Id: menu_settings.c 223 2014-08-12 17:03:46Z true $
 */


#include "menu_settings.h"
#include "../device/lightsensor.h" 	// for lightsensor default defines
#include "../device/attiny.h" 		// eyes program set for favcolor editor

  // menus
MenuItem *menuc; 			// lcd_menu.h

uint16_t menu_editing; 		// lcd_menu.h
uint32_t menu_oldval;
uint32_t menu_newval;

  // settings
PirateSettings settings; 	// pirate.h
PirateSettings *settings_eeprom = (PirateSettings *)PIRATE_SETTINGS_EEPROM_ADDR;


/* settings functions */
void menu_settings_save(uint16_t id)
{
	settings_save();
}

void settings_save()
{
	uint32_t *active = (uint32_t *)&settings;

	int i;

	DATA_EEPROM_Unlock();

	// make sure the first name byte is set to something not considered invalid
	// if it isn't set we set it to be a space
	if (settings.name[0] == 0x00 || settings.name[0] == 0xff) {
		settings.name[0] = 0x20;
	}

	// write data
	for (i = 0; i < sizeof(settings); i += 4) {
		DATA_EEPROM_ProgramWord(PIRATE_SETTINGS_EEPROM_ADDR + i, *active++);
	}

	DATA_EEPROM_Lock();
}

void settings_restore(uint8_t load_defaults)
{
	uint8_t *active = (uint8_t *)&settings;
	uint8_t *stored = (uint8_t *)PIRATE_SETTINGS_EEPROM_ADDR;

	int i;
	int j;
	uint8_t valid = 0;

	// see if valid settings are in EEPROM
	if (!(settings_eeprom->name[0] == 0x00 || settings_eeprom->name[0] == 0xff)) {
		// seems valid, copy it
		for (i = 0; i < sizeof(settings); i++) {
			*active++ = *stored++;
		}
		valid = 1;
	}

	if (!valid || load_defaults) {
		// not valid, set defaults
		if (load_defaults & 0x02) {
			// only clear name if invalid settings, or if second bit is set
			strncpy(settings.name, "        ", 8);
		}

		settings.fav_color[0] = 0x80;
		settings.fav_color[1] = 0x10;
		settings.fav_color[2] = 0;

		settings.beeper = 0x80 + 31;
		settings.beep_type[0] = 24;
		settings.beep_type[1] = 30;
		settings.beep_type[2] = 28;

		settings.contrast = 0x15; 	// gives great readability with scrolling on my badge

		settings.autorun = 0;

		settings.led_autoadjust = 0x88;
		settings.led_autogain_lev_min = LIGHTSENS_AUTOGAIN_LEVEL_MIN;
		settings.led_autogain_lev_max = LIGHTSENS_AUTOGAIN_LEVEL_MAX;
		settings.led_autothresh[0] = LIGHTSENS_THRESH_DAY;
		settings.led_autothresh[1] = LIGHTSENS_THRESH_TOP_BRIGHT;
		settings.led_autothresh[2] = LIGHTSENS_THRESH_TOP_NORM;
		settings.led_autothresh[3] = LIGHTSENS_THRESH_TOP_DIM;
		settings.led_autothresh[4] = LIGHTSENS_THRESH_TOP_DARK;

		settings.lcd_autobrite = 1;
		settings.lcd_brightness = 50;

		settings.light_setgain = 0;

		for (i = 0; i < PIRATE_PROG_SAVED_MAX; i++) {
			for (j = 0; j < 3; j++) {
				settings.led_prog[j][i].type = 0b00000010; // not enabled, will init, program mode
				settings.led_prog[j][i].progidx = 0;
				settings.led_prog[j][i].wait = 80;
				settings.led_prog[j][i].level = 0;
				settings.led_prog[j][i].offset = 0;
				settings.led_prog[j][i].option = 0;
				settings.led_prog[j][i].dwell = 10;
			}
		}

		settings.led_prog_mode = 0b00010101; // all program types set to normal run

		settings.mic_cal[0] = 2300;
		settings.mic_cal[1] = 2800;

		/* default programs */
		#include "menu_settings_defprog.h"

		// fix the null name issue
		if (settings.name[0] == 0x00) {
			strncpy(settings.name, "        ", 8);
		}
	}
}


/* menu functions */
  // feedback display
static char * menu_pref_disp(uint16_t id)
{
	switch (id) {
		case SETTING_NAME: {
			return settings.name;
		}
		case SETTING_AUTORUN_ENA: {
			return settings.autorun & 0x01 ? "Y" : "N";
		}
		case SETTING_ALWAYS_RUN_PROG_ENA: {
			return settings.autorun & 0x02 ? "Y" : "N";
		}

		case SETTING_FAVCOLOR_RED:
		case SETTING_FAVCOLOR_GREEN:
		case SETTING_FAVCOLOR_BLUE: {
			return pirate_sitoa(settings.fav_color[id - SETTING_FAVCOLOR_RED], 16, 2);
		}

		case SETTING_BEEPER_ENA: {
			return (settings.beeper & 0x80) ? " On" : "Off";
		}
		case SETTING_BEEP_TYPE_BUTTON:
		case SETTING_BEEP_TYPE_PAGING:
		case SETTING_BEEP_TYPE_ALARM: {
			return pirate_sitoa(settings.beep_type[id - SETTING_BEEP_TYPE_BUTTON], 10, 2);
		}

		case SETTING_BRITE_AUTO_ENA: {
			return (settings.led_autoadjust & 0x80) ? "  On" : " Off";
		}

		case SETTING_BRITE_AUTO_SPEED: {
			switch (settings.led_autoadjust & 0x7f) {
			 	case 0: 	return " Off";
				case 2: 	return "Hypr";
				case 4: 	return "High";
				case 6: 	return " Med";
				case 8: 	return "Norm";
				case 10: 	return "Slow";
				default: 	return pirate_sitoa(settings.led_autoadjust, 10, 4);
			}
		}
		case SETTING_BRITE_LCD_AUTO: {
			return (settings.lcd_autobrite) ? "Auto" : "Manu";
		}
		case SETTING_BRITE_LCD_VALUE: {
			return pirate_sitoa(settings.lcd_brightness, 10, 3);
		}
		case SETTING_BRITE_AUTOGAIN_LO: {
			return pirate_sitoa(settings.led_autogain_lev_min, 10, 2);
		}
		case SETTING_BRITE_AUTOGAIN_HI: {
			return pirate_sitoa(settings.led_autogain_lev_max, 10, 2);
		}
		case SETTING_BRITE_THRESH0:
		case SETTING_BRITE_THRESH1:
		case SETTING_BRITE_THRESH2:
		case SETTING_BRITE_THRESH3:
		case SETTING_BRITE_THRESH4: {
			return pirate_sitoa(settings.led_autothresh[id - SETTING_BRITE_THRESH0], 10, 2);
		}

		default: {
			return "";
		}
	}
}

char * menu_pref_run_prog_disp(uint16_t id)
{
	if (settings.autorun & 0x02) {
		return "RunProg:Always!";
	} else {
		return "RunProg:Runmode";
	}
}

  // name changing handler
void menu_pref_name_next()
{
	if (menu_newval) {
		if (settings.name[menu_oldval] < 0x20) {
			settings.name[menu_oldval] = 0x20;
		} else if (settings.name[menu_oldval] < 0x5e) {
			settings.name[menu_oldval]++;
		} else if (settings.name[menu_oldval] == 0x5e) {
			settings.name[menu_oldval] = 0x61;
		} else if (settings.name[menu_oldval] < 0x7d) {
			settings.name[menu_oldval]++;
		}
	} else {
		if (menu_oldval < 7) {
			menu_oldval++;
		}
	}

	if (settings.beeper) {
		beep(settings.beep_type[0], 10);
	}

	lcd_set_cursor(LCD_LINE_2 + menu_oldval, 0);
}

void menu_pref_name_prev()
{
	if (menu_newval) {
		if (settings.name[menu_oldval] > 0x7a) {
			settings.name[menu_oldval] = 0x7a;
		} else if (settings.name[menu_oldval] > 0x61) {
			settings.name[menu_oldval]--;
		} else if (settings.name[menu_oldval] == 0x61) {
			settings.name[menu_oldval] = 0x5e;
		} else if (settings.name[menu_oldval] > 0x20) {
			settings.name[menu_oldval]--;
		}
	} else {
		if (menu_oldval) {
			menu_oldval--;
		}
	}

	if (settings.beeper) {
		beep(settings.beep_type[0], 10);
	}

	lcd_set_cursor(LCD_LINE_2 + menu_oldval, 0);
}

void menu_pref_name_menu()
{
	menuc = (MenuItem *)&menu_pref_ritem[0];
	menu_btntype_menus();

	if (settings.beeper) {
		beep(settings.beep_type[0] - 1, 10);
	}
}

void menu_pref_name_ok()
{
	menu_newval ^= 1;

	if (menu_newval) {
		lcd_set_cursor(LCD_LINE_2 + menu_oldval, LCD_CMD_CURSOR_FLASH);
	} else {
		lcd_set_cursor(LCD_LINE_2 + menu_oldval, LCD_CMD_CURSOR_UNDER);
	}

	if (settings.beeper) {
		beep(settings.beep_type[0] + 1, 10);
	}
}

static void menu_pref_name_start(uint16_t id)
{
	// oldval is cursor pos, newval is editing-is-on
	menu_oldval = menu_newval = 0;
	lcd_set_cursor(LCD_LINE_2, LCD_CMD_CURSOR_UNDER);

	lcd_btn_fn[BTN_UP] = menu_pref_name_prev;
	lcd_btn_fn[BTN_MENU] = menu_pref_name_menu;
	lcd_btn_fn[BTN_OK] = menu_pref_name_ok;
	lcd_btn_fn[BTN_DOWN] = menu_pref_name_next;
}

static void menu_pref_favcolor_start(uint16_t id)
{
	led_eyes_set_program(1, 1, 0, 0, 0, 0);
}

  // other settings
static void menu_pref_autorun_set(uint16_t id)
{
	if (settings.autorun & 0x01) {
		settings.autorun &= 0xfe;
	} else {
		settings.autorun |= 0x01;
	}
}

static void menu_pref_alwaysrun_tog(uint16_t id)
{
	if (settings.autorun & 0x02) {
		settings.autorun &= 0xfd;
	} else {
		settings.autorun |= 0x02;
	}
}

static void menu_pref_beepena_set(uint16_t id)
{
	if (settings.beeper & 0x80) {
		settings.beeper = 0;
	} else {
		settings.beeper |= 0x80;
	}
}

static void menu_pref_brite_auto_ena_set(uint16_t id)
{
	if (settings.led_autoadjust & 0x80) {
		settings.led_autoadjust &= 0x7f;
	} else {
		settings.led_autoadjust |= 0x80;
	}
}

static void menu_pref_brite_auto_spd_set(uint16_t id)
{
	if ((settings.led_autoadjust & 0x7f) >= 10) {
		settings.led_autoadjust &= 0x80;
	} else {
		settings.led_autoadjust += 2;
	}
}

static void menu_pref_brite_lcd_auto_set(uint16_t id)
{
	if (settings.lcd_autobrite) {
		settings.lcd_autobrite = 0;
	} else {
		settings.lcd_autobrite = 1;
	}
}



/* menu construction */
  // declaration
const MenuItem menu_pref_nameitem[];
const MenuItem menu_pref_favcitem[];
const MenuItem menu_pref_beepitem[];
const MenuItem menu_pref_briteitem[];


  // main menu
const MenuData menu_pref_rdata[] = {
	{"Set Name",            NULL,                   0, 0, 0, 0, 0,       menu_pref_name_start, 0},
	{"FavColor",            NULL,                   0, 0, 0, 0, 0,   menu_pref_favcolor_start, 0},
	{"", menu_pref_run_prog_disp,                   0, 4,
			LCD_DEF_SPACING, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, menu_pref_alwaysrun_tog, 0},
	{"Contrast",            NULL,                   0, 0, 0, 0, 0,          menu_edit_start, SETTING_CONTRAST},
	{"Beeps",     menu_pref_disp,  SETTING_BEEPER_ENA, 4, 0, 0, 0,      menu_pref_beepena_set, 0},
	{"BeepMenu",            NULL,                   0, 0, 0, 0, 0,                       NULL, 0},
	{"Brightness",          NULL,                   0, 0,
			LCD_DEF_SPACING, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN,                    NULL, 0},
	{"AutoRun",   menu_pref_disp, SETTING_AUTORUN_ENA, 4, 0, 0, 0,      menu_pref_autorun_set, 0},
};

const MenuItem menu_pref_ritem[] = {
	{&menu_ritem[LCD_MENU_ROOT_SETTINGS],                   0, &menu_pref_ritem[1],  &menu_pref_nameitem[0], &menu_pref_rdata[0]},
	{&menu_ritem[LCD_MENU_ROOT_SETTINGS], &menu_pref_ritem[0], &menu_pref_ritem[2],  &menu_pref_favcitem[0], &menu_pref_rdata[1]},
	{&menu_ritem[LCD_MENU_ROOT_SETTINGS], &menu_pref_ritem[1], &menu_pref_ritem[3],                       0, &menu_pref_rdata[2]},
	{&menu_ritem[LCD_MENU_ROOT_SETTINGS], &menu_pref_ritem[2], &menu_pref_ritem[4],                       0, &menu_pref_rdata[3]},
	{&menu_ritem[LCD_MENU_ROOT_SETTINGS], &menu_pref_ritem[3], &menu_pref_ritem[5],                       0, &menu_pref_rdata[4]},
	{&menu_ritem[LCD_MENU_ROOT_SETTINGS], &menu_pref_ritem[4], &menu_pref_ritem[6],  &menu_pref_beepitem[0], &menu_pref_rdata[5]},
	{&menu_ritem[LCD_MENU_ROOT_SETTINGS], &menu_pref_ritem[5], &menu_pref_ritem[7], &menu_pref_briteitem[0], &menu_pref_rdata[6]},
	{&menu_ritem[LCD_MENU_ROOT_SETTINGS], &menu_pref_ritem[6],                   0,                       0, &menu_pref_rdata[7]},
};

  // name editing
const MenuData menu_pref_namedata[] = {
	{"", menu_pref_disp, SETTING_NAME, 4, 0, 0, 0, NULL, 0}
};
const MenuItem menu_pref_nameitem[] = {
	{&menu_pref_ritem[0], 0, 0, 0, &menu_pref_namedata[0]},
};

  // fav color menu
const MenuData menu_pref_favcdata[] = {
	{"Red   ",   menu_pref_disp,   SETTING_FAVCOLOR_RED, 4, 0, 0, 0, menu_edit_start,   SETTING_FAVCOLOR_RED},
	{"Green ",   menu_pref_disp, SETTING_FAVCOLOR_GREEN, 4, 0, 0, 0, menu_edit_start, SETTING_FAVCOLOR_GREEN},
	{"Blue  ",   menu_pref_disp,  SETTING_FAVCOLOR_BLUE, 4, 0, 0, 0, menu_edit_start,  SETTING_FAVCOLOR_BLUE},
};
const MenuItem menu_pref_favcitem[] = {
	{&menu_pref_ritem[1],                      0, &menu_pref_favcitem[1], 0, &menu_pref_favcdata[0]},
	{&menu_pref_ritem[1], &menu_pref_favcitem[0], &menu_pref_favcitem[2], 0, &menu_pref_favcdata[1]},
	{&menu_pref_ritem[1], &menu_pref_favcitem[1],                      0, 0, &menu_pref_favcdata[2]},
};

  // beeper config menu
const MenuData menu_pref_beepdata[] = {
	{"Button", menu_pref_disp, SETTING_BEEP_TYPE_BUTTON, 4, 0, 0, 0, menu_edit_start, SETTING_BEEP_TYPE_BUTTON},
	{"Paging", menu_pref_disp, SETTING_BEEP_TYPE_PAGING, 4, 0, 0, 0, menu_edit_start, SETTING_BEEP_TYPE_PAGING},
	{"Alarm ", menu_pref_disp,  SETTING_BEEP_TYPE_ALARM, 4, 0, 0, 0, menu_edit_start,  SETTING_BEEP_TYPE_ALARM},
};
const MenuItem menu_pref_beepitem[] = {
	{&menu_pref_ritem[5],                      0, &menu_pref_beepitem[1], 0, &menu_pref_beepdata[0]},
	{&menu_pref_ritem[5], &menu_pref_beepitem[0], &menu_pref_beepitem[2], 0, &menu_pref_beepdata[1]},
	{&menu_pref_ritem[5], &menu_pref_beepitem[1],                      0, 0, &menu_pref_beepdata[2]},
};

  // brightness menu
const MenuData menu_pref_britedata[] = {
	{"Auto",   menu_pref_disp,    SETTING_BRITE_AUTO_ENA, 4, 0, 0, 0, menu_pref_brite_auto_ena_set,                         0},
	{"Spd ",   menu_pref_disp,  SETTING_BRITE_AUTO_SPEED, 4, 0, 0, 0, menu_pref_brite_auto_spd_set,                         0},
	{"LCD ",   menu_pref_disp,    SETTING_BRITE_LCD_AUTO, 4, 0, 0, 0, menu_pref_brite_lcd_auto_set,                         0},
	{"LCDlv",  menu_pref_disp,   SETTING_BRITE_LCD_VALUE, 4, 0, 0, 0,              menu_edit_start,   SETTING_BRITE_LCD_VALUE},
	{"AGNLo",  menu_pref_disp, SETTING_BRITE_AUTOGAIN_LO, 4, 0, 0, 0,              menu_edit_start, SETTING_BRITE_AUTOGAIN_LO},
	{"AGNHi",  menu_pref_disp, SETTING_BRITE_AUTOGAIN_HI, 4, 0, 0, 0,              menu_edit_start, SETTING_BRITE_AUTOGAIN_HI},
	{"GnDay ", menu_pref_disp,     SETTING_BRITE_THRESH0, 4, 0, 0, 0,              menu_edit_start,     SETTING_BRITE_THRESH0},
	{"GnBrt ", menu_pref_disp,     SETTING_BRITE_THRESH1, 4, 0, 0, 0,              menu_edit_start,     SETTING_BRITE_THRESH1},
	{"GnNrm ", menu_pref_disp,     SETTING_BRITE_THRESH2, 4, 0, 0, 0,              menu_edit_start,     SETTING_BRITE_THRESH2},
	{"GnDim ", menu_pref_disp,     SETTING_BRITE_THRESH3, 4, 0, 0, 0,              menu_edit_start,     SETTING_BRITE_THRESH3},
	{"GnDrk ", menu_pref_disp,     SETTING_BRITE_THRESH4, 4, 0, 0, 0,              menu_edit_start,     SETTING_BRITE_THRESH4},
};
const MenuItem menu_pref_briteitem[] = {
	{&menu_pref_ritem[6],                       0,  &menu_pref_briteitem[1], 0, &menu_pref_britedata[0]},
	{&menu_pref_ritem[6], &menu_pref_briteitem[0],  &menu_pref_briteitem[2], 0, &menu_pref_britedata[1]},
	{&menu_pref_ritem[6], &menu_pref_briteitem[1],  &menu_pref_briteitem[3], 0, &menu_pref_britedata[2]},
	{&menu_pref_ritem[6], &menu_pref_briteitem[2],  &menu_pref_briteitem[4], 0, &menu_pref_britedata[3]},
	{&menu_pref_ritem[6], &menu_pref_briteitem[3],  &menu_pref_briteitem[5], 0, &menu_pref_britedata[4]},
	{&menu_pref_ritem[6], &menu_pref_briteitem[4],  &menu_pref_briteitem[6], 0, &menu_pref_britedata[5]},
	{&menu_pref_ritem[6], &menu_pref_briteitem[5],  &menu_pref_briteitem[7], 0, &menu_pref_britedata[6]},
	{&menu_pref_ritem[6], &menu_pref_briteitem[6],  &menu_pref_briteitem[8], 0, &menu_pref_britedata[7]},
	{&menu_pref_ritem[6], &menu_pref_briteitem[7],  &menu_pref_briteitem[9], 0, &menu_pref_britedata[8]},
	{&menu_pref_ritem[6], &menu_pref_briteitem[8], &menu_pref_briteitem[10], 0, &menu_pref_britedata[9]},
	{&menu_pref_ritem[6], &menu_pref_briteitem[9],                        0, 0, &menu_pref_britedata[10]},
};
