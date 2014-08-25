/*
 * menu_program.h: led and lcd program selection and configuration
 * 2014 by true
 *
 * ----
 *
 * $Id$
 */


#ifndef __PIRATE_MENU_PROGRAM_H
#define __PIRATE_MENU_PROGRAM_H


/* menus */
#include "../pirate.h"
#include "lcd_menu.h"


/* program specifics */
#define PROG_TYPE_SKULL 		0x00
#define PROG_TYPE_BONES 		0x01
#define PROG_TYPE_EYES 			0x02
#define PROG_TYPE_INFOPIR8 		0x10

#define PROG_GLOBAL_MODE 		0x0200
#define PROG_GLOBAL_CHANGEIDX 	0x0210

#define PROG_EDIT_ENABLE 		0x0220
#define PROG_EDIT_FORCE_INIT 	0x0221
#define PROG_EDIT_PROG_DWELL 	0x0222
#define PROG_EDIT_TYPE 			0x0223
#define PROG_EDIT_PROG_ID 		0x0224
#define PROG_EDIT_CYCLESPEED 	0x0225
#define PROG_EDIT_LED_LEVEL 	0x0226
#define PROG_EDIT_LED_OFFSET 	0x0227
#define PROG_EDIT_LED_OPTIONS 	0x0228


/* program menus */
extern const MenuItem menu_prog_ritem[];
extern const MenuItem menu_prog_leditem[];


/* variables */
extern uint8_t prog_id[3];
extern uint16_t prog_dwell[3];


/* prototypes */
void prog_init();
uint8_t prog_get_mode(uint8_t id);

uint8_t menu_prog_get_type();
uint8_t menu_prog_get_idx();

void menu_btntype_program();


#endif
