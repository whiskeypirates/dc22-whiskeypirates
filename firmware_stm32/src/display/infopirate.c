/*
 * infopirate.c: namebadge and other information animated 8x2 display
 * 2014 by true
 *
 * ----
 *
 * $Id: infopirate.c 214 2014-08-04 05:31:33Z true $
 */


#include "infopirate.h"
#include "infopirate_prog.h"

#include "../menu/lcd_menu.h"

char lcd_line[2][9]; 				// lcd.h

static uint8_t prog_idx; 			// currently selected program
static uint8_t prog_loop; 			// current loop thru program

static uint8_t (*infopirate_prog)(void);
static uint8_t (*infopirate_prog_list[])() = {
	&infopirate_prog_scroll_name_in_left,
	&infopirate_prog_nameonly,
	&infopirate_prog_scroll_name_out_left,
	&infopirate_prog_logo01,
	&infopirate_prog_logo02
};
const uint8_t prog_loop_run[] = { 	// amount of passes per program in static mode
	1,
	10,
	1,
	1,
	1
};


/* functions */
void infopirate_init()
{
	prog_wait = 0;
	prog_loop = 0;
	prog_idx = 0;
	infopirate_prog = NULL;
}

static void infopirate_next_prog()
{
	prog_wait = 0; 		// infopirate_prog.h
	prog_loop = 0;
	prog_idx++;

	if (prog_idx >= sizeof_array(infopirate_prog_list)) prog_idx = 0;

	infopirate_prog = infopirate_prog_list[prog_idx];
}

void infopirate_update()
{
	uint8_t state;

	state = INFOPIRATE_PROG_NONE;

	if (infopirate_prog != NULL) {
		state = infopirate_prog();
	}

	switch (state) {
		case INFOPIRATE_PROG_NONE: {
			prog_idx = 0;
			prog_loop = 0;
			infopirate_prog = infopirate_prog_list[0];
			break;
		}
		case INFOPIRATE_PROG_DONE: {
			prog_loop++;
			if (prog_loop >= prog_loop_run[prog_idx]) {
				infopirate_next_prog();
			}
			break;
		}
	}
}

void menu_infopirate_btn_prev()
{

}

void menu_infopirate_btn_next()
{

}

void menu_infopirate_btn_ok()
{

}
