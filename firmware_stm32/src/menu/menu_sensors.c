/*
 * menu_sensors.c: sensors menu
 * 2014 by true
 *
 * ----
 *
 * $Id: menu_sensors.c 221 2014-08-07 16:55:36Z true $
 */


#include "menu_sensors.h"

  // menus
MenuItem *menuc; 			// lcd_menu.h

  // adc
uint16_t adc_result[ADC_MAX_RESULT_COUNT];

  // others
uint8_t temperature; 		// pirate.h
int8_t temperature_cal;
uint8_t light_level;
uint8_t light_gain;
uint16_t mic_peak;


/* menu functions */
  // feedback display
char * menu_sens_disp(uint16_t id)
{
	switch (id) {
		case SENSOR_LIGHT_CURRENTGAIN: {
			return pirate_sitoa(light_gain, 10, 3);
		}
		case SENSOR_LIGHT_SETTINGSGAIN: {
			return pirate_sitoa(settings.light_setgain, 10, 3);
		}
		case SENSOR_LIGHT_LEVEL: {
			return pirate_sitoa(light_level, 10, 3);
		}

		case SENSOR_MIC_CAL_LOW:
		case SENSOR_MIC_CAL_HIGH: {
			if (menu_editing == SENSOR_MIC_CAL_LOW) {
				if (settings.mic_cal[0] > mic_peak) {
					settings.mic_cal[0] = mic_peak - (mic_peak >> 6);
				}
			} else if (menu_editing == SENSOR_MIC_CAL_HIGH) {
				if (settings.mic_cal[1] < mic_peak) {
					settings.mic_cal[1] = mic_peak + (mic_peak >> 6);
				}
			}

			return pirate_sitoa(settings.mic_cal[id - SENSOR_MIC_CAL_LOW], 10, 4);
		}

		case SENSOR_TEMP_CALVALUE: {
			return pirate_sitoa(temperature_cal, 10, 3);
		}

		default: {
			return "";
		}
	}
}

char * menu_sensor_batt_disp(uint16_t id)
{
	int i;
	static char volt[6];
	uint16_t raw;

	// get voltage, clear target array
	raw = pirate_batt_voltage();
	for (i = 0; i < 6; i++) {
		volt[i] = 0;
	}

	// voltage whole number
	strcpy(volt, pirate_sitoa(raw / 100, 10, 0));

	// voltage decimal start
	if (raw % 100 <= 9) {
		strncat(volt, ".0", 6);
	} else {
		strncat(volt, ".", 6);
	}

	// voltage decimal
	strncat(volt, pirate_sitoa(raw % 100, 10, 0), 6);

	// voltage postfix
	volt[4] = 'V';
	volt[5] = 0;

	return volt;
}
char * menu_sensor_temp_disp(uint16_t id)
{
	static char temp[7];
	uint16_t raw;

	raw = pirate_thermometer(id);

	if (!id) {
		strcpy(temp, pirate_sitoa(raw / 10, 10, 0));
		strncat(temp, ".", 7);
		strncat(temp, pirate_sitoa(raw % 10, 10, 0), 7);
		strncat(temp, "C", 7);
		temp[6] = 0;
	} else {
		strcpy(temp, pirate_sitoa(raw, 10, 0));
		strncat(temp, "F", 5);
		temp[4] = 0;
	}

	return temp;
}

char * menu_sensor_mic_disp(uint16_t id)
{
	uint16_t w;

	if (!id) {
		return pirate_sitoa(mic_peak, 10, 4);
	} else {
		w = (settings.mic_cal[1] - settings.mic_cal[0]);
		return pirate_sitoa(((mic_peak - settings.mic_cal[0]) * 100) / w, 10, 4);
	}
}


  // menu actions
void menu_sensor_mic_cal_start(uint16_t id)
{
	if (menu_editing) {
		menu_editing = 0;
	} else {
		menu_editing = id;
		settings.mic_cal[id - SENSOR_MIC_CAL_LOW] = mic_peak;
	}

	menu_btntype_editing();
}


/* menu construction */
  // forward declaration
const MenuItem menu_sens_viewitem[];

const MenuItem menu_sens_setitem[];
const MenuItem menu_sens_set_lightitem[];
const MenuItem menu_sens_set_micitem[];


  // main menu
