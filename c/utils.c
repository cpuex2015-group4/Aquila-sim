#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "./utils.h"

void print_int2bin(unsigned int n)
{
	char buf[32];
	int i;
	for(i = 0; i < 32; i++){
		if(n % 2 == 0){
			buf[i] = '0';
		}else{
			buf[i] = '1';
		}
		n = n >> 1;
	}
	for(i = 0; i < 32; i++){
		printf("%c", buf[31 - i]);
	}
	puts("");
	return;
}

void print_char2bin(unsigned char c)
{
	unsigned char buf[8];
	int i;
	for(i = 0; i < 8; i++){
		if(c % 2 == 0){
			buf[i] = '0';
		}else{
			buf[i] = '1';
		}
		c = c >> 1;
	}
	for(i = 0; i < 8; i++){
		printf("%c", buf[7 - i]);
	}
	puts("");
	return;
}

unsigned int get_binary_unsigned(unsigned int n, int start, int end)
{
	//printf("inst_int = %d\n", inst.inst_int);
	int len = end - start;
	//printf("%int", inst.inst_int);
	n = n << start;
	n = n >> (32 - len);
	return n;
}

int get_binary_signed(int n, int start, int end)
{
	//printf("inst_int = %d\n", inst.inst_int);
	int len = end - start;
	//printf("%int", inst.inst_int);
	n = n << start;
	n = n >> (32 - len);
	return n;
}

float int2float(int i)
{
  union float_i_ fi;
  fi.i = i;
  return fi.f;
}

int float2int(float f)
{
  union float_i_ fi;
  fi.f = f;
  return fi.i;
}

float ui2float(unsigned int ui)
{
  union float_ui_ f_ui;
  f_ui.ui = ui;
  return f_ui.f;
}
   
unsigned int float2ui(float f)
{
	union float_ui_ f_ui;
	f_ui.f = f;
	return f_ui.ui;
}

uint32_t reverse(uint32_t in)
{
	int i;
	uint32_t out = 0;
	for(i = 0; i < 32; i++){
		if((in >> i) % 2 == 0){
			out += (1 << i);
		}
	}
	return out + 1;
}

extern uint32_t fromdownto(uint32_t from, int up, int down);

// to(up downto down) <= from;
extern uint32_t todownto(uint32_t from, uint32_t to, int up, int down);

extern void printbit(char *name, uint32_t f, int up, int down);

int bitwiseor(uint32_t i);

