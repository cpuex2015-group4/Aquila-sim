#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

union f {
	float f;
	uint32_t i;
};

uint32_t finv(uint32_t x) {
	union f f1;
	union f res;
	f1.i = x;
	res.f = 1.0 / f1.f;
	return res.i;
}
