/*
 * lcd_menu.c: lcd menuing prototypes
 * 2014 by true
 *
 * ----
 *
 * $Id$
 */


#ifndef __PIRATE_LCD_MENU_H
#define __PIRATE_LCD_MENU_H


#include <string.h>
#include "../device/lcd.h"
#include "../device/beep.h"


#define LCD_ROOT_TEXT 				"T3K 2014 Badge v0.2e by true"

#define LCD_DEF_SPACING 			4
#define LCD_DEF_SCROLL_WAIT 		144
#define LCD_DEF_SCROLL_RUN 			16

#define LCD_MENU_ROOT_SETTINGS 		1
#define LCD_MENU_ROOT_PROGRAMS 		2
#define LCD_MENU_ROOT_SENSORS 		4
#define LCD_MENU_ROOT_TESTING 		5
#define LCD_MENU_ROOT_CREDITS 		6

#define BTN_UP 						0
#define BTN_MENU 					1
#define BTN_OK 						2
#define BTN_DOWN 					3

#define MENU_MODE_NAVIGATING_MENUS 	0
#define MENU_MODE_EDITING 			1
#define MENU_MODE_RUNNING_PROGRAM 	2
#define MENU_MODE_PROGRAM_EDITOR 	3


/* struct */
typedef struct MenuData {
	char *disp; 					// string to display
	char * (*dispfn)(uint16_t id); 	// function which returns string to display
	uint16_t disp_id; 				// index to pass to dispfn
	uint8_t dispfn_delay; 			// how many 5000hz ticks to wait before re-calling dispfn
	uint8_t scroll_spaces; 			// how many spaces between scroll; 0 disables scroll
	uint8_t scroll_time_wait; 		// how long to wait before (re)scrolling
	uint8_t scroll_time_run; 		// how long to wait between characters
	void (*entryfn)(uint16_t eid); 	// function to call when entering menu
	uint16_t entry_id; 				// index to pass to entryfn
} MenuData;

typedef struct MenuItem MenuItem;
struct MenuItem {
	const MenuItem *root;
	const MenuItem *prev;
	const MenuItem *next;
	const MenuItem *enter;
	const MenuData *this;
};


/* lcd variables */
extern MenuItem *menuc;
extern const MenuItem menu_ritem[8];

extern void (*lcd_btn_fn[4])();


/* menu variables */
extern uint8_t menu_mode;

extern uint16_t menu_editing;
extern uint32_t menu_oldval;
extern uint32_t menu_newval;


/* menu constants */
extern const MenuItem menu_runitem;


/* prototypes */
void lcd_menu_init();
void lcd_menu_set(const MenuItem *set);
void lcd_menu_update();

void lcd_menu_linescroll_reset(uint8_t idx);

void menu_edit_start(uint16_t id);
void menu_root_run(uint16_t id);

void menu_btn_prev();
void menu_btn_next();
void menu_btn_menu();
void menu_btn_ok();

void menu_btntype_menus();
void menu_btntype_run();
void menu_btntype_editing();


#endif
