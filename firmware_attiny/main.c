/*
 * Whiskey Pirates Badge "PEGLEG CPU" Sub MCU Firmware for ATtiny88
 * Badge Codename "Three Kings"
 * Created: 5/21/2014 9:24:48 PM
 * Author: true
 *
 * $Id: main.c 226 2014-08-18 02:03:12Z true $
 * ts:4
 *
 ********
 * Use the following fuses for this project. Needs to be 8MHz for 400khz I2C operation.
 * 8MHz, self-program enabled (bootloader support), preserve EEPROM, BOD disabled, ISP enabled
 * -U lfuse:w:0xEE:m -U hfuse:w:0xD7:m -U efuse:w:0x00:m
 *       (efuse will likely show as 0x06 after setting; this is fine, don't worry about it)
 *
 * Resources used: Mainline ISR 5% (3/63 timer counts simulated, as of some old rev)
 *                 Flash 26.8%, SRAM 10.4% (as of the above rev)
 **********
 * Register Variables:
 *		main.c:comm_cmd:r15, adc.h:adc_read_step:r14, led.c:rgbled_idx:r13
 */


#include "config.h"

/* system */
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <util/delay.h>


/* modes (commands) */
#define	MODE_NONE					0b0000

#define	MODE_EXT_CMD				0b0001		// bit 4 unset = extra parameters needed, up to 6 commands
#define	MODE_LED_SET_LEVEL			0b0010 		// (0x00 reserved, 0x1? extra, leaving 0x2? through 0x7?)
#define MODE_TEMPSENSOR_CAL			0b0011
#define	MODE_EEPROM_READ			0b0100
#define	MODE_EEPROM_WRITE			0b0101

#define	MODE_TEMPSENSOR_READ		0b1000		// bit 4 set = immediate, no extra parameters, process now
#define	MODE_LIGHTSENSOR_READ		0b1001
#define MODE_SLEEP					0b1111		

#define	MODE_AUX_PIN_GET			0x10		// extended (send 0x10, cmd, data...)
#define	MODE_AUX_PIN_SET			0x11		// ext commands MUST be 0x10 or higher! 0x00-0x0f are RESERVED!
#define MODE_LIGHTSENSOR_SENS 		0x19

/* aux pin mode */
#define	AUX_PIN_COUNT				10
#define	AUX_PIN_OUTPUT				0x01		// sets to output if set, input if cleared
#define	AUX_PIN_PORTSET				0x02		// output: sets high/low; input: sets pullup on/off

/* adc modes */
#define	ADC_MODE_TEMPSENSOR			8
#define	ADC_MODE_LIGHTSENSOR		1


/* pirated */
#include "timer.h"
#include "adc.h"
#include "led.h"


/* prototypes */
static void system_init();
static void system_io_init();
static void pirate_tempsensor_calibrate();


/* globals */
static uint8_t tim0_milli;
static uint8_t tim0_centi;
static uint8_t tim0_profiler;

#define COMM_DATA_SIZE 				4 		// must be power of 2; default value is 4
register uint8_t comm_cmd asm("r15");		// currently processed SPI command
uint8_t comm_data_idx;						// command data write index
uint8_t comm_data[COMM_DATA_SIZE];			// command data register
uint8_t comm_timeout;						// command invalidate timeout


/* core */
static uint8_t pirate_sleep_mode = SLEEP_MODE_IDLE;

/* tempsensor */
static int8_t temperature;
static int16_t temp_offset;

/* eeprom */
#include <avr/eeprom.h>
#define	EEPROM_ADDR_TEMPCAL				62		// 2 bytes

/* adc */
volatile uint8_t adc_busy;
volatile uint16_t adc_result[ADC_MAX_FEEDBACK + 1];

/* led */
uint8_t rgbled_pwm_lf[4];				// pwm value for TIMER1 OCA LED outputs
uint8_t rgbled_pwm_rt[4];				// pwm value for TIMER1 OCB LED outputs

uint8_t rgbled_light_level[4];


/* interfaces */
#include "i2c.h"


