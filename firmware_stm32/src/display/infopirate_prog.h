/*
 * infopirate_prog.h: program functions for badge related bullshit
 * 2014 by true
 *
 * ----
 *
 * $Id$
 */


#include "../menu/lcd_menu.h"


static uint16_t prog_wait; 		// used by function to keep track of iteration time
static uint32_t prog_state[4]; 	// used by the function to keep track of state
static uint32_t prog_work[2]; 	// used for misc shit


/* cgram characters from cgram.h */
const uint8_t cgram_crown[6][8];


/* functions */
uint8_t infopirate_prog_nameonly()
{
	if (!prog_wait) {
		prog_wait = 100;
		lcd_set_height(LCD_CMD_DOUBLEHEIGHT);
	}

	strncpy(lcd_line[0], settings.name, 8);

	prog_wait--;
	return prog_wait ? INFOPIRATE_PROG_RUNNING : INFOPIRATE_PROG_DONE;
}

uint8_t infopirate_prog_scroll_name_in_left()
{
	if (!prog_wait) {
		prog_wait = 121;
		prog_state[0] = 8;
		lcd_set_height(LCD_CMD_DOUBLEHEIGHT);
	} else if (prog_wait % 15 == 0) {
		if (prog_state[0]) prog_state[0]--;

		strncpy(lcd_line[0], "        ", 8); 		// make the line all spaces
		if (prog_state[0] < 8) {
			lcd_line[0][prog_state[0]] = 0x00; 		// set our null
			strncat(lcd_line[0], settings.name, 8); // copy our name
		}
	}

	prog_wait--;
	return prog_wait ? INFOPIRATE_PROG_RUNNING : INFOPIRATE_PROG_DONE;
}

uint8_t infopirate_prog_scroll_name_out_left()
{
	int i, j;

	if (!prog_wait) {
		prog_wait = 121;
		prog_state[0] = 0;
		lcd_set_height(LCD_CMD_DOUBLEHEIGHT);
	} else if (prog_wait % 15 == 0) {
		if (prog_state[0] < 7) prog_state[0]++;

		strncpy(lcd_line[0], "        ", 8); 		// make the line all spaces
		j = 0;
		for (i = prog_state[0]; i < 8; i++) {
			lcd_line[0][j++] = settings.name[i];
		}
	}

	prog_wait--;
	return prog_wait ? INFOPIRATE_PROG_RUNNING : INFOPIRATE_PROG_DONE;
}

uint8_t infopirate_prog_logo01()
{
	int i;

	static const char line1[8] = {0x00, 0x01, 0x02, 'T', 'h', 'r', 'e', 'e'};
	static const char line2[8] = {0x03, 0x04, 0x05, 'K', 'i', 'n', 'g', 's'};

	if (!prog_wait) {
		// set our state timer
		prog_wait = 400;
		prog_state[0] = 0;

		// enable CGRAM
		lcd_set_height(LCD_CMD_SINGLEHEIGHT & 0xfe); 	// enable cgram

		// set the CGRAM data to load
		lcd_set_cgram_load(cgram_crown, sizeof_array(cgram_crown));

		// set data
		for (i = 0; i < 8; i++) {
			lcd_line[0][i] = line1[i];
			lcd_line[1][i] = line2[i];
		}
	}

	prog_wait--;
	return prog_wait ? INFOPIRATE_PROG_RUNNING : INFOPIRATE_PROG_DONE;
}

uint8_t infopirate_prog_logo02()
{
	// prog_state[0] is the current character on this line to print
	// prog_work[0] is the current character overall to print
	// prog_work[1] is the current position of the character
	if (!prog_wait) {
		// new setup
		prog_wait = 9;
		prog_state[0] = 0; 		// first character
		prog_work[0] = 0; 		// first character
		prog_work[1] = 8; 		// scrolling from right, set to last character

		// clear strings for new setup
		strncpy(lcd_line[0], "        ", 8);
		strncpy(lcd_line[1], "        ", 8);
	} else if (prog_work[0] == 16) {
		// we're done. wait a while
		prog_wait = 210;
		prog_work[0]++;
	} else if (prog_wait == 1) {
		// loop timeout expired, reload timer and process
		prog_wait = 9;

		static const char line1[] = "Whiskey ";
		static const char line2[] = " Pirates";

		lcd_set_height(LCD_CMD_SINGLEHEIGHT);

		// did our character reach the end?
		if (prog_work[1] <= prog_state[0]) {
			// then go to the next character and reset right position
			prog_work[0]++;
			prog_state[0] = prog_work[0] & 0x07;
			prog_work[1] = 8;
		}

		// do we have characters left to update?
		if (prog_work[0] < 16) {
			// working variables
			char *src = prog_work[0] < 8 ? (char *)line1 : (char *)line2;
			char *target = prog_work[0] < 8 ? lcd_line[0] : lcd_line[1];

			// set the character one less than prog_work[1] to our target
			target[prog_work[1] - 1] = src[prog_state[0]];
			// if this isn't the last one, set the current working character to a space
			if (prog_work[1] < 8) target[prog_work[1]] = 0x20;
			// and continue scrolling our character in to the left
			prog_work[1]--;
		}
	}

	prog_wait--;
	return (prog_wait != 10) ? INFOPIRATE_PROG_RUNNING : INFOPIRATE_PROG_DONE;
}
