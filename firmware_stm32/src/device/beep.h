/*
 * beep.h: beeper fuckin musicality bitches
 * 2014 by true
 *
 * ----
 *
 * $Id: beep.h 201 2014-08-02 19:42:47Z true $
 *
 * do what you want 'cause a pirate is free
 * you are a pirate
 */


#ifndef __PIRATE_DEV_BEEP_H
#define __PIRATE_DEV_BEEP_H


#include "gpio.h"


#define BEEP_TIM 				TIM7

#define BEEP_PLAYLIST_COUNT 	20
#define BEEP_SONG_DONE 			0xffff

#define BEEP_BUF_SIZE 			8 		// max 127

#define NOTE_NONE 		255
#define NOTE_B 			11
#define NOTE_BFLAT 		10
#define NOTE_ASHARP 	10
#define NOTE_A 			9
#define NOTE_AFLAT 		8
#define NOTE_GSHARP 	8
#define NOTE_G 			7
#define NOTE_GFLAT 		6
#define NOTE_FSHARP 	6
#define NOTE_F 			5
#define NOTE_E 			4
#define NOTE_EFLAT 		3
#define NOTE_DSHARP 	3
#define NOTE_D 			2
#define NOTE_DFLAT 		1
#define NOTE_CSHARP 	1
#define NOTE_C 			0


typedef struct MusicSong {
	uint8_t octave;
	uint8_t note;
	uint32_t duration;
} MusicSong;

typedef struct MusicPlaylist {
	MusicSong *song;
	uint16_t note_count;
} MusicPlaylist;


void beep_init();
void beep_update();

void beep_clear_queue();
uint8_t beep(uint16_t tone_id, uint16_t duration);
// void beeper_song(uint16_t song_idx, uint16_t spacing);


#endif