/* it begins */
int main(void)
{
	// configure device
	system_init();
	
	// configure IO
	system_io_init();
	
	// configure RGBLEDs
	rgbled_io_init();
	
	// configure i2c communications
	i2c_slave_init(I2C_SLAVE_ADDRESS, I2C_ENABLE_ADDR_ZERO);

	// configure system timer
	timer0_init();
	// configure led pwm timer
	timer1_init();
	
	// initialize adc
	adc_init();
	
	// are we calibrating the temp sensor?
	pirate_tempsensor_calibrate();
	
	// and now we wait.
    while (1) {
		// set sleep mode, then set for idle sleep (CPU off, all peripherals on)
		set_sleep_mode(pirate_sleep_mode);
		pirate_sleep_mode = SLEEP_MODE_IDLE;
		
		// re-enable interrupts and nap.
		sei();
		sleep_mode();
    }
}


/* func */
static void system_init()
{
	// make SURE we are running at 8MHz
	// do this by first enabling clock divider setup mode,
	CLKPR = _BV(CLKPCE);
	// and disabling any divider
	CLKPR = 0;
}

static void system_io_init()
{
	// enable break-before-make outputs on all IO. SAFETY FIRST
	PORTCR |= (_BV(BBMA) | _BV(BBMB) | _BV(BBMC) | _BV(BBMD));
	
	// set ALL pins as inputs except PB0
	DDRA = 0x00;
	DDRB = 0x01;
	DDRC = 0x00;
	DDRD = 0x00;
	
	// and enable pullups (this reduces power consumption)
	// in the case of PB0, set it LOW
	PORTA = 0xff;
	PORTB = 0xfe;
	PORTC = 0xff;
	PORTD = 0xff;
}

static void pirate_tempsensor_process()
{
	int8_t temp_gain;
	int8_t temp_adj;
	
	temp_adj = adc_result[ADC_CHAN_TEMP] - temp_offset;		// our offset adjusted temperature
	temp_gain = temp_adj >> 3;								// our gain adjust amount
	while (temp_gain > 0) {temp_gain--; temp_adj--;};		// correct positive gain
	while (temp_gain < 0) {temp_gain++; temp_adj++;};		// correct negative gain
	
	// save temperature
	temperature = temp_adj;
}

static void pirate_tempsensor_read()
{
	if (adc_read_step == 0) {
		// select temperature channel
		adc_channel(ADC_CHAN_TEMP, ADC_REF_BANDGAP);
	} else if (adc_read_step == 3) {
		// we've waited 1.5ms so the bandgap voltage should have been set.
		// start conversion after bandgap ref change timeout per datasheet
		adc_start(4, 1);
	} else if (adc_read_step > 3) {
		adc_read_step--;
		if (!adc_busy) {
			// revert reference to AVCC...per the datasheet only changes to
			// bandgap should take ~1ms but changes back to AVCC seem to
			// take a while to be accurate as well
			adc_channel(ADC_CHAN_GND, ADC_REF_AVCC);
			adc_read_mode = 0;
			pirate_tempsensor_process();
		}
	}
}

static void pirate_tempsensor_calibrate()
{
	// NOTE: CALIBRATION IS SINGLE-POINT TO 0 DEGREES CELCIUS! ICE YOUR PEGLEG, MOTHERFUCKER!
	// is pin PA2 low? (pin 3 on attiny88 - is pulled up - short to pin 5 / ground somewhere)
	if ((PINA & _BV(PINA2)) == 0) {
		// turn on the ADC, wait for it to warm up
		adc_init();
		_delay_ms(100);
		
		// is the pin still low? not a glitch?
		if ((PINA & _BV(PINA2)) == 0) {
			// enable interrupts
			sei();
			// start reading the temp sensor, stall until adc is done
			adc_read_mode = ADC_MODE_TEMPSENSOR;
			pirate_tempsensor_read();
			while (adc_busy);
			
			// here's our correction factor!
			temp_offset = adc_result[ADC_CHAN_TEMP];
			
			// write it to eeprom
			eeprom_write_word((uint16_t *)EEPROM_ADDR_TEMPCAL, temp_offset);
			
			// and that's it - we're done.
			adc_read_mode = 0;
			return;
		}
	}
	
	// not saving a value - read the value from eeprom
	temp_offset = eeprom_read_word((uint16_t *)EEPROM_ADDR_TEMPCAL);
	
	// does it seem invalid?
	if (temp_offset > 560) {
		// yeah, this isn't right. load rough value for 0degC from datasheet.
		// but you really should calibrate your chip, you lazy fuck.
		temp_offset = 273;
	}
}

