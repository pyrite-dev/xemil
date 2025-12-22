#include <xmllib.h>

#define CAST_I32(x) ((int)(x))

int xmllib_unicode_count(unsigned char c) {
	if(c < 0x80) {
		return 1;
	}
	if(0xc2 <= c && c < 0xe0) {
		return 2;
	}
	if(0xe0 <= c && c < 0xf0) {
		return 3;
	}
	if(0xf0 <= c && c < 0xf8) {
		return 4;
	}
	return 0;
}

static int unicode_later(unsigned char c) {
	return 0x80 <= c && c < 0xc0;
}

int xmllib_unicode_8_to_32(const char* input, int* output) {
	const unsigned char* inbuf = (const unsigned char*)input;
	int		     b	   = xmllib_unicode_count(inbuf[0]);
	if(b == 0) return 0;

	if(b == 1) *output = inbuf[0];
	if(b == 2) {
		if(!unicode_later(inbuf[1])) return 0;
		if((inbuf[0] & 0x1e) == 0) return 0;

		*output = CAST_I32(inbuf[0] & 0x1f) << 6;
		*output |= CAST_I32(inbuf[1] & 0x3f);
	}
	if(b == 3) {
		if(!unicode_later(inbuf[1]) || !unicode_later(inbuf[2])) return 0;
		if((inbuf[0] & 0x0f) == 0 && (inbuf[1] & 0x20) == 0) return 0;

		*output = CAST_I32(inbuf[0] & 0x0f) << 12;
		*output |= CAST_I32(inbuf[1] & 0x3f) << 6;
		*output |= CAST_I32(inbuf[2] & 0x3f);
	}
	if(b == 4) {
		if(!unicode_later(inbuf[1]) || !unicode_later(inbuf[2]) || !unicode_later(inbuf[3])) return 0;
		if((inbuf[0] & 0x07) == 0 && (inbuf[1] & 0x30) == 0) return 0;

		*output = CAST_I32(inbuf[0] & 0x07) << 18;
		*output |= CAST_I32(inbuf[1] & 0x3f) << 12;
		*output |= CAST_I32(inbuf[2] & 0x3f) << 6;
		*output |= CAST_I32(inbuf[3] & 0x3f);
	}

	return b;
}

int xmllib_unicode_32_to_8(const int input, char* output) {
	if(input < 128) {
		output[0] = input;
		return 1;
	} else if(input < 2048) {
		output[0] = 0xc0 | (input >> 6);
		output[1] = 0x80 | (input & 0x3f);
		return 2;
	} else if(input < 65536) {
		output[0] = 0xe0 | (input >> 12);
		output[1] = 0x80 | ((input >> 6) & 0x3f);
		output[2] = 0x80 | (input & 0x3f);
		return 3;
	} else {
		output[0] = 0xf0 | (input >> 18);
		output[1] = 0x80 | ((input >> 12) & 0x3f);
		output[2] = 0x80 | ((input >> 6) & 0x3f);
		output[3] = 0x80 | (input & 0x3f);
		return 4;
	}
}
