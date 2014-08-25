/*
 * menu_program.c: led and lcd program selection and configuration
 * 2014 by true
 *
 * ----
 *
 * $Id: menu_program.c 219 2014-08-06 01:31:48Z true $
 */


#include "menu_program.h"
#include "../device/beep.h"

#include "../led/led_matrix.h"
#include "../led/led_bone.h"
#include "../device/attiny.h" 	// eyes


  // menus
MenuItem *menuc; 			// lcd_menu.h

uint8_t menu_mode; 			// lcd_menu.h

char lcd_line[2][9]; 		// lcd.h

void (*lcd_btn_fn[4])(); 	// pirate.h


  // programs
static char prog_str[20]; 		// program names max 16 characters (currently coded to 15 characters)

static uint8_t prog_type; 		// which menu has been selected (skull, bones, eyes)
static uint8_t prog_idx; 		// which program is being edited within the menu

static uint16_t prog_hex_val; 	// the value being edited

extern uint16_t menu_editing; 	// lcd_menu.h
extern uint32_t menu_oldval;
extern uint32_t menu_newval;

  // program running
uint8_t prog_id[3];
uint16_t prog_dwell[3];

  // program names
const char led_matrix_prog_name[LED_MATRIX_PROG_COUNT][16];
const char led_bone_prog_name[LED_BONE_PROG_COUNT][16];
const char led_eyes_prog_name[LED_EYES_PROG_COUNT][16];

const char led_matrix_ptrn_name[LED_MATRIX_PTRN_COUNT][16];

const char (*prog_name[3])[16] = {
	led_matrix_prog_name,
	led_bone_prog_name,
	led_eyes_prog_name
};
const uint8_t prog_count[3] = {
	LED_MATRIX_PROG_COUNT,
	LED_BONE_PROG_COUNT,
	LED_EYES_PROG_COUNT
};

const char (*ptrn_name[3])[16] = {
	0,
	0,
	0
};
const uint8_t ptrn_count[3] = {
	LED_MATRIX_PTRN_COUNT,
	LED_BONE_PTRN_COUNT,
	LED_EYES_PTRN_COUNT
};


  // programs menu forward declaration
const MenuItem menu_prog_leditem[];
const MenuItem menu_prog_ledaitem;


/* external program functions */
/********
 * initialize variables for program running
 ********/
void prog_init()
{
	uint8_t i;

	for (i = 0; i < 3; i++) {
		prog_id[i] = 255;
		prog_dwell[i] = 0;
	}
}

uint8_t prog_get_mode(uint8_t id)
{
	return (settings.led_prog_mode >> (id << 1)) & 0x03;
}

uint8_t menu_prog_get_type()
{
	return prog_type;
}

uint8_t menu_prog_get_idx()
{
	return prog_idx;
}

/* program functions */
char * menu_prog_disp(uint16_t id)
{
	switch (id) {
		case PROG_GLOBAL_MODE: {
			switch ((settings.led_prog_mode >> (prog_type << 1)) & 0x03) {
				case 0b00: 	return " Off";
				case 0b01: 	return "Nrml";
				case 0b10: 	return "Rand";
				default: 	return "";
			}
		}

		case PROG_GLOBAL_CHANGEIDX: {
			prog_str[0] = '#';
			prog_str[1] = 0;
			strncat(prog_str, pirate_sitoa(prog_idx + 1, 10, 2), 4);
			if (prog_str[1] == 0x00 || prog_str[1] == 0x20) prog_str[1] = 0x30;
			return prog_str;
		}

		case PROG_EDIT_ENABLE: {
			if (settings.led_prog[prog_type][prog_idx].type & 0x80) {
				return "Y";
			} else {
				return "N";
			}
		}
		case PROG_EDIT_FORCE_INIT: {
			if (settings.led_prog[prog_type][prog_idx].type & 0x02) {
				return "Y";
			} else {
				return "N";
			}
		}
		case PROG_EDIT_TYPE: {
			if (settings.led_prog[prog_type][prog_idx].type & 0x01) {
				return "Ptrn";
			} else {
				return "Prog";
			}
		}
		case PROG_EDIT_PROG_ID: {
			prog_str[0] = 0;
			prog_str[1] = 0;
			strncat(prog_str, pirate_sitoa(settings.led_prog[prog_type][prog_idx].progidx, 10, 2), 4);
			if (prog_str[0] == 0x00 || prog_str[0] == 0x20) prog_str[0] = 0x30;
			return prog_str;
		}

		default: {return "";}
	}
}

