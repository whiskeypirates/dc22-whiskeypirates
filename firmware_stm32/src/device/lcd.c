/*
 * lcd.c: lcd menuing and support functions
 * 2014 by true
 *
 * ----
 *
 * $Id: lcd.c 215 2014-08-04 06:28:50Z true $
 */


#include "lcd.h"

#include "../interface/i2c.h"
#include "../interface/gpio.h"


  // global settings
PirateSettings settings; 	// pirate.h

  // led backlight
static uint16_t lcd_led_level = 0;
static uint16_t lcd_led_level_set = 0;
static uint8_t lcd_led_rate;
static uint8_t lcd_led_upd;

  // lcd control
static uint8_t lcd_height;
static uint8_t lcd_cursor_pos;
static uint8_t lcd_cursor_type;
uint8_t (*lcd_cgram)[8];
uint8_t lcd_cgram_len;

  // lcd hardware
static const tGPIO lcd_led_pin = {GPIOA, GPIO_Pin_6, 6};


/* functions */
void lcd_init()
{
	static const uint8_t cmd_init[] = {
		0x38, 	// set to i2c
		0x39, 	// set to i2c
		0x1d, 	// 1/4 bias, osc ~220hz
		0x78, 	// contrast low 4 bits
		0x5d, 	// icon enable, voltage boost enable, contrast high 2 bits
		0x6d, 	// internal follower enable,
		0x0c, 	// display on
		0x01, 	// clear display
		0x06 	//
	};

	uint8_t *cmdptr = (uint8_t *)cmd_init;

	I2C_WriteTransfer(LCD_I2C_ADDR, cmdptr, 9, LCD_CMD, 1);

	// set stored contrast
	lcd_set_contrast(settings.contrast);
}

void lcd_led_init()
{
	TIM_TimeBaseInitTypeDef tim_tb;
	TIM_OCInitTypeDef tim_oc;
	GPIO_InitTypeDef gpio;

	// enable clocks
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);

	// set up with 250 levels of brightness, with a bit of empty
	tim_tb.TIM_Prescaler = 5 - 1; 		// 6.4MHz
	tim_tb.TIM_Period = 320 - 1; 		// 25KHz PWM
	tim_tb.TIM_ClockDivision = 0;
	tim_tb.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(LCD_LED_TIMER, &tim_tb);

	// configure outputs as pwm
	tim_oc.TIM_OCMode = TIM_OCMode_PWM1;
	tim_oc.TIM_OutputState = TIM_OutputState_Enable;
	tim_oc.TIM_Pulse = 0;
	tim_oc.TIM_OCPolarity = LCD_LED_OC_POLARITY;
	TIM_OC1Init(LCD_LED_TIMER, &tim_oc);
	TIM_OC1PreloadConfig(LCD_LED_TIMER, TIM_OCPreload_Enable);

	TIM_ARRPreloadConfig(LCD_LED_TIMER, ENABLE);
	TIM_Cmd(LCD_LED_TIMER, ENABLE);

	// configure output pin as timer AF
	GPIO_StructInit(&gpio);
	gpio.GPIO_Mode = GPIO_Mode_AF;
	gpio.GPIO_Speed = GPIO_Speed_2MHz;
	gpio.GPIO_Pin = lcd_led_pin.pin;
	GPIO_Init(lcd_led_pin.port, &gpio);

	GPIO_PinAFConfig(lcd_led_pin.port, lcd_led_pin.pinsource, GPIO_AF_TIM10);
}

void lcd_print(uint8_t pos, uint8_t *msg, uint8_t msg_len)
{
	uint8_t len;

	// which line/pos to print to?
	I2C_WriteTransfer(LCD_I2C_ADDR, &pos, 1, LCD_CMD, 1);

	// length to print
	len = msg_len < LCD_MAX_LINE_LENGTH ? msg_len : LCD_MAX_LINE_LENGTH;

	// send it
	I2C_WriteTransfer(LCD_I2C_ADDR, msg, len, LCD_DATA, 1);
}

void lcd_linebuf_send()
{
	lcd_print(LCD_LINE_1, (uint8_t *)lcd_line[0], 8);

	if (lcd_get_height() != LCD_CMD_DOUBLEHEIGHT) {
		lcd_print(LCD_LINE_2, (uint8_t *)lcd_line[1], 8);
	}
}


void lcd_cmd(uint8_t cmd)
{
	I2C_WriteTransfer(LCD_I2C_ADDR, &cmd, 1, LCD_CMD, 1);
	// pirate_delay(1);
}

void lcd_set_contrast(uint8_t level) {
	settings.contrast = level & 0x3f;
}

void lcd_apply_contrast()
{
	// contrast is 6 bits; can be between 0 and 63
	uint8_t cmd[2] = {0};

	// form command
	cmd[0] = 0x5c | (settings.contrast >> 4);
	cmd[1] = 0x70 | (settings.contrast & 0x0f);

	I2C_WriteTransfer(LCD_I2C_ADDR, cmd, 2, LCD_CMD, 1);
}

void lcd_set_height(uint8_t height)
{
	if (height)
		lcd_height = height;
}

uint8_t lcd_get_height()
{
	return lcd_height;
}

void lcd_set_cursor(uint8_t pos, uint8_t type)
{
	if (pos)
		lcd_cursor_pos = pos;

	if (type)
		lcd_cursor_type = type;
}

uint8_t lcd_get_cursor_pos()
{
	return lcd_cursor_pos;
}

uint8_t lcd_get_cursor_type()
{
	return lcd_cursor_type;
}

void lcd_set_cgram_load(const uint8_t (*data)[8], const uint8_t len)
{
	lcd_cgram = (uint8_t (*)[8])data;
	lcd_cgram_len = (uint8_t)len;
}

void lcd_led_set_level(uint8_t level, uint8_t ramp_speed)
{
	lcd_led_level_set = level;
	lcd_led_rate = ramp_speed;
	lcd_led_upd = LCD_LED_UPD_RATE;

	// force an update now
	lcd_led_update();
}

void lcd_led_update()
{
	if (lcd_led_upd) {
		lcd_led_upd--;
		if (!lcd_led_upd) {
			// are we done?
			if (lcd_led_level == lcd_led_level_set) {
				return;
			}
			// nope, check and see if we are increasing level
			if (lcd_led_level_set > lcd_led_level) {
				lcd_led_level += lcd_led_rate;
				// level higher than target? then force to target
				if (lcd_led_level > lcd_led_level_set) {
					lcd_led_level = lcd_led_level_set;
				}
			}
			// nope, check and see if we are decreasing level
			if (lcd_led_level_set < lcd_led_level) {
				// rate lower than target? if so, force to target
				if ((lcd_led_level - lcd_led_level_set) > lcd_led_rate) {
					lcd_led_level = lcd_led_level_set;
				} else {
					lcd_led_level -= lcd_led_rate;
				}
			}

			// set OC
			LCD_LED_TIMER->CCR1 = lcd_led_level;

			// and reset countdown timer
			lcd_led_upd = LCD_LED_UPD_RATE;
		}
	}
}
