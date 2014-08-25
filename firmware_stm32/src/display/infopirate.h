/*
 * infopirate.h: namebadge and other information animated 8x2 display
 * 2014 by true
 *
 * ----
 *
 * $Id: infopirate.h 214 2014-08-04 05:31:33Z true $
 */


#ifndef __PIRATE_DISPLAY_INFOPIRATE_H
#define __PIRATE_DISPLAY_INFOPIRATE_H


#include "pirate.h"
#include "cgram/cgram.h"


#define INFOPIRATE_PROG_NONE 		0x00
#define INFOPIRATE_PROG_PAUSED 		0x01
#define INFOPIRATE_PROG_RUNNING 	0x02
#define INFOPIRATE_PROG_DONE 		0xFE


/* variables */


/* prototypes */
void infopirate_init();

void infopirate_update();

void menu_infopirate_btn_prev();
void menu_infopirate_btn_next();
void menu_infopirate_btn_ok();


#endif
