/*
 * menu_sensors.h: sensors menu
 * 2014 by true
 *
 * ----
 *
 * $Id$
 */


#ifndef __PIRATE_MENU_SENSORS_H
#define __PIRATE_MENU_SENSORS_H


/* menus */
#include "../pirate.h"
#include "lcd_menu.h"

/* sensor items */
#include "../interface/adc.h"


/* menus */
extern const MenuItem menu_sens_ritem[];


/* sensors defines */
#define SENSOR_LIGHT_CURRENTGAIN 		0x0400
#define SENSOR_LIGHT_SETTINGSGAIN 		0x0401

#define SENSOR_LIGHT_LEVEL 				0x0404

#define SENSOR_MIC_CAL_LOW 				0x0410
#define SENSOR_MIC_CAL_HIGH 			0x0411

#define SENSOR_TEMP_CALVALUE 			0x0420


#endif
