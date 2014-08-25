/**
 * Whiskey Pirates DC22 Badge "PIRATE CPU" Firmware for STM32L100RBT6
 * Badge Codename "Three Kings"
 * Originally written 2014 by true
 * Creation Date: 2014.05.16
 *
 * ----
 *
 * $Id: main.c 226 2014-08-18 02:03:12Z true $
 * ts:4
 *
 * ----
 *
 * ISR Usage ~13% average, 26% max (as of rev186)
 *
**/


/* pirates */
#include "pirate.h" 				// cmsis include is located here

#include "interface/gpio.h"
#include "interface/led_pwm.h"
#include "interface/adc.h"
#include "interface/i2c.h"

#include "device/lcd.h"
#include "device/beep.h"
#include "device/attiny.h"
#include "device/lightsensor.h"
#include "device/nrf24l01.h"

#include "led/led_matrix.h"
#include "led/led_bone.h"

#include "menu/lcd_menu.h"
#include "menu/menu_settings.h" 	// settings_restore() lives here
#include "menu/menu_program.h" 		// LED program related shit lives here

#include "display/infopirate.h"


  // hard buttons
static const tGPIO btn[4] = {
	{GPIOC, GPIO_Pin_0, 0},
	{GPIOA, GPIO_Pin_15, 15},
	{GPIOC, GPIO_Pin_1, 1},
	{GPIOB, GPIO_Pin_2, 2},
};

#define BTN_DEBOUNCE_COUNT 		30

static const uint8_t btn_high = 0b00001000; 	// button is active high when its bit is set, otherwise active low
uint16_t btn_press[4];
uint16_t btn_hold[4];

  // button callbacks
void (*lcd_btn_fn[4])(); 	// lcd_menu.h

  // menuing and LCD
uint8_t menu_mode; 			// lcd_menu.h
uint16_t menu_editing; 		// lcd_menu.h
const MenuItem menu_runitem;// lcd_menu.h

uint8_t (*lcd_cgram)[8]; 	// lcd.h
uint8_t lcd_cgram_len; 		// lcd.h

  // misc extras
uint8_t temperature; 		// pirate.h
uint8_t light_level; 		// pirate.h
uint8_t light_gain; 		// pirate.h
uint16_t mic_peak; 			// pirate.h

  // adc
uint16_t adc_result[ADC_MAX_RESULT_COUNT];

  // LED programs
uint8_t prog_id[3]; 		// menu_program.h
uint16_t prog_dwell[3]; 	// menu_program.h

  // mainline timer timekeeping
uint16_t tim6_count;
uint32_t tim6_usage; 		// profiling


/* functions */
/********
 * initialize mainline interrupt
 ********/
void pirate_tim6_init()
{
	TIM_TimeBaseInitTypeDef tim_tb;
	NVIC_InitTypeDef nvic;

	// enable peripheral clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	// set up to fire 0.2ms mainline loop (~6400 clocks per cycle)
	// LED light rate for main LED matrix is 312.5Hz
	TIM_TimeBaseStructInit(&tim_tb);
	tim_tb.TIM_Period = (SystemCoreClock / 5000) - 1;
	TIM_TimeBaseInit(TIM6, &tim_tb);

	// enable interrupt
	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);

	// set as highest priority - this is our UI
	nvic.NVIC_IRQChannel = TIM6_IRQn;
	nvic.NVIC_IRQChannelPreemptionPriority = 2;
	nvic.NVIC_IRQChannelSubPriority = 1;
	nvic.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&nvic);

	// finally, enable the timer
	TIM_Cmd(TIM6, ENABLE);

	// make sure our profiling variable doesn't get optimized away
	__asm__ __volatile__("" :: "m" (tim6_usage));
}


/********
 * initialize gpio for buttons
 * this assumes the GPIOs are clocked already
 ********/
static inline void btn_init()
{
	GPIO_InitTypeDef gpio;
	int i;

	gpio.GPIO_Mode = GPIO_Mode_IN;
	gpio.GPIO_Speed = GPIO_Speed_400KHz;

	for (i = 0; i < 4; i++) {
		if (btn_high & (1 << i)) {
			gpio.GPIO_PuPd = GPIO_PuPd_NOPULL;
		} else {
			gpio.GPIO_PuPd = GPIO_PuPd_UP;
		}

		gpio.GPIO_Pin = btn[i].pin;
		GPIO_Init(btn[i].port, &gpio);
	}

	// set initial button types to operate the main menu
	// TODO: maybe this won't do this and it'll instead run the program by default
	menu_btntype_menus();
}