char * prog_type_edit_str(uint16_t id) {
	switch (id) {
		case PROG_TYPE_SKULL: 		{strcpy(prog_str, "Skull   "); prog_str[5] = 0; break;}
		case PROG_TYPE_BONES: 		{strcpy(prog_str, "Bones   "); prog_str[5] = 0; break;}
		case PROG_TYPE_EYES: 		{strcpy(prog_str, "Eyes    "); prog_str[5] = 0; break;}
		case PROG_TYPE_INFOPIR8: 	return "InfoPir8";
	}

	if (menu_mode == MENU_MODE_PROGRAM_EDITOR) {
		if (prog_idx < PIRATE_PROG_SAVED_MAX) {
			strncat(prog_str, "#", 8);
			strncat(prog_str, pirate_sitoa(prog_idx + 1, 10, 2), 8);

			if (prog_str[6] == 0x20 || prog_str[6] == 0x00) prog_str[6] = 0x30;
		}
	}

	prog_str[8] = 0x00;

	return prog_str;
}

char * prog_dwell_str(uint16_t id)
{
	strcpy(prog_str, pirate_sitoa(settings.led_prog[prog_type][prog_idx].dwell, 10, 2));

	if (settings.led_prog[prog_type][prog_idx].dwell <= 99) {
		strncat(prog_str, "s", 4);
	}

	return prog_str;
}

char * prog_advanced_str(uint16_t id)
{
	uint32_t work;

	switch (menu_editing) {
		case PROG_EDIT_PROG_ID: {
			work = settings.led_prog[prog_type][prog_idx].progidx;
			strcpy(prog_str, pirate_sitoa(work, 10, 0));
			strncat(prog_str, ":", 19);
			strncat(prog_str, prog_name[prog_type][work], 19);
			prog_str[19] = 0;
			return prog_str;
		}
		case PROG_EDIT_CYCLESPEED: {
			work = settings.led_prog[prog_type][prog_idx].wait;
			strcpy(prog_str, pirate_sitoa(work, 10, 6));
			strncat(prog_str, "ms", 8);
			prog_str[8] = 0;
			return prog_str;
		}
		case PROG_EDIT_LED_LEVEL: {
			work = settings.led_prog[prog_type][prog_idx].level;
			strcpy(prog_str, " 0x");
			strncat(prog_str, pirate_itoa(work, 16, 4), 6);

			for (work = 3; work < 6; work++) {
				if (prog_str[work] == 0x00 || prog_str[work] == 0x20) prog_str[work] = 0x30;
			}

			prog_str[8] = 0;
			return prog_str;
		}
		case PROG_EDIT_LED_OFFSET: {
			work = settings.led_prog[prog_type][prog_idx].offset;
			strncpy(prog_str, pirate_itoa(work, 16, 8), 8);

			for (work = 0; work < 8; work++) {
				if (prog_str[work] == 0x00 || prog_str[work] == 0x20) prog_str[work] = 0x30;
			}

			prog_str[8] = 0;
			return prog_str;
		}
		case PROG_EDIT_LED_OPTIONS: {
			work = settings.led_prog[prog_type][prog_idx].option;
			strncpy(prog_str, pirate_itoa(work, 16, 8), 8);

			for (work = 0; work < 8; work++) {
				if (prog_str[work] == 0x00 || prog_str[work] == 0x20) prog_str[work] = 0x30;
			}

			prog_str[8] = 0;
			return prog_str;
		}

		default: {return "";}
	}
}