static inline uint8_t pirate_data_send()
{
	uint8_t ret;
	
	switch (comm_cmd) {
		case MODE_EEPROM_READ: {
			if (comm_data[0] < 64) {
				i2c_slave_tx(eeprom_read_byte((uint8_t *)(uint16_t)comm_data[0]), 1);
				ret = 1;
			} else {
				// invalid address...
				i2c_disable_slave();
				ret = 255;
			}
			comm_cmd = 0;
			comm_timeout = 0;
			return ret;
		}
		
		case MODE_TEMPSENSOR_READ: {
			i2c_slave_tx(temperature, 1);
			comm_cmd = 0;
			comm_timeout = 0;
			return 1;
		}
		case MODE_LIGHTSENSOR_READ: {
			i2c_slave_tx(adc_result[comm_data[0] & 0x03] >> 2, 1);
			comm_cmd = 0;
			comm_timeout = 0;
			return 1;	
		}
		
		default: {
			// nothing active; invalid :(
			i2c_disable_slave();
			return 255;
		}
	}
	
	return 0;
}

static uint8_t pirate_process_cmd()
{
	switch (comm_cmd) {
		// no command: we just ACK this as we shouldn't ever get here
		case MODE_NONE: {
			comm_cmd = 0;
			comm_timeout = 0;
			break;
		}
		
		// standard commands
		case MODE_EXT_CMD: {		// used for more commands
			if (comm_data_idx >= 1) {
				comm_cmd = comm_data[1];
				// set data index to last (will be reset on next data sent)
				// and also reset timeout since this is effectively a new command
				comm_data_idx = COMM_DATA_SIZE - 1;
				comm_timeout = 0;
			}
			break;
		}

		case MODE_LED_SET_LEVEL: {		// sets PWM rate for the 8 PWM LEDs
			if (comm_data[0] < 4) {
				rgbled_pwm_lf[comm_data[0]] = comm_data[1];
			} else if (comm_data[0] < 8) {
				rgbled_pwm_rt[comm_data[0] - 4] = comm_data[1];
			}
			comm_cmd = 0;
			comm_timeout = 0;
			break;
		}
		
		case MODE_TEMPSENSOR_CAL: {
			// to use this function, make sure the temperature has been previously read.
			// calculates the appropriate offset based on the reported temperature.
			if (comm_data_idx >= 1) {
 				// update the offset
				if (adc_result[ADC_CHAN_TEMP]) {
					temp_offset = adc_result[ADC_CHAN_TEMP] - (int8_t)comm_data[1];
 					// write it to eeprom
 					eeprom_write_word((uint16_t *)EEPROM_ADDR_TEMPCAL, temp_offset);
 					// and that's it - we're done.
 					comm_cmd = 0;
					comm_timeout = 0;
				} else {
					// no valid read performed; I fucking told you to do this!
					comm_cmd = 0;
					comm_timeout = 0;
					return 1;
				}
			}
			break;
		}
		
		case MODE_EEPROM_READ: {
			// our data packet is already set; ready for read.
			// but if the host keeps writing...
			if (comm_data_idx > 1) {
				comm_cmd = 0;
				comm_timeout = 0;
			}
			break;
		}
		case MODE_EEPROM_WRITE: {
			// TODO: implement
			comm_cmd = 0;
			comm_timeout = 0;
			break;
		}

		// immediate commands
		case MODE_TEMPSENSOR_READ: {
			// attempt another read
			if (!adc_read_mode) {
				adc_read_mode = 0;
				adc_read_step = 0;
				adc_read_mode = ADC_MODE_TEMPSENSOR;
			}
			
			// now we are ready to read.
			// data will probably be prior data unless the host waits. (need to verify)
			comm_timeout = 0;
			break;
		}
		case MODE_LIGHTSENSOR_READ: {
			if (!adc_read_mode) {
				// set the led and step to first step
				adc_read_mode = 0;
				adc_read_step = 0;
				rgbled_sensor_read_idx(comm_data[0]);
				adc_read_mode = ADC_MODE_LIGHTSENSOR;
			}
			
			// now we are ready to read.
			// data will probably be prior data unless the host waits.
			comm_timeout = 0;
			break;
		}
		case MODE_SLEEP: {
			// put this bitch to bed.
			// we'll wake up when we get another I2C command.
			pirate_sleep_mode = SLEEP_MODE_PWR_DOWN;
			
			comm_cmd = 0;
			comm_timeout = 0;
			break;
		}
		
		// extended commands
		case MODE_AUX_PIN_SET: {
			// TODO: come up with an implementation that isn't shitty
		}
		
		case MODE_AUX_PIN_GET: {
			// TODO: implement when response sending is ...implemented
			comm_cmd = 0;
			comm_timeout = 0;
			break;
		}
		case MODE_LIGHTSENSOR_SENS: { 		// 0x10 0x19 <led 0-3> <sens 1-200>
			if (comm_data_idx >= 1) {
				if ((comm_data[0] <= 0x03) && (comm_data[1])) {
					rgbled_sensor_sensitivity(comm_data[0], comm_data[1]);
				}
				comm_cmd = 0;
				comm_timeout = 0;
			}
			break;
		}
		
		// invalid commands
		default: {
			// send NAK
			comm_cmd = 0;
			comm_timeout = 0;
			return 1;
		}
	}
	
	return 0;
}