/********
 * default button push handlers
 * TODO: implement what is necessary to make these more useful
 ********/
static inline void btn_evt_push(uint16_t btn)
{
	if (lcd_btn_fn[btn] != 0) {
		lcd_btn_fn[btn]();
	}
}

static inline void btn_evt_hold(uint16_t btn, uint16_t count)
{
	if (btn == BTN_UP || btn == BTN_DOWN) {
		if (!(count == 0 || count == 2)) {
			btn_evt_push(btn);
		}
	}
}

static inline void btn_evt_release(uint16_t btn)
{
	return;
}

/********
 * update the state of pressed buttons
 ********/
static inline void btn_update()
{
	uint8_t i;
	uint8_t pressed;

	for (i = 0; i < 4; i++) {
		pressed = 0;

		// button high or low?
		if (btn_high & (1 << i)) {
			if (btn[i].pin & GPIO_ReadInputData(btn[i].port)) pressed = 1;
		} else {
			if (!(btn[i].pin & GPIO_ReadInputData(btn[i].port))) pressed = 1;
		}

		if (pressed) {
			btn_press[i]++;
			if (btn_press[i] == BTN_DEBOUNCE_COUNT) {
				// button is pushed
				btn_evt_push(i);
			} else if (btn_press[i] == 1500) {
				// btn is held
				btn_evt_hold(i, btn_hold[i]);

				// btn is held for 0.3s (or re-fired 0.1s)
				btn_hold[i]++;
				// we only uniquely count 30 seconds of hold...
				if (btn_hold[i] > 30000) {
					btn_hold[i] = 30000;
				}
				// reset to re-fire in 0.1s increments
				btn_press[i] = 1000;
			}

			// done handling buttons - we only do one pressed at a time right now
			break;
		} else {
			// if pushed, fire release event
			if (btn_press[i] > BTN_DEBOUNCE_COUNT) {
				btn_evt_release(i);
			}
			// reset pushed state
			btn_press[i] = 0;
			btn_hold[i] = 0;
		}
	}
}