/* program editing */
void menu_prog_edit_start(uint16_t id)
{
	uint8_t cursor = 0;

	menu_editing = id;

	switch (id) {
		case PROG_EDIT_PROG_DWELL: {
			menu_newval = menu_oldval = settings.led_prog[prog_type][prog_idx].dwell;
			break;
		}

		case PROG_GLOBAL_CHANGEIDX: {
			menu_newval = menu_oldval = prog_idx;
			break;
		}

		case PROG_EDIT_PROG_ID: {
			menuc = (MenuItem *)&menu_prog_ledaitem;
			strcpy(lcd_line[0], "Choose: ");
			menu_oldval = settings.led_prog[prog_type][prog_idx].progidx;
			cursor = 1;
			break;
		}

		case PROG_EDIT_CYCLESPEED: {
			menuc = (MenuItem *)&menu_prog_ledaitem;
			strcpy(lcd_line[0], "CycleSpd");
			menu_oldval = settings.led_prog[prog_type][prog_idx].wait;
			cursor = 1;
			break;
		}
	}

	if (!cursor) {
		lcd_set_cursor(LCD_LINE_2 + 7, LCD_CMD_CURSOR_FLASH);
	}
}

void prog_edit_tog_mode(uint16_t id)
{
	uint8_t mode;

	mode = (settings.led_prog_mode >> (prog_type << 1)) & 0b11;
	mode++;

	if (mode > 2) mode = 0;

	settings.led_prog_mode &= ~(0b11 << (prog_type << 1));
	settings.led_prog_mode |= (mode << (prog_type << 1));
}

void prog_edit_tog_enable(uint16_t id)
{
	if (prog_idx < PIRATE_PROG_SAVED_MAX) {
		if (settings.led_prog[prog_type][prog_idx].type & 0x80) {
			settings.led_prog[prog_type][prog_idx].type &= 0x7f;
		} else {
			settings.led_prog[prog_type][prog_idx].type |= 0x80;
		}
	}
}

void prog_edit_tog_type(uint16_t id)
{
	if (prog_idx < PIRATE_PROG_SAVED_MAX) {
		if (settings.led_prog[prog_type][prog_idx].type & 0x01) {
			settings.led_prog[prog_type][prog_idx].type &= 0xfe;
		} else {
			settings.led_prog[prog_type][prog_idx].type |= 0x01;
		}
	}
}

void prog_edit_tog_force_init(uint16_t id)
{
	if (prog_idx < PIRATE_PROG_SAVED_MAX) {
		if (settings.led_prog[prog_type][prog_idx].type & 0x02) {
			settings.led_prog[prog_type][prog_idx].type &= 0xfd;
		} else {
			settings.led_prog[prog_type][prog_idx].type |= 0x02;
		}
	}
}