static uint8_t pirate_process_data(uint8_t data)
{
	if (comm_cmd == MODE_NONE) {
		// we aren't processing any commands, so this must be a new command/request.
		// first 4 bits = command
		comm_cmd = data >> 4;
		// last 4 bits = optional data
		comm_data[0] = data & 0x0f;
	
		// clear our data watchdog and data index
		comm_data_idx = 0;
		comm_timeout = 0;
	
		// if this is an immediate command, process it
		if (data & 0x80) {
			return pirate_process_cmd();
		}
	} else {
		// command in progress - add data and continue
		comm_data_idx++;
		comm_data_idx &= (COMM_DATA_SIZE - 1);		// data will LOOP in case of problems; TODO: NAK on failure
		comm_data[comm_data_idx] = data;
		return pirate_process_cmd();
	}
	
	return 0;
}


/* ISR handlers */
ISR(TIMER0_COMPA_vect)
{		
	/****
	 * this mainline loop runs at 2KHz, or 0.5ms.
	 * at the faster speed, this is ~3968 cycles/loop @8MHz.
	 ****/
	
	/* TIMEKEEPING */
	// we only count a total of 10 seconds this way before we loop.
	tim0_milli++;
	if (tim0_milli >= 200) {
		tim0_milli = 0;
		tim0_centi++;
		if (tim0_centi >= 100) {
			tim0_centi = 0;
		}
	}
	
	// 2khz fix. our timer isn't evenly divisible to get our 2K, so we do this...
	OCR0A = (OCR0A == TIMER0_COMPARE) ? TIMER0_COMPARE + 1 : TIMER0_COMPARE;	
	
	
	/* ADC / LIGHT SENSOR / TEMP SENSOR */
	// main adc handler
	if (adc_read_mode) {
		switch (adc_read_mode) {
			case ADC_MODE_TEMPSENSOR: {
				pirate_tempsensor_read();
				break;
			}
			case ADC_MODE_LIGHTSENSOR: {
				rgbled_sensor_read();
				break;	
			}
		}
		
		if (adc_read_step <= 0xfe) {
			adc_read_step++;
		}
	} /* else {
		if (adc_read_step != 0xff) {
			adc_read_step++;
			if (adc_read_step == 0xff) {
				// disable the ADC
				ADCSRA &= ~(_BV(ADEN));
			}
		}
	} */ // NOTE: we need to keep the ADC enabled; it takes too long to warm up.
	
	
	/* LEDs */
	// TODO: test common cathode mode
	// figure out next LED to update (order is always RGB(X) repeating)
	if (adc_read_mode != ADC_MODE_LIGHTSENSOR) {
		rgbled_idx++;
#ifdef LED_RGBX_4LED
		rgbled_idx &= 0x03;
#else
		if (rgbled_idx > 2) {
			rgbled_idx = 0;
		}
#endif

		// and now update them
		rgbled_update();
	}
	
	
	/* COMMAND IO */
	// stop processing the command if it isn't complete in 100ms.
	if (comm_cmd != MODE_NONE) {
		if (++comm_timeout >= 50) {
			comm_cmd = MODE_NONE;
			comm_timeout = 0;
			i2c_disable_slave();
			i2c_enable_slave();
		}
	}
	
	/* PROFILING */
	// determine how much CPU usage we are consuming
	tim0_profiler = TCNT0;
}


/* communications interrupts */
#include "i2c_interrupt.h"