/* mainline */
int main(void)
{
	int i;

	// update clock info
	SystemCoreClockUpdate();

	// set up LED matrix GPIO and clocks
	led_matrix_io_init();
	// set up bone LED GPIO and clocks
	led_bone_io_init();

	// set up LED PWM peripherals
	led_pwm_init_all();

	// set up buttons
	btn_init();

	// read settings from EEPROM
	// NOTE: MENU button will load default settings (but keep name)
	// NOTE: MENU+OK will also erase name, restoring all defaults
	// TODO: separate programs from this, have another combo for erasing programs
	i = (GPIO_ReadInputDataBit(btn[BTN_MENU].port, btn[BTN_MENU].pin)) ? 0x00 : 0x01;
	if (i) i |= (GPIO_ReadInputDataBit(btn[BTN_OK].port, btn[BTN_OK].pin)) ? 0x00 : 0x02;

	settings_restore(i);

	// set up program run variables
	prog_init();

	// set up ADC
	adc_init();

	// set up magical beeper
	beep_init();

	// and play startup tone
	beep(26, 40);
	beep(31, 40);
	beep(30, 40);
	beep(29, 40);
	beep(28, 40);

	// set up I2C master peripheral
	i2c_init(I2C2, 400000);

	// wait some more time for pegleg to boot. it can take a while
	pirate_delay(20);

	// set up LCD
	lcd_init();
	lcd_cmd(LCD_CMD_CLEAR_SCREEN);

	// set up LCD menus
	lcd_menu_init();

	// set up LCD backlight
	lcd_led_init();

	// set up pegleg light sensor gain
	if (settings.light_setgain) {
		// we have a fixed gain set
		light_gain = settings.light_setgain;
		attiny_write_light_sensitivity(0, light_gain);
	} else {
		// we don't have a fixed gain, so load a default gain value
		light_gain = LIGHTSENS_THRESH_TOP_NORM;
		attiny_write_light_sensitivity(0, LIGHTSENS_THRESH_TOP_NORM);
	}

	// set up mainline interrupt
	pirate_tim6_init();

	// after system is booted, we can enable the prefetch buffer
	// for some reason, at 32MHz, power glitches could cause some
	// freak out when booting if we turned this on any earlier
	FLASH->ACR |= FLASH_ACR_PRFTEN;

    while (1) {
    	// update clock info
    	SystemCoreClockUpdate();

    	// update various devices
    	if (tim6_count % 500 == 0) {
    		static uint8_t update = 0;

    		update++;
    		if (update > 10) update = 1;

    		switch (update) {
    			case 1: { 	 // light sensor
					// TODO: investigate the bug that causes this to read ~70-80 when it
    				//       should be reading >100
    				if ((settings.led_autoadjust & 0x80) || settings.lcd_autobrite) {
						if (!settings.light_setgain) {
							light_level = attiny_read_light_level(0);
						}
					} else {
						light_level = settings.led_autogain_lev_max - 1;
					}
					break;
				}

    			case 2: {
					// auto-update light sensor gain
					if (!settings.light_setgain) {
						if (lightsensor_gainvalue_update()) {
							// updated, so send updated value
							attiny_write_light_sensitivity(0, light_gain);
						}
					} else {
						light_gain = settings.light_setgain;
					}
					break;
				}

    			case 3:
				case 8: { 		// temp sensor
					pirate_thermometer_log(attiny_read_temp());
					break;
				}
    		}
    	}

    	// max update 100 times/second
    	if (tim6_count % 50 == 8) {
			// update eyes
			led_eyes_tx();

			// update LCD contrast
			lcd_apply_contrast();

			// update LCD CGRAM
			if (lcd_cgram_len) {
				// set LCD height
				// make sure in your code that you set this to CGRAM mode!
				lcd_cmd(lcd_get_height());

				// only update CGRAM if we are in CGRAM mode
				if ((lcd_get_height() & 0x01) == 0) {
					// make sure to force updating the mode again
					lcd_cmd(lcd_get_height());
					// and update CGRAM contents
					for (i = 0; i < lcd_cgram_len; i++) {
						lcd_print(0x40 + (i << 3), lcd_cgram[i], 8);
					}
					lcd_cgram_len = 0;
				}
			} else {
				// set LCD height
				// this is here like this to fix compiler optimization bug?
				lcd_cmd(lcd_get_height());
			}

			// update LCD characters
			lcd_linebuf_send();

			// set the LCD cursor position and type
			lcd_cmd(lcd_get_cursor_type());
			lcd_cmd(lcd_get_cursor_pos());
    	}

    	// update battery voltage info 10 times/second
    	if (tim6_count % 500 == 499) {
    		pirate_batt_log(adc_result[ADC_READ_BATT_VOLTAGE]);
    	}

    	// then go to sleep
    	__WFI();
    }

    return(0);
}


