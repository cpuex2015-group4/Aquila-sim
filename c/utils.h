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

void print_int2bin(unsigned int n);

void print_char2bin(unsigned char c);

unsigned int get_binary_unsigned(unsigned int n, int start, int end);

int get_binary_signed(int n, int start, int end);

float int2float(int i);

int float2int(float f);

float ui2float(unsigned int);

unsigned int float2ui(float);

#endif
