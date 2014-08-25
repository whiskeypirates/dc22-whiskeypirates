/*
 * lcd.c: lcd menuing and support function prototypes
 * 2014 by true
 *
 * ----
 *
 * $Id: lcd.h 214 2014-08-04 05:31:33Z true $
 */


#ifndef __PIRATE_DEV_LCD_H
#define __PIRATE_DEV_LCD_H


#include <string.h>

#include "pirate.h"


#define LCD_I2C_DEV 				I2C1
#define LCD_I2C_ADDR 				0x3e

#define LCD_MAX_LINE_LENGTH 		8

#define LCD_LED_TIMER 				TIM10
#define LCD_LED_OC_POLARITY 		TIM_OCPolarity_High
#define LCD_LED_UPD_RATE 			100 	// updates every 5000/100 = 50ms update rate

#define LCD_LINE_1 					0x80
#define LCD_LINE_2 					0xc0

#define LCD_CMD 					0x00
#define LCD_DATA 					0x40

#define LCD_CMD_CLEAR_SCREEN 		0x01
#define LCD_CMD_SINGLEHEIGHT 		0x39
#define LCD_CMD_DOUBLEHEIGHT 		0x35
#define LCD_CMD_NO_CURSOR_FLASH		0x0c
#define LCD_CMD_CURSOR_FLASH 		0x0d
#define LCD_CMD_CURSOR_UNDER 		0x0e
#define LCD_CMD_CURSOR_UNDER_FLASH 	0x0f


/* variables */
extern char lcd_line[2][9];

extern uint8_t (*lcd_cgram)[8];
extern uint8_t lcd_cgram_len;


/* prototypes */
void lcd_init();
void lcd_led_init();

void lcd_cmd(uint8_t command);
void lcd_print(uint8_t pos, uint8_t *msg, uint8_t msg_len);
void lcd_linebuf_send();

void lcd_set_contrast(uint8_t level);
void lcd_apply_contrast();

void lcd_set_height(uint8_t height);
uint8_t lcd_get_height();

void lcd_set_cursor(uint8_t pos, uint8_t type);
uint8_t lcd_get_cursor_pos();
uint8_t lcd_get_cursor_type();

void lcd_set_cgram_load(const uint8_t (*data)[8], const uint8_t len);

void lcd_led_set_level(uint8_t level, uint8_t ramp_speed);
void lcd_led_update();


#endif
