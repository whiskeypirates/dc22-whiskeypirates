/*
 * beep_music.h: musical scores for pirates
 * 2014 by true
 *
 * ----
 *
 * $Id: beep_song.h 160 2014-07-04 19:23:24Z true $
 */


static MusicSong you_are_a_pirate[] = {
		{5, NOTE_E, 200},
		{5, NOTE_C, 200},
		{5, NOTE_C, 200},
		{5, NOTE_C, 200},
		{4, NOTE_G, 200},
		{4, NOTE_A, 200},
		{5, NOTE_C, 200},
		{4, NOTE_B, 200},
		{4, NOTE_D, 200},
		{4, NOTE_A, 250},
		{0, NOTE_NONE, 350},
		{4, NOTE_G, 600},
		{5, NOTE_C, 600},
		{5, NOTE_C, 200},
		{5, NOTE_D, 360},
		{4, NOTE_A, 360}
};

static MusicPlaylist you_are_a_pirate_s = {you_are_a_pirate, 16};
