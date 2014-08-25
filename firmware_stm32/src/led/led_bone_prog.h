/*
 * led_bone.c: skull crossbones led matrix fixed patterns
 * 2014 true
 *
 * ----
 *
 * $Id: led_bone_prog.h 211 2014-08-03 21:06:13Z true $
 *
 */


static uint16_t led_prog_wait; 		// used by function to keep track of iteration time
static uint32_t led_prog_state[4]; 	// used by the function to keep track of lit LEDs, state
static uint32_t led_prog_work[4]; 	// used for misc shit

static uint16_t led_prog_set_wait; 	// used by setter function to set iteration time in ms
static uint16_t led_prog_set_level;	// used by setter function to set desired level
static uint32_t led_prog_set_offset;// used by setter function to set desired offset
static uint32_t led_prog_set_option;// used by setter to set options


/* programs */
static void led_prog_all_on()
{
	uint8_t i;

	if (!led_prog_wait) {
		led_prog_wait = 50;

		for (i = 0; i < 9; i++) {
			led_level[i] = led_prog_set_level & 0xff;
		}
	}

	led_prog_wait--;
}

/********
 * around the bones in a loop program
 * speed: 	set as desired
 * options: bit 2 = direction (0 = clockwise, 1 = counter-clockwise),
 * 			bit 4 = decay trails faster, bit 5 = decay trails even faster,
 *          bit 6 = trails enable
 *********/

static void led_prog_loops()
{
	uint8_t i;

	if (!led_prog_wait) {
		// load new timer wait value
		led_prog_wait = led_prog_set_wait * 5;

		// first run?
		if (!led_prog_state[3]) {
			// store starting offset
			led_prog_state[0] = led_prog_set_offset & 0x07;

			// reset levels
			for (i = 0; i < 9; i++) {
				led_level[i] = 0;
			}

			led_prog_state[3] = 1;
		}

		// set decay rate (LED PWM active values are right shifted by this value)
		led_prog_work[0] = 1;
		if (led_prog_set_option & BIT_4) led_prog_work[0]++;
		if (led_prog_set_option & BIT_5) led_prog_work[0] += 2;

		for (i = 0; i < 8; i++) {
			if (i == led_prog_state[0]) {
				// LED is next in line so turn it on
				led_level[led_bone_order[i]] = led_prog_set_level & 0xff;
			} else {
				// LED is not on so decay or turn off
				led_level[led_bone_order[i]] = (led_prog_set_option & BIT_6) ?
						led_level[led_bone_order[i]] >> led_prog_work[0] : 0;
			}
		}

		// update state with next LED
		led_prog_state[0] += (led_prog_set_option & BIT_2) ? 7 : 1;
		led_prog_state[0] &= 0x07;
	}

	led_prog_wait--;
}

/********
 * sweeping up and down symetically
 * speed: 	set as desired
 * options: bit 4 = decay trails faster, bit 5 = decay trails even faster,
 *          bit 6 = trails enable
 *********/
void led_prog_updown()
{
	uint8_t i;

	if (!led_prog_wait) {
		// load new timer wait value
		led_prog_wait = led_prog_set_wait * 5;

		// first run?
		if (!led_prog_state[3]) {
			// store starting offset
			led_prog_state[0] = led_prog_set_offset & 0x07;

			// reset levels
			for (i = 0; i < 9; i++) {
				led_level[i] = 0;
			}

			led_prog_state[3] = 1;
		}

		// set decay rate (LED PWM active values are right shifted by this value)
		led_prog_work[0] = 1;
		if (led_prog_set_option & BIT_4) led_prog_work[0] = 2;
		if (led_prog_set_option & BIT_5) led_prog_work[0] += 2;

		led_prog_work[1] = led_prog_state[0] >= 4 ? 7 - led_prog_state[0] : led_prog_state[0];

		for (i = 0; i < 4; i++) {
			if (i == led_prog_work[1]) {
				// LED is next in line so turn it on
				led_level[led_bone_order[i]] = led_prog_set_level & 0xff;
			} else {
				// LED is not on so decay or turn off
				led_level[led_bone_order[i]] = (led_prog_set_option & BIT_6) ?
						led_level[led_bone_order[i]] >> led_prog_work[0] : 0;
			}

			led_level[led_bone_order[7 - i]] = led_level[led_bone_order[i]];
		}

		// update state with next LED
		led_prog_state[0] += (led_prog_set_option & BIT_2) ? 7 : 1;
		led_prog_state[0] &= 0x07;

	}

	led_prog_wait--;
}