/* program button handlers */
static void menu_btn_prog_prev()
{
	uint8_t do_beep = 0;
	uint32_t work;

	switch (menu_editing) {
		case 0: {
			menu_btn_prev();
			break;
		}

		case PROG_EDIT_PROG_DWELL: {
			if (menu_newval > 1) {
				menu_newval--;
				settings.led_prog[prog_type][prog_idx].dwell = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}

		case PROG_GLOBAL_CHANGEIDX: {
			if (menu_newval) {
				menu_newval--;
				prog_idx = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}
		case PROG_EDIT_PROG_ID: {
			if (settings.led_prog[prog_type][prog_idx].progidx) {
				settings.led_prog[prog_type][prog_idx].progidx--;
				do_beep = settings.beeper;
			}
			break;
		}
		case PROG_EDIT_CYCLESPEED: {
			work = settings.led_prog[prog_type][prog_idx].wait;
			do_beep = settings.beeper;

			if (work > 1000) {
				settings.led_prog[prog_type][prog_idx].wait -= 50;
			} else if (work > 100) {
				settings.led_prog[prog_type][prog_idx].wait -= 20;
			} else if (work > 10) {
				settings.led_prog[prog_type][prog_idx].wait -= 5;
			} else if (work > 1) {
				settings.led_prog[prog_type][prog_idx].wait--;
			} else {
				do_beep = 0;
			}

			break;
		}
	}

	if (do_beep) {
		beep(settings.beep_type[0], 10);
	}

	lcd_menu_linescroll_reset(1);
}

static void menu_btn_prog_next()
{
	uint8_t do_beep = 0;
	uint32_t work;

	switch (menu_editing) {
		case 0: {
			menu_btn_next();
			break;
		}

		case PROG_EDIT_PROG_DWELL: {
			if (menu_newval < 240) {
				menu_newval++;
				settings.led_prog[prog_type][prog_idx].dwell = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}

		case PROG_GLOBAL_CHANGEIDX: {
			if (menu_newval < PIRATE_PROG_SAVED_MAX - 1) {
				menu_newval++;
				prog_idx = menu_newval;
				do_beep = settings.beeper;
			}
			break;
		}
		case PROG_EDIT_PROG_ID: {
			if (settings.led_prog[prog_type][prog_idx].progidx < (prog_count[prog_type] - 1)) {
				settings.led_prog[prog_type][prog_idx].progidx++;
				do_beep = settings.beeper;
			}
			break;
		}
		case PROG_EDIT_CYCLESPEED: {
			work = settings.led_prog[prog_type][prog_idx].wait;
			do_beep = settings.beeper;

			if (work < 10) {
				settings.led_prog[prog_type][prog_idx].wait++;
			} else if (work < 100) {
				settings.led_prog[prog_type][prog_idx].wait += 5;
			} else if (work < 1000) {
				settings.led_prog[prog_type][prog_idx].wait += 20;
			} else if (work < 5000) {
				settings.led_prog[prog_type][prog_idx].wait += 50;
			} else {
				do_beep = 0;
			}

			break;
		}
	}

	if (do_beep) {
		beep(settings.beep_type[0], 10);
	}

	lcd_menu_linescroll_reset(1);
}

static void menu_btn_prog_menu()
{
	switch (menu_editing) {
		case 0: {
			menuc = (MenuItem *)&menu_prog_ritem[prog_type];

			prog_type = 0;
			prog_idx = 0;

			menu_btntype_menus();

			if (settings.beeper) {
				beep(settings.beep_type[0] - 1, 10);
			}

			return;
		}

		case PROG_EDIT_PROG_DWELL: {
			settings.led_prog[prog_type][prog_idx].dwell = menu_oldval;
			break;
		}

		case PROG_GLOBAL_CHANGEIDX: {
			prog_idx = menu_oldval;
			break;
		}

		case PROG_EDIT_PROG_ID: {
			settings.led_prog[prog_type][prog_idx].progidx = menu_oldval;
			strcpy(lcd_line[0], prog_type_edit_str(prog_type));
			menuc = (MenuItem *)&menu_prog_leditem[5];
			break;
		}
		case PROG_EDIT_CYCLESPEED: {
			settings.led_prog[prog_type][prog_idx].wait = menu_oldval;
			strcpy(lcd_line[0], prog_type_edit_str(prog_type));
			menuc = (MenuItem *)&menu_prog_leditem[6];
			break;
		}
	}

	menu_editing = 0;

	lcd_set_cursor(LCD_LINE_2 + 7, LCD_CMD_NO_CURSOR_FLASH);

	if (settings.beeper) {
		beep(settings.beep_type[0], 10);
	}
}

static void menu_btn_prog_ok()
{
	switch (menu_editing) {
		case 0: {
			menu_btn_ok();
			return;
		}

		case PROG_GLOBAL_CHANGEIDX: {
			strcpy(lcd_line[0], prog_type_edit_str(prog_type));
			break;
		}

		case PROG_EDIT_PROG_ID: {
			strcpy(lcd_line[0], prog_type_edit_str(prog_type));
			menuc = (MenuItem *)&menu_prog_leditem[5];
			break;
		}
		case PROG_EDIT_CYCLESPEED: {
			strcpy(lcd_line[0], prog_type_edit_str(prog_type));
			menuc = (MenuItem *)&menu_prog_leditem[6];
			break;
		}
	}

	menu_editing = 0;

	lcd_set_cursor(LCD_LINE_2 + 7, LCD_CMD_NO_CURSOR_FLASH);

	if (settings.beeper) {
		beep(settings.beep_type[0] + 1, 10);
	}
}

static void menu_prog_hexedit_adj(uint32_t val, uint8_t offset, uint8_t size, uint8_t direction)
{
	uint32_t mask;
	uint32_t work;
	uint8_t nybble;

	nybble = (offset + size - menu_oldval - 1) << 2;

	// create a mask of the affected bits
	mask = (0xf << nybble);
	// extract and shift the working bits
	work = ((val & mask) >> (nybble)) & 0xf;
	// clear working bits in value
	val &= ~mask;
	// increment/decrement them
	if (direction) work++; else work--;
	// shift them back and restore them to value
	val |= (work << nybble) & mask;

	switch (menu_editing) {
		case PROG_EDIT_LED_LEVEL: {
			settings.led_prog[prog_type][prog_idx].level = (uint16_t)val;
			break;
		}
		case PROG_EDIT_LED_OFFSET: {
			settings.led_prog[prog_type][prog_idx].offset = val;
			break;
		}
		case PROG_EDIT_LED_OPTIONS: {
			settings.led_prog[prog_type][prog_idx].option = val;
			break;
		}
	}

}

static void menu_prog_hexedit_prev()
{
	uint32_t val = 0;
	uint8_t size = 8;
	uint8_t start = 0;

	switch (menu_editing) {
		case PROG_EDIT_LED_LEVEL: {
			val = settings.led_prog[prog_type][prog_idx].level;
			size = 4;
			start = 3;
			break;
		}
		case PROG_EDIT_LED_OFFSET: {
			val = settings.led_prog[prog_type][prog_idx].offset;
			break;
		}
		case PROG_EDIT_LED_OPTIONS: {
			val = settings.led_prog[prog_type][prog_idx].option;
			break;
		}
	}


	if (menu_newval) {
		menu_prog_hexedit_adj(val, start, size, 0);
	} else {
		if (menu_oldval > start) {
			menu_oldval--;
		}
	}

	if (settings.beeper) {
		beep(settings.beep_type[0], 10);
	}

	lcd_set_cursor(LCD_LINE_2 + menu_oldval, 0);
}

static void menu_prog_hexedit_next()
{
	uint32_t val = 0;
	uint8_t size = 8;
	uint8_t start = 0;
	uint8_t end = 7;

	switch (menu_editing) {
		case PROG_EDIT_LED_LEVEL: {
			val = settings.led_prog[prog_type][prog_idx].level;
			size = 4;
			start = 3;
			end = 6;
			break;
		}
		case PROG_EDIT_LED_OFFSET: {
			val = settings.led_prog[prog_type][prog_idx].offset;
			break;
		}
		case PROG_EDIT_LED_OPTIONS: {
			val = settings.led_prog[prog_type][prog_idx].option;
			break;
		}
	}


	if (menu_newval) {
		menu_prog_hexedit_adj(val, start, size, 1);
	} else {
		if (menu_oldval < end) {
			menu_oldval++;
		}
	}

	if (settings.beeper) {
		beep(settings.beep_type[0], 10);
	}

	lcd_set_cursor(LCD_LINE_2 + menu_oldval, 0);
}

static void menu_prog_hexedit_ok()
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


static void menu_prog_hexedit_menu()
{
	uint8_t id = 0;

	switch (menu_editing) {
		case PROG_EDIT_LED_LEVEL: 	{id = 7; break;}
		case PROG_EDIT_LED_OFFSET: 	{id = 8; break;}
		case PROG_EDIT_LED_OPTIONS: {id = 9; break;}
	}

	strcpy(lcd_line[0], prog_type_edit_str(prog_type));

	menu_editing = 0;

	lcd_set_cursor(LCD_LINE_2 + 7, LCD_CMD_NO_CURSOR_FLASH);

	menuc = (MenuItem *)&menu_prog_leditem[id];
	menu_btntype_program();

	if (settings.beeper) {
		beep(settings.beep_type[0] - 1, 10);
	}
}

void menu_btntype_program()
{
	menu_mode = MENU_MODE_PROGRAM_EDITOR;

	lcd_btn_fn[BTN_UP] = menu_btn_prog_prev;
	lcd_btn_fn[BTN_MENU] = menu_btn_prog_menu;
	lcd_btn_fn[BTN_OK] = menu_btn_prog_ok;
	lcd_btn_fn[BTN_DOWN] = menu_btn_prog_next;
}

static void menu_prog_enter_led(uint16_t id)
{
	prog_type = id;
	prog_idx = 0;

	menuc = (MenuItem *)&menu_prog_leditem[0];

	menu_btntype_program();

	strcpy(lcd_line[0], prog_type_edit_str(id));
}

static void menu_prog_hexedit_start(uint16_t id)
{
	// oldval is cursor pos, newval is editing-is-on
	menu_newval = 0;
	menu_editing = id;

	menuc = (MenuItem *)&menu_prog_ledaitem;

	switch (id) {
		case PROG_EDIT_LED_LEVEL: {
			strcpy(lcd_line[0], "Level:  ");
			menu_oldval = 3;
			prog_hex_val = settings.led_prog[prog_type][prog_idx].level;
			break;
		}
		case PROG_EDIT_LED_OFFSET: {
			strcpy(lcd_line[0], "Offset: ");
			menu_oldval = 0;
			prog_hex_val = settings.led_prog[prog_type][prog_idx].offset;
			break;
		}
		case PROG_EDIT_LED_OPTIONS: {
			strcpy(lcd_line[0], "Options:");
			menu_oldval = 0;
			prog_hex_val = settings.led_prog[prog_type][prog_idx].option;
			break;
		}
	}

	lcd_set_cursor(LCD_LINE_2 + menu_oldval, LCD_CMD_CURSOR_UNDER);

	lcd_btn_fn[BTN_UP] = menu_prog_hexedit_prev;
	lcd_btn_fn[BTN_MENU] = menu_prog_hexedit_menu;
	lcd_btn_fn[BTN_OK] = menu_prog_hexedit_ok;
	lcd_btn_fn[BTN_DOWN] = menu_prog_hexedit_next;
}


/* menu construction */
  // programs menu
const MenuData menu_prog_rdata[] = {
	{"", prog_type_edit_str,    PROG_TYPE_SKULL, 0, 0, 0, 0, menu_prog_enter_led, PROG_TYPE_SKULL},
	{"", prog_type_edit_str,    PROG_TYPE_BONES, 0, 0, 0, 0, menu_prog_enter_led, PROG_TYPE_BONES},
	{"", prog_type_edit_str,     PROG_TYPE_EYES, 0, 0, 0, 0, menu_prog_enter_led, PROG_TYPE_EYES},
	{"", prog_type_edit_str, PROG_TYPE_INFOPIR8, 0, 0, 0, 0, NULL, 0},
};
const MenuItem menu_prog_ritem[] = {
	{&menu_ritem[LCD_MENU_ROOT_PROGRAMS],                   0, &menu_prog_ritem[1], 0, &menu_prog_rdata[0]},
	{&menu_ritem[LCD_MENU_ROOT_PROGRAMS], &menu_prog_ritem[0], &menu_prog_ritem[2], 0, &menu_prog_rdata[1]},
	{&menu_ritem[LCD_MENU_ROOT_PROGRAMS], &menu_prog_ritem[1], &menu_prog_ritem[3], 0, &menu_prog_rdata[2]},
	{&menu_ritem[LCD_MENU_ROOT_PROGRAMS], &menu_prog_ritem[2],                   0, 0, &menu_prog_rdata[3]},
};

const MenuData menu_prog_leddata[] = {
	{"Mode",     menu_prog_disp,      PROG_GLOBAL_MODE, 0, 0, 0, 0,       prog_edit_tog_mode,                     0},
	{"Edit ",    menu_prog_disp, PROG_GLOBAL_CHANGEIDX, 0, 0, 0, 0,     menu_prog_edit_start, PROG_GLOBAL_CHANGEIDX},
	{"Enable ",  menu_prog_disp,      PROG_EDIT_ENABLE, 0, 0, 0, 0,     prog_edit_tog_enable,                     0},
	{"Dwell",    prog_dwell_str,  PROG_EDIT_PROG_DWELL, 0, 0, 0, 0,     menu_prog_edit_start,  PROG_EDIT_PROG_DWELL},
	{"Type",     menu_prog_disp,        PROG_EDIT_TYPE, 0, 0, 0, 0,       prog_edit_tog_type,                     0},
	{"PID  #",   menu_prog_disp,     PROG_EDIT_PROG_ID, 0, 0, 0, 0,     menu_prog_edit_start,     PROG_EDIT_PROG_ID},
	{"CycleSpd",           NULL,                     0, 0, 0, 0, 0,     menu_prog_edit_start,  PROG_EDIT_CYCLESPEED},
	{"LEDLevel",           NULL,                     0, 0, 0, 0, 0,  menu_prog_hexedit_start,   PROG_EDIT_LED_LEVEL},
	{"Offset",             NULL,                     0, 0, 0, 0, 0,  menu_prog_hexedit_start,  PROG_EDIT_LED_OFFSET},
	{"Options",            NULL,                     0, 0, 0, 0, 0,  menu_prog_hexedit_start, PROG_EDIT_LED_OPTIONS},
	{"InitFn?",  menu_prog_disp,  PROG_EDIT_FORCE_INIT, 0, 0, 0, 0, prog_edit_tog_force_init,  PROG_EDIT_FORCE_INIT},
};
const MenuItem menu_prog_leditem[] = {
	{(MenuItem *)255,                     0,  &menu_prog_leditem[1], 0, &menu_prog_leddata[0]},
	{(MenuItem *)255, &menu_prog_leditem[0],  &menu_prog_leditem[2], 0, &menu_prog_leddata[1]},
	{(MenuItem *)255, &menu_prog_leditem[1],  &menu_prog_leditem[3], 0, &menu_prog_leddata[2]},
	{(MenuItem *)255, &menu_prog_leditem[2],  &menu_prog_leditem[4], 0, &menu_prog_leddata[3]},
	{(MenuItem *)255, &menu_prog_leditem[3],  &menu_prog_leditem[5], 0, &menu_prog_leddata[4]},
	{(MenuItem *)255, &menu_prog_leditem[4],  &menu_prog_leditem[6], 0, &menu_prog_leddata[5]},
	{(MenuItem *)255, &menu_prog_leditem[5],  &menu_prog_leditem[7], 0, &menu_prog_leddata[6]},
	{(MenuItem *)255, &menu_prog_leditem[6],  &menu_prog_leditem[8], 0, &menu_prog_leddata[7]},
	{(MenuItem *)255, &menu_prog_leditem[7],  &menu_prog_leditem[9], 0, &menu_prog_leddata[8]},
	{(MenuItem *)255, &menu_prog_leditem[8], &menu_prog_leditem[10], 0, &menu_prog_leddata[9]},
	{(MenuItem *)255, &menu_prog_leditem[9],                      0, 0, &menu_prog_leddata[10]},
};

  // advanced LED menu editor target menu
const MenuData menu_prog_ledadata = {
		"", prog_advanced_str, 0, 4, LCD_DEF_SPACING, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0
};
const MenuItem menu_prog_ledaitem = {(MenuItem *)255, 0, 0, 0, &menu_prog_ledadata};
