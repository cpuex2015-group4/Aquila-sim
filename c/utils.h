#ifndef UTILS_H
#define UTILS_H

union float_ui_{
  float f;
  unsigned int ui;
};

union float_i_{
  float f;
  int i;
};

extern void print_int2bin(unsigned int n);

extern void print_char2bin(unsigned char c);

extern unsigned int get_binary_unsigned(unsigned int n, int start, int end);

extern int get_binary_signed(int n, int start, int end);

extern float int2float(int i);

extern int float2int(float f);

extern float ui2float(unsigned int);

extern unsigned int float2ui(float);

extern uint32_t reverse(uint32_t);

union hoge{
	float    f;
	uint32_t i;
};

extern uint32_t fromdownto(uint32_t, int, int);

// to(up downto down) <= from;
extern uint32_t todownto(uint32_t, uint32_t, int, int);

extern void printbit(char*, uint32_t, int, int);

extern int bitwiseor(uint32_t);


#endif
