/*
 * menu_settings_defprog.h: firmware default LED programs
 * 2014 by true
 *
 * $Id: menu_settings_defprog.h 220 2014-08-07 15:50:32Z true $
 */

/* SKULL */
// random flashing
settings.led_prog[0][0].type = 0b10000000; 	// enabled, no init, program mode
settings.led_prog[0][0].dwell = 8;
settings.led_prog[0][0].progidx = 4;
settings.led_prog[0][0].wait = 80;
settings.led_prog[0][0].level = 0xffe0;
settings.led_prog[0][0].offset = 0x00;
settings.led_prog[0][0].option = 0x03;

// loops with trails clockwise
settings.led_prog[0][1].type = 0b10000000; 	// enabled, no init, program mode
settings.led_prog[0][1].dwell = 7;
settings.led_prog[0][1].progidx = 1;
settings.led_prog[0][1].wait = 80;
settings.led_prog[0][1].level = 0xfff0;
settings.led_prog[0][1].offset = 0x80;
settings.led_prog[0][1].option = 0xc3;

// loops with trails white changes direction
settings.led_prog[0][2].type = 0b10000000; 	// enabled, no init, program mode
settings.led_prog[0][2].dwell = 7;
settings.led_prog[0][2].progidx = 3;
settings.led_prog[0][2].wait = 80;
settings.led_prog[0][2].level = 0xfff0;
settings.led_prog[0][2].offset = 0x00000000;
settings.led_prog[0][2].option = 0x400f00c7;

// purple haze
settings.led_prog[0][3].type = 0b10000000; 	// enabled, no init, program mode
settings.led_prog[0][3].dwell = 3;
settings.led_prog[0][3].progidx = 4;
settings.led_prog[0][3].wait = 15;
settings.led_prog[0][3].level = 0xff10;
settings.led_prog[0][3].offset = 0x00;
settings.led_prog[0][3].option = 0xc3;


/* BONES */
// clockwise rotation
settings.led_prog[1][0].type = 0b10000000; 	// enabled, no init, program mode
settings.led_prog[1][0].dwell = 15;
settings.led_prog[1][0].progidx = 1;
settings.led_prog[1][0].wait = 160;
settings.led_prog[1][0].level = 0xf0;
settings.led_prog[1][0].offset = 0x00;
settings.led_prog[1][0].option = 0x50;

// up-down scrubbing
settings.led_prog[1][1].type = 0b10000010; 	// enabled, init, program mode
settings.led_prog[1][1].dwell = 9;
settings.led_prog[1][1].progidx = 2;
settings.led_prog[1][1].wait = 100;
settings.led_prog[1][1].level = 0xff;
settings.led_prog[1][1].offset = 0x00;
settings.led_prog[1][1].option = 0x50;

// all on
settings.led_prog[1][2].type = 0b10000010; 	// enabled, init, program mode
settings.led_prog[1][2].dwell = 1;
settings.led_prog[1][2].progidx = 0;
settings.led_prog[1][2].wait = 80;
settings.led_prog[1][2].level = 0xf0;
settings.led_prog[1][2].offset = 0x00;
settings.led_prog[1][2].option = 0x00;


/* EYES */
// red with green flashes
settings.led_prog[2][0].type = 0b10000000; 	// enabled, no init, program mode
settings.led_prog[2][0].dwell = 15;
settings.led_prog[2][0].progidx = 2;
settings.led_prog[2][0].wait = 30;
settings.led_prog[2][0].level = 0x00;
settings.led_prog[2][0].offset = 0x00f000e6;
settings.led_prog[2][0].option = 0x14000001;

// pink-ized "cop mode"
settings.led_prog[2][1].type = 0b10000000; 	// enabled, no init, program mode
settings.led_prog[2][1].dwell = 10;
settings.led_prog[2][1].progidx = 2;
settings.led_prog[2][1].wait = 50;
settings.led_prog[2][1].level = 0x00;
settings.led_prog[2][1].offset = 0x0000f090;
settings.led_prog[2][1].option = 0x20001001;

// candle flicker with favcolor
settings.led_prog[2][2].type = 0b10000000; 	// enabled, no init, program mode
settings.led_prog[2][2].dwell = 18;
settings.led_prog[2][2].progidx = 1;
settings.led_prog[2][2].wait = 80;
settings.led_prog[2][2].level = 0x00;
settings.led_prog[2][2].offset = 0x00000000;
settings.led_prog[2][2].option = 0x00000000;

// turquoise-type pulse with red fail-flicker
settings.led_prog[2][3].type = 0b10000000; 	// enabled, no init, program mode
settings.led_prog[2][3].dwell = 7;
settings.led_prog[2][3].progidx = 2;
settings.led_prog[2][3].wait = 20;
settings.led_prog[2][3].level = 0x00;
settings.led_prog[2][3].offset = 0x00f93012;
settings.led_prog[2][3].option = 0x70000001;