/* ISR */
// mainline interrupt
void TIM6_IRQHandler()
{
	int i;
	int j;
	int k;
	int n;

	// update counter
	tim6_count++;
	if (tim6_count >= 5000) {
		tim6_count = 0;
	}

	// update pirate prng
	// actually this is handled by whatever needs it...
	// pirate_prng();

	// manage buttons
	btn_update();

	// update ADC values, currently 2.5KHz
	if ((tim6_count & 0x01) == 0x01) {
		adc_start();
	}

	// update LED mic peak
	// mic peak decaying
	if ((tim6_count & 0x03) == 0x03) {
		if (mic_peak > adc_result[ADC_READ_MIC_PEAK]) {
			mic_peak--;
		} else {
			mic_peak = adc_result[ADC_READ_MIC_PEAK];
		}
	}

	// update led autoadjust value for LEDs, polls 25 times/second (40ms)
	if (tim6_count % 200 == 0) {
		// always need to run this as the LCD might use it
		lightsensor_scaler_update();

		// backlight LCD is a little different
		if (settings.lcd_autobrite) {
			lcd_led_set_level(lightsensor_get_scalerval(LIGHTSENS_SCALED_BACKLIGHT), 1);
		} else {
			lcd_led_set_level(settings.lcd_brightness, 1);
		}
	}


	// update main matrix LEDs
	led_matrix_mode_update();
	led_matrix_next();

	// update bone matrix LEDs
	led_bone_mode_update();
	led_bone_next();

	// update eyes RGBLEDs
	led_eyes_mode_update();

	// update beeper
	beep_update();

	// update LCD backlight
	lcd_led_update();

	// update LCD data 100Hz
	// yes, we only update it at 50Hz. but timings use 100Hz timings.
	if (tim6_count % 50 == 0) {
		if (menuc == (MenuItem *)&menu_runitem) {
			// run mode display
			infopirate_update();
		} else {
			// info display
			lcd_menu_update();
		}
	}

	// runmode program
	if (tim6_count == 0) {
		for (i = 0; i < 3; i++) {
			if (!((menu_mode == MENU_MODE_RUNNING_PROGRAM) || (menu_mode == MENU_MODE_PROGRAM_EDITOR) || settings.autorun & 0x02)) {
				prog_id[i] = 255;
				switch (i) {
					case PROG_TYPE_SKULL: {
						led_matrix_set_mode(LED_MATRIX_MODE_OFF);
						break;
					}
					case PROG_TYPE_BONES: {
						led_bone_set_mode(LED_MATRIX_MODE_OFF);
						break;
					}
					case PROG_TYPE_EYES: {
						// only stop eyes if not in favcolor editor
						if (!(menuc->root == &menu_pref_ritem[1])) {
							led_eyes_set_mode(LED_EYES_MODE_OFF);
						}
					}
				}
			} else {
				if (prog_dwell[i] > 0) {
					prog_dwell[i]--;
				}
				if (prog_dwell[i] == 0) {
					if (menu_mode == MENU_MODE_PROGRAM_EDITOR) {
						n = menu_prog_get_type();
						j = menu_prog_get_idx();
						k = (prog_id[n] == 255 || (settings.led_prog[n][j].type & 0x02)) ? 1 : 0;

						prog_id[i] = 0;
						prog_dwell[i] = k ? settings.led_prog[n][j].dwell : k;

						switch (n) {
							case PROG_TYPE_SKULL: {
								led_matrix_set_program(
										settings.led_prog[0][j].progidx,
										k,
										settings.led_prog[0][j].wait,
										settings.led_prog[0][j].level,
										settings.led_prog[0][j].offset,
										settings.led_prog[0][j].option
								);
								break;
							}
							case PROG_TYPE_BONES: {
								led_bone_set_program(
										settings.led_prog[1][j].progidx,
										k,
										settings.led_prog[1][j].wait,
										settings.led_prog[1][j].level,
										settings.led_prog[1][j].offset,
										settings.led_prog[1][j].option
								);
								break;
							}
							case PROG_TYPE_EYES: {
								led_eyes_set_program(
										settings.led_prog[2][j].progidx,
										k,
										settings.led_prog[2][j].wait,
										settings.led_prog[2][j].level,
										settings.led_prog[2][j].offset,
										settings.led_prog[2][j].option
								);
								break;
							}
						}
					} else {
						j = PIRATE_PROG_SAVED_MAX + 1;
						k = prog_id[i];
						while (j) {
							j--;
							prog_id[i]++;
							if (prog_id[i] >= PIRATE_PROG_SAVED_MAX) prog_id[i] = 0;

							if (settings.led_prog[i][prog_id[i]].type & 0x80) {
								// we have an enabled program
								break;
							}
						}

						if (j) {
							// load new program if it doesn't match the old program
							// or if a program was never run (such as at reboot)
							if (prog_get_mode(i) == 1) {
								k = (k == 255 || (settings.led_prog[i][prog_id[i]].type & 0x02)) ? 1 : 0;
								// program change check: (settings.led_prog[i][prog_id[i]].progidx != k)

								prog_dwell[i] = settings.led_prog[i][prog_id[i]].dwell;

								switch (i) {
									case PROG_TYPE_SKULL: {
										led_matrix_set_program(
												settings.led_prog[0][prog_id[0]].progidx,
												k,
												settings.led_prog[0][prog_id[0]].wait,
												settings.led_prog[0][prog_id[0]].level,
												settings.led_prog[0][prog_id[0]].offset,
												settings.led_prog[0][prog_id[0]].option
										);
										break;
									}
									case PROG_TYPE_BONES: {
										led_bone_set_program(
												settings.led_prog[1][prog_id[1]].progidx,
												k,
												settings.led_prog[1][prog_id[1]].wait,
												settings.led_prog[1][prog_id[1]].level,
												settings.led_prog[1][prog_id[1]].offset,
												settings.led_prog[1][prog_id[1]].option
										);
										break;
									}
									case PROG_TYPE_EYES: {
										led_eyes_set_program(
												settings.led_prog[2][prog_id[2]].progidx,
												k,
												settings.led_prog[2][prog_id[2]].wait,
												settings.led_prog[2][prog_id[2]].level,
												settings.led_prog[2][prog_id[2]].offset,
												settings.led_prog[2][prog_id[2]].option
										);
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}

	// profiling: get current counter value
	tim6_usage = tim6_usage;
	tim6_usage = TIM6->CNT;

	// clear interrupt bit
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
}
