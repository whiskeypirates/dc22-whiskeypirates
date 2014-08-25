/*
 * menu_testing.c: test menu and testing functions
 * 2014 by true
 *
 * ----
 *
 * $Id$
 */


#include "menu_testing.h"

  // settings
PirateSettings settings; 	// pirate.h

  // menus
MenuItem *menuc; 			// lcd_menu.h

  // testing
static uint8_t buzzer_select;
static uint8_t matrix_select;


/* menu functions */
static char * lcd_menu_testing_buzzer(uint16_t id)
{
	return pirate_sitoa(buzzer_select, 10, 0);
}

static char * lcd_menu_testing_matrix(uint16_t id)
{
	return pirate_sitoa(matrix_select, 10, 0);
}

static char * lcd_menu_testing_boneled(uint16_t id)
{
	return pirate_sitoa(matrix_select, 10, 0);
}

static char * lcd_menu_testing_settings_bytes(uint16_t id)
{
	static char size[12];

	strcpy(size, pirate_sitoa(sizeof(settings), 10, 0));
	strncat(size, "bytes", 12);

	return size;
}

/* menu construction */
const MenuData menu_test_rdata[] = {
	{"Buzz  ",  lcd_menu_testing_buzzer,         0, 0,                 0,                  0,                  0, NULL, 0},
	{"MPurp ",  lcd_menu_testing_matrix,         0, 0,                 0,                  0,                  0, NULL, 0},
	{"MWht  ",  lcd_menu_testing_matrix,         0, 0,                 0,                  0,                  0, NULL, 0},
	{"Bone  ",  lcd_menu_testing_boneled,        0, 0,                 0,                  0,                  0, NULL, 0},
	{"NVRAM",   lcd_menu_testing_settings_bytes, 0, 10, LCD_DEF_SPACING, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
};

const MenuItem menu_test_ritem[] = {
	{&menu_ritem[LCD_MENU_ROOT_TESTING],                   0, &menu_test_ritem[1], 0, &menu_test_rdata[0]},
	{&menu_ritem[LCD_MENU_ROOT_TESTING], &menu_test_ritem[0], &menu_test_ritem[2], 0, &menu_test_rdata[1]},
	{&menu_ritem[LCD_MENU_ROOT_TESTING], &menu_test_ritem[1], &menu_test_ritem[3], 0, &menu_test_rdata[2]},
	{&menu_ritem[LCD_MENU_ROOT_TESTING], &menu_test_ritem[2], &menu_test_ritem[4], 0, &menu_test_rdata[3]},
	{&menu_ritem[LCD_MENU_ROOT_TESTING], &menu_test_ritem[3],                   0, 0, &menu_test_rdata[4]},
};
