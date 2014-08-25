/*
 * led_matrix_pattern.h: fixed patterns for the RGBLED eyes
 * 2014 by true
 *
 * ----
 *
 * $Id: led_matrix_pattern.h 174 2014-07-24 00:42:04Z true $
 */


  // all LEDs on 100%
static const LEDFixedPattern led_pattern_01[] = {
	{1000, {0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
			0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff}},
};

  // all LEDs on 50%
static const LEDFixedPattern led_pattern_02[] = {
	{1000, {0x7d7d7d7d, 0x7d7d7d7d, 0x7d7d7d7d, 0x7d7d7d7d,
			0x7d7d7d7d, 0x7d7d7d7d, 0x7d7d7d7d, 0x7d7d7d7d}},
};

  // alternate, fast fade
static const LEDFixedPattern led_pattern_03[] = {
	{600,  {0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00,
			0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff}},
	{80,  	{0x80808080, 0x80808080, 0x80808080, 0x80808080,
			0x80808080, 0x80808080, 0x80808080, 0x80808080}},
	{600,  {0x00ff00ff, 0x00ff00ff, 0x00ff00ff, 0x00ff00ff,
			0xff00ff00, 0xff00ff00, 0xff00ff00, 0xff00ff00}},
	{80,   {0x80808080, 0x80808080, 0x80808080, 0x80808080,
			0x80808080, 0x80808080, 0x80808080, 0x80808080}}
};

  // fast white/alternate flash
static const LEDFixedPattern led_pattern_04[] = {
	{80,   {0x34343434, 0x34343434, 0x34343434, 0x34343434,
			0xa0a0a0a0, 0xa0a0a0a0, 0xa0a0a0a0, 0xa0a0a0a0}},
	{80,   {0x04040404, 0x04040404, 0x04040404, 0x04040404,
			0xfdfdfdfd, 0xfdfdfdfd, 0xfdfdfdfd, 0xfdfdfdfd}},
	{80,   {0x78787878, 0x78787878, 0x78787878, 0x78787878,
			0x86868686, 0x86868686, 0x86868686, 0x86868686}},
	{80,   {0xa0a0a0a0, 0xa0a0a0a0, 0xa0a0a0a0, 0xa0a0a0a0,
			0x34343434, 0x34343434, 0x34343434, 0x34343434}},
	{80,   {0xfdfdfdfd, 0xfdfdfdfd, 0xfdfdfdfd, 0xfdfdfdfd,
			0x04040404, 0x04040404, 0x04040404, 0x04040404}},
	{80,   {0x86868686, 0x86868686, 0x86868686, 0x86868686,
			0x78787878, 0x78787878, 0x78787878, 0x78787878}}
};

  // pattern count - make sure this is updated with the amount
  // of entries in the patterns above!
static const uint8_t led_pattern_size[] = {
	1,
	1,
	4,
	6
};
