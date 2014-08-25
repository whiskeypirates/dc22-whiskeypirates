/*
 * led.c
 *
 * Created: 6/13/2014 1:44:28 AM
 *  Author: true
 */ 


#include "led.h"


/* led */
uint8_t rgbled_pwm_lf[4];				// pwm value for TIMER1 OCA LED outputs
uint8_t rgbled_pwm_rt[4];				// pwm value for TIMER1 OCB LED outputs

static uint8_t rgbled_read_sel;			// LED light sensor select
static uint8_t rgbled_sensitivity[4] = {1, 1, 1, 1};

/* adc */
volatile uint8_t adc_busy;
volatile uint16_t adc_result[ADC_MAX_FEEDBACK + 1];
volatile uint8_t adc_read_mode;


/* functions */
  // sets up the IO pins for LED output function.
void rgbled_io_init()
{
	// PortD[4:7] (left) and PortC[0:3] (right) are LEDs
	// Also, Right LEDs are capable of ADC input on ALL pins (ADC[0:3])
	
	// set all pins as outputs
	DDRD |= 0xf0;
	DDRC |= 0x0f;
	
#ifdef LED_COMMON_ANODE
	// com-anode: pin is high to disable
	PORTD |= 0xf0;
	PORTC |= 0x0f;
#else
	// com-cathode: pin is low to disable
	PORTD &= 0x0f;
	PORTC &= 0xf0;
#endif
}

  // sets the appropriate pins and PWM values for the LED index specified in rgbled_idx.
  // note: this index is updated externally, and not by this function.
void rgbled_update()
{
	// clear all selected LED pins
	// we have to clear the select line then set it because we can't fully disable the PWM for some reason
#ifdef LED_COMMON_ANODE
	// make sure all pins are high
	PORTD |= 0xf0;
	PORTC |= 0x0f;
#else
	// make sure all pins are low
	PORTD &= 0x0f;
	PORTC &= 0xf0;
#endif
	
	// set pwm OCx registers to max
	timer1_set_oca(TIMER1_COMPARE);
	timer1_set_ocb(TIMER1_COMPARE);
	// and set pwm counter value to 1 before expiry
	timer1_set(TIMER1_COMPARE - 1);
	
	// now we set the desired LED
	// turn on one LED in each group
#ifdef LED_COMMON_ANODE
	// make sure active pin is output low and others are high
	PORTD &= ~(_BV(rgbled_idx + 4));
	PORTC &= ~(_BV(rgbled_idx));
#else
	// make sure active pin is output high and others are low
	PORTD |= _BV(rgbled_idx) << 4;
	PORTC |= _BV(rgbled_idx);
#endif
	
	// load new pwm OCx values for this LED
	timer1_set_oca(rgbled_pwm_lf[rgbled_idx]);
	timer1_set_ocb(rgbled_pwm_rt[rgbled_idx]);
}

  // sets up the PortC LED matrix as a diode light sensor.
  // configures the specified LED on the ADC.
  // this function assumes common anode.
void rgbled_sensor_init(uint8_t led)
{
	// set the left eye high (fixes color flash/tearing)
	PORTD |= 0xf0;
	
	// disable both eye's PWM (required for right eye, fixes color flash/tearing in left eye)
	TCCR1A &= ~(_BV(COM1A1) | _BV(COM1A0) | _BV(COM1B1) | _BV(COM1B0));
		
	// ground the LED
	DDRC &= 0xf0;				// set all LEDs cathode as inputs
	MCUCR |= (_BV(PUD));		// globally disable pullups
	DDRC |= (_BV(led));			// set our LED as an output
	PORTC = (PORTC & 0xf0); 	// set all LEDs cathode low
	PORTB |= (_BV(PORTB2));		// set anode high
			
	// reverse LED bias
	PORTC |= (_BV(led));		// set cathode high
	PORTB &= ~(_BV(PORTB2));	// set anode low
	_delay_us(3);				// allow it to charge fully
	
	// set LED as input
	DDRC &= ~(_BV(led));		// set led cathode as input
	PORTC &= ~(_BV(led));		// set led anode pullup off
	MCUCR &= ~(_BV(PUD));		// re-enable global pullups
}

  // starts ADC to read the value of charge remaining on the LED
void rgbled_sensor_sensitivity(uint8_t ledidx, uint8_t sensitivity)
{
	if (ledidx <= 0x03) {
		if (sensitivity) {
			rgbled_sensitivity[ledidx] = sensitivity;
		}
	}
}

void rgbled_sensor_read_idx(uint8_t ledidx)
{
	rgbled_read_sel = ledidx & 0x03;
}

void rgbled_sensor_read()
{
	uint8_t sens;
	
	sens = rgbled_sensitivity[rgbled_read_sel];
	if (!sens) sens = 1;
	
	if (adc_read_step == 0) {
		// adc_init();
		// clear ADC bias by reading ground
		adc_channel(ADC_CHAN_GND, ADC_REF_AVCC);
	} else if (adc_read_step == 1) {
		adc_start(0, 0);
	} else if (adc_read_step == 2) {
		// if the ADC is done, charge up LED for reading
		if (adc_busy) {
			adc_read_step--;
		} else {
			rgbled_sensor_init(rgbled_read_sel);
			adc_channel(rgbled_read_sel, ADC_REF_NO_SET);
		}
	} else if (adc_read_step == (2 + sens)) {
		// if vref has changed, it should be stable by now; start reading LED value
		adc_start(3, 1);
	} else if (adc_read_step > (2 + sens)) {
		adc_read_step--;
		if (!adc_busy) {
			// we are done! :) finish up...
			adc_channel(ADC_CHAN_GND, ADC_REF_NO_SET); 	// change ADC to not look at a real pin
			rgbled_io_init(); 							// set LED pins up again
			timer1_pwm_reinit(); 						// and set up PWM again
			adc_read_mode = 0; 							// clear ADC read mode
		}
	}
}
