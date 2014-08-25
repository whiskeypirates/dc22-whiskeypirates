/*
 * led_bone.c: skull crossbones led matrix fixed patterns
 * 2014 true
 *
 * ----
 *
 * $Id: led_bone_pattern.h 199 2014-08-01 05:14:30Z true $
 *
 */

// LEDs are in this arrangement:
// W-BR W-TR xxxx
// P-BR P-TL W-TL
// P-TR P-BL W-BL


  // all on 100%
static const LEDBonePattern led_pattern_01[] = {
	{1000, {0xff, 0xff, 0x00,
			0xff, 0xff, 0xff,
			0xff, 0xff, 0xff}},
};

  // all on 50%
static const LEDBonePattern led_pattern_02[] = {
	{1000, {0x33, 0x33, 0x00,
			0x33, 0x33, 0x33,
			0x33, 0x33, 0x33}},
};

  // all on, white matching apparent purple 100% brightness
static const LEDBonePattern led_pattern_03[] = {
	{1000, {0x10, 0x10, 0x00,
			0x10, 0x10, 0x10,
			0x10, 0x10, 0x10}},
};

  // pattern count - make sure this is updated with the amount
  // of entries in the patterns above!
static uint8_t led_pattern_size[] = {
	1,
	1,
	1
};
