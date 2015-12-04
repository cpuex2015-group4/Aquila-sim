#ifndef __FPU__H_
#define __FPU__H_

#include <stdint.h>

union hoge{
	float    f;
	uint32_t i;
};

uint32_t fromdownto(uint32_t from, int up, int down);
uint32_t todownto(uint32_t from, uint32_t to, int up, int down);
void printbit(char *name, uint32_t f, int up, int down);

#endif //__FPU__H_