const MenuData menu_sens_rdata[] = {
	{"View",  NULL, 0, 0, LCD_DEF_SPACING, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
	{"Setup", NULL, 0, 0, LCD_DEF_SPACING, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0}
};
const MenuItem menu_sens_ritem[] = {
	{&menu_ritem[LCD_MENU_ROOT_SENSORS],                   0, &menu_sens_ritem[1], &menu_sens_viewitem[0], &menu_sens_rdata[0]},
	{&menu_ritem[LCD_MENU_ROOT_SENSORS], &menu_sens_ritem[0],                   0,  &menu_sens_setitem[0], &menu_sens_rdata[1]},
};


  // "view" menu
const MenuData menu_sens_viewdata[] = {
	{"Bat",   menu_sensor_batt_disp,                      0, 20, 4, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
	{"Tmp",   menu_sensor_temp_disp,                    0, 20, 4, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
	{"Tmp ",  menu_sensor_temp_disp,                    1, 20, 4, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
	{"MicR",  menu_sensor_mic_disp,                       0, 12, 4, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
	{"Mic%",  menu_sensor_mic_disp,                       1, 12, 4, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
	{"LitGn", menu_sens_disp, SENSOR_LIGHT_CURRENTGAIN, 12, 4, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
	{"LitLv", menu_sens_disp,       SENSOR_LIGHT_LEVEL, 12, 4, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
};
const MenuItem menu_sens_viewitem[] = {
	{&menu_sens_ritem[0],                      0, &menu_sens_viewitem[1], 0, &menu_sens_viewdata[0]},
	{&menu_sens_ritem[0], &menu_sens_viewitem[0], &menu_sens_viewitem[2], 0, &menu_sens_viewdata[1]},
	{&menu_sens_ritem[0], &menu_sens_viewitem[1], &menu_sens_viewitem[3], 0, &menu_sens_viewdata[2]},
	{&menu_sens_ritem[0], &menu_sens_viewitem[2], &menu_sens_viewitem[4], 0, &menu_sens_viewdata[3]},
	{&menu_sens_ritem[0], &menu_sens_viewitem[3], &menu_sens_viewitem[5], 0, &menu_sens_viewdata[4]},
	{&menu_sens_ritem[0], &menu_sens_viewitem[4], &menu_sens_viewitem[6], 0, &menu_sens_viewdata[5]},
	{&menu_sens_ritem[0], &menu_sens_viewitem[5],                      0, 0, &menu_sens_viewdata[6]},
};

  // "setup" menu and submenus
const MenuData menu_sens_setdata[] = {
	{"LightSensor", NULL, 0, 0,
			LCD_DEF_SPACING, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
	{"Mic", NULL, 0, 0, LCD_DEF_SPACING, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, NULL, 0},
	{"T-Cal", menu_sens_disp, SENSOR_TEMP_CALVALUE, 0,
			LCD_DEF_SPACING, LCD_DEF_SCROLL_WAIT, LCD_DEF_SCROLL_RUN, menu_edit_start, SENSOR_TEMP_CALVALUE},
};
const MenuItem menu_sens_setitem[] = {
	{&menu_sens_ritem[1],                     0, &menu_sens_setitem[1], &menu_sens_set_lightitem[0], &menu_sens_setdata[0]},
	{&menu_sens_ritem[1], &menu_sens_setitem[0], &menu_sens_setitem[2],   &menu_sens_set_micitem[0], &menu_sens_setdata[1]},
	{&menu_sens_ritem[1], &menu_sens_setitem[1],                     0,                           0, &menu_sens_setdata[2]},
};

  // setup/lightsensor
const MenuData menu_sens_set_lightdata[] = {
	{"Gain ", menu_sens_disp,  SENSOR_LIGHT_CURRENTGAIN, 4, 0, 0, 0,            NULL, 0},
	{"Fixed", menu_sens_disp, SENSOR_LIGHT_SETTINGSGAIN, 4, 0, 0, 0, menu_edit_start, SENSOR_LIGHT_SETTINGSGAIN},
};
const MenuItem menu_sens_set_lightitem[] = {
	{&menu_sens_setitem[0],                           0, &menu_sens_set_lightitem[1], 0, &menu_sens_set_lightdata[0]},
	{&menu_sens_setitem[0], &menu_sens_set_lightitem[0], &menu_sens_set_lightitem[2], 0, &menu_sens_set_lightdata[1]},
	{&menu_sens_setitem[0], &menu_sens_set_lightitem[1],                           0, 0, &menu_sens_set_lightdata[2]},
};

  // setup/mic
const MenuData menu_sens_set_micdata[] = {
	{"CLow", menu_sens_disp,  SENSOR_MIC_CAL_LOW, 1, 0, 0, 0, menu_sensor_mic_cal_start,  SENSOR_MIC_CAL_LOW},
	{"CHi ", menu_sens_disp, SENSOR_MIC_CAL_HIGH, 1, 0, 0, 0, menu_sensor_mic_cal_start, SENSOR_MIC_CAL_HIGH},
};
const MenuItem menu_sens_set_micitem[] = {
	{&menu_sens_setitem[1],                         0, &menu_sens_set_micitem[1], 0, &menu_sens_set_micdata[0]},
	{&menu_sens_setitem[1], &menu_sens_set_micitem[0],                         0, 0, &menu_sens_set_micdata[1]},
};
