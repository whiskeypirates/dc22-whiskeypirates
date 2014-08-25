/*
 * beep.c: beeper fuckin musicality bitches, beep buzz boop
 * 2014 by true
 *
 * ----
 *
 * $Id: beep.c 214 2014-08-04 05:31:33Z true $
 *
 * ----
 *
 * TODO: remove freq lookup table, replace with freq defines by note (A4 = 440 for example)
 * TODO: let you know that the device we chose cannot play music; it is a buzzer.
 * TODO: do what you want 'cause a pirate is free
 */


#include "beep.h"


static const tGPIO beep_pin = {GPIOA, GPIO_Pin_4, 4};
static GPIO_InitTypeDef beep_gpio;
// static TIM_TimeBaseInitTypeDef beep_tb;
// static DAC_InitTypeDef beep_dac;

// static uint16_t active_song = BEEP_SONG_DONE;
// static uint16_t beep_idx;
// static uint16_t beep_spacing;
// static uint16_t beep_space;

static uint8_t  beep_buf_idx;
static uint8_t  beep_play_idx;
static uint16_t beep_dur_buf[BEEP_BUF_SIZE];
static uint16_t beep_lev_buf[BEEP_BUF_SIZE];


// music
/*
static const uint16_t note_freq[6][12] = {
	{65, 69, 73, 78, 82, 87, 92, 98, 104, 110, 117, 123},
	{131, 139, 147, 156, 165, 175, 185, 196, 208, 220, 233, 247},
	{262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494},
	{523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988},
	{1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976},
	{2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951}
};
*/

/*
#include "beep_song.h"
static const MusicPlaylist *playlist[BEEP_PLAYLIST_COUNT] = {
	&you_are_a_pirate_s
};
*/


/* beeper bullshit */
void beep_init()
{
	/*
	NVIC_InitTypeDef nvic;

	// enable clocks
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	*/

	// set up beeper gpio struct
	beep_gpio.GPIO_Mode = GPIO_Mode_OUT;
	beep_gpio.GPIO_OType = GPIO_OType_PP;
	beep_gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
	beep_gpio.GPIO_Speed = GPIO_Speed_400KHz;
	beep_gpio.GPIO_Pin = beep_pin.pin;

	// and set it up
	GPIO_Init(beep_pin.port, &beep_gpio);

	// reset buffer index
	beep_buf_idx = 0;

	/*
	// set up the DAC timer
	TIM_TimeBaseStructInit(&beep_tb);
	beep_tb.TIM_Prescaler = (SystemCoreClock / 8000000) - 1;
	beep_tb.TIM_Period = 0;
	TIM_TimeBaseInit(BEEP_TIM, &beep_tb);

	TIM_ARRPreloadConfig(BEEP_TIM, ENABLE);

	// set up the timer interrupt
	TIM_ITConfig(BEEP_TIM, TIM_IT_Update, ENABLE);

	// and set interrupt priority
	nvic.NVIC_IRQChannel = TIM7_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 1;
	nvic.NVIC_IRQChannelSubPriority = 0;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	// finally make sure the timer is disabled
	TIM_Cmd(BEEP_TIM, DISABLE);
	*/

	// set up the DAC
	/*
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	beep_dac.DAC_Trigger = DAC_Trigger_T7_TRGO;
	beep_dac.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &beep_dac);
	DAC_Cmd(DAC_Channel_1, DISABLE);
	*/
}

