#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

union f {
	float f;
	uint32_t i;
};

uint32_t fmul(uint32_t x, uint32_t y) {
	union f f1;
	union f f2;
	union f res;
	f1.i = x;
	f2.i = y;
	res.f = f1.f * f2.f;
	return res.i;
}
