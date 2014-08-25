/*
 * menu_credits.c: we are legit
 * 2014 by true
 *
 * ----
 *
 * $Id: menu_credits.c 212 2014-08-04 02:06:08Z true $
 */


#include "menu_credits.h"

  // menus
MenuItem *menuc; 	// lcd_menu.h


/* menu construction */
const MenuData menu_cred_rdata[] = {
	{"idea by rCON and true (at the last minute)",   NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT - 40, LCD_DEF_SCROLL_RUN - 4, NULL, 0},
	{"T3K shipmates Bad Kobold and the lord canon",  NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT - 40, LCD_DEF_SCROLL_RUN - 4, NULL, 0},
	{"a fire? don't blame us if you didn't know",    NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT - 40, LCD_DEF_SCROLL_RUN - 4, NULL, 0},
	{"financed by captain rCON's billions",          NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT - 40, LCD_DEF_SCROLL_RUN - 4, NULL, 0},
	{"code and design rushed by true",               NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT - 40, LCD_DEF_SCROLL_RUN - 4, NULL, 0},
	{"the lord canon's divorce",                     NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT - 40, LCD_DEF_SCROLL_RUN - 4, NULL, 0},
	{"keep it between the bones, guys",              NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT - 40, LCD_DEF_SCROLL_RUN - 4, NULL, 0},
	{"is your badge shit? did it break? blame BK",   NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT - 40, LCD_DEF_SCROLL_RUN - 4, NULL, 0},
	{"always  need moar cake from avah",             NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT - 40, LCD_DEF_SCROLL_RUN - 4, NULL, 0},
	{"do what you want 'cause a pirate is free",     NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT - 40, LCD_DEF_SCROLL_RUN - 4, NULL, 0},
	{"you are A PIRATE",                             NULL, 0, 0, 3, LCD_DEF_SCROLL_WAIT,      LCD_DEF_SCROLL_RUN + 2, NULL, 0},
};
const MenuItem menu_cred_ritem[] = {
	{&menu_ritem[LCD_MENU_ROOT_CREDITS],                   0,  &menu_cred_ritem[1], 0,  &menu_cred_rdata[0]},
	{&menu_ritem[LCD_MENU_ROOT_CREDITS], &menu_cred_ritem[0],  &menu_cred_ritem[2], 0,  &menu_cred_rdata[1]},
	{&menu_ritem[LCD_MENU_ROOT_CREDITS], &menu_cred_ritem[1],  &menu_cred_ritem[3], 0,  &menu_cred_rdata[2]},
	{&menu_ritem[LCD_MENU_ROOT_CREDITS], &menu_cred_ritem[2],  &menu_cred_ritem[4], 0,  &menu_cred_rdata[3]},
	{&menu_ritem[LCD_MENU_ROOT_CREDITS], &menu_cred_ritem[3],  &menu_cred_ritem[5], 0,  &menu_cred_rdata[4]},
	{&menu_ritem[LCD_MENU_ROOT_CREDITS], &menu_cred_ritem[4],  &menu_cred_ritem[6], 0,  &menu_cred_rdata[5]},
	{&menu_ritem[LCD_MENU_ROOT_CREDITS], &menu_cred_ritem[5],  &menu_cred_ritem[7], 0,  &menu_cred_rdata[6]},
	{&menu_ritem[LCD_MENU_ROOT_CREDITS], &menu_cred_ritem[6],  &menu_cred_ritem[7], 0,  &menu_cred_rdata[7]},
	{&menu_ritem[LCD_MENU_ROOT_CREDITS], &menu_cred_ritem[7],  &menu_cred_ritem[8], 0,  &menu_cred_rdata[8]},
	{&menu_ritem[LCD_MENU_ROOT_CREDITS], &menu_cred_ritem[8], &menu_cred_ritem[10], 0,  &menu_cred_rdata[9]},
	{&menu_ritem[LCD_MENU_ROOT_CREDITS], &menu_cred_ritem[9],                    0, 0, &menu_cred_rdata[10]},
};