void beep_update()
{
	uint16_t duration;
	uint16_t level;

	/*
	uint8_t n, o;

	if (active_song != BEEP_SONG_DONE) {
		if (beep_dur) {
			// continue note
			beep_dur--;
		} else if (beep_space) {
			if (beep_space == beep_spacing) {
				// stop note
				// DAC_Cmd(DAC_Channel_1, DISABLE);
				TIM_Cmd(BEEP_TIM, DISABLE);
				GPIO_ResetBits(beep_pin.port, beep_pin.pin);
			}

			// count down empty space
			beep_space--;
		} else {
			// stop note if playing
			// DAC_Cmd(DAC_Channel_1, DISABLE);
			TIM_Cmd(BEEP_TIM, DISABLE);
			GPIO_ResetBits(beep_pin.port, beep_pin.pin);

			// next note
			if (beep_idx >= playlist[active_song]->note_count) {
				// there aren't any more; we're done.
				active_song = BEEP_SONG_DONE;
			} else {
				// load next note values
				beep_space = beep_spacing * 5;
				beep_dur = playlist[active_song]->song[beep_idx].duration * 5;

				// set up DAC with values
				n = playlist[active_song]->song[beep_idx].note;
				if (n != NOTE_NONE) {
					o = playlist[active_song]->song[beep_idx].octave;

					// set up DAC to specified note
					TIM_SetAutoreload(BEEP_TIM, (uint16_t)((4000000 / note_freq[o][n]) - 1));
					TIM_SetCounter(BEEP_TIM, 1);
					TIM_GenerateEvent(BEEP_TIM, TIM_EventSource_Update);

					// re-enable
					// DAC_Cmd(DAC_Channel_1, ENABLE);
					TIM_Cmd(BEEP_TIM, ENABLE);
				}
			}

			// prepare next note
			beep_idx++;
		}
	}
	*/

	// do we have an active tone?
	if (beep_buf_idx & 0x80) {
		duration = beep_dur_buf[beep_play_idx];
		level = beep_lev_buf[beep_play_idx];

		if (duration) {
			if (level == 0) { 						// loudest
				GPIO_SetBits(beep_pin.port, beep_pin.pin);
			} else if (level == 32) { 				// silent
				GPIO_ResetBits(beep_pin.port, beep_pin.pin);
			} else if (duration % level == 0) { 	// tones
				GPIO_SetBits(beep_pin.port, beep_pin.pin);
			} else {
				GPIO_ResetBits(beep_pin.port, beep_pin.pin);
			}

			// count down the times
			beep_dur_buf[beep_play_idx]--;
		} else {
			// stop the tone
			GPIO_ResetBits(beep_pin.port, beep_pin.pin);

			// move to the next buffer
			beep_play_idx++;

			// is there any more? need to reset buffer if there isn't any more
			if (((beep_buf_idx & 0x7f) == beep_play_idx) || beep_play_idx >= BEEP_BUF_SIZE) {
				// nope, no more
				beep_buf_idx = 0;
				beep_play_idx = 0;
			}
		}
	}
}

/* beep control */
void beep_clear_queue()
{
	beep_buf_idx = 0;
	GPIO_ResetBits(beep_pin.port, beep_pin.pin);
}

uint8_t beep(uint16_t tone_id, uint16_t duration)
{
	// TIM_SetAutoreload(BEEP_TIM, (uint16_t)((4000000 / freq) - 1));
	if ((beep_buf_idx & 0x7f) < BEEP_BUF_SIZE) {
		// if not in use, reset the playback index
		if (beep_buf_idx == 0) {
			beep_play_idx = 0;
		}

		// mark the beeper as being used
		beep_buf_idx |= 0x80;

		// set the desired tone and duration
		beep_lev_buf[beep_buf_idx & 0x7f] = (tone_id < 32) ? 32 - tone_id : 0;
		beep_dur_buf[beep_buf_idx & 0x7f] = duration * 5;

		// prepare for the next buffer location
		beep_buf_idx++;

		return (beep_buf_idx & 0x7f);
	}

	return 0xff;
}

/*
void beep_song(uint16_t song_idx, uint16_t spacing)
{
	active_song = song_idx;
	beep_spacing = 0;
	beep_idx = 0;
	beep_dur = 0;
	beep_space = 0;
}
*/


/* beep isr */
/*
void TIM7_IRQHandler()
{
	static uint8_t flip = 0;

	if (flip) {
		GPIO_ResetBits(beep_pin.port, beep_pin.pin);
	} else {
		GPIO_SetBits(beep_pin.port, beep_pin.pin);
	}

	flip = ~flip;

	// clear interrupt flag
	TIM_ClearITPendingBit(BEEP_TIM, TIM_IT_Update);
}
*/
