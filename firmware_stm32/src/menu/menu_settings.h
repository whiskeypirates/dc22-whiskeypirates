/*
 * menu_settings.h: setting shit up for pirates since
 * 2014 by true
 *
 * ----
 *
 * $Id$
 */


#ifndef __PIRATE_MENU_SETTINGS_H
#define __PIRATE_MENU_SETTINGS_H


/* menus */
#include "../pirate.h"
#include "lcd_menu.h"

extern const MenuItem menu_pref_ritem[];


/* settings defines */
#define SETTING_NAME 				0x0101

#define SETTING_AUTORUN_ENA 		0x0105
#define SETTING_ALWAYS_RUN_PROG_ENA	0x0106

#define SETTING_CONTRAST 			0x0107

#define SETTING_FAVCOLOR_RED 		0x0109
#define SETTING_FAVCOLOR_GREEN 		0x010a
#define SETTING_FAVCOLOR_BLUE 		0x010b

#define SETTING_BEEPER_ENA 			0x0110
#define SETTING_BEEP_TYPE_BUTTON 	0x0111
#define SETTING_BEEP_TYPE_PAGING 	0x0112
#define SETTING_BEEP_TYPE_ALARM 	0x0113

#define SETTING_BRITE_AUTO_ENA 		0x0120
#define SETTING_BRITE_AUTO_SPEED 	0x0121
#define SETTING_BRITE_LCD_AUTO 		0x0122
#define SETTING_BRITE_LCD_VALUE 	0x0123
#define SETTING_BRITE_AUTOGAIN_LO 	0x0124
#define SETTING_BRITE_AUTOGAIN_HI 	0x0125
#define SETTING_BRITE_THRESH0 		0x012a
#define SETTING_BRITE_THRESH1 		0x012b
#define SETTING_BRITE_THRESH2 		0x012c
#define SETTING_BRITE_THRESH3 		0x012d
#define SETTING_BRITE_THRESH4 		0x012e


/* prototypes */
void menu_settings_save(uint16_t id);

void settings_save();
void settings_restore(uint8_t load_defaults);


#endif
