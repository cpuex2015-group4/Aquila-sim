#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "fpu.h"

// A = 1 X X X X X X X X X | X X X X X X X X X X X X X X
//    [      A0(key)      ] [        A1(variable)       ]
// FINVMAN(A) = (2*x0 - A0*x0^2/2^(23+k)) - A1*x0^2/2^46
//             [        constant         ]    [ liner  ]
//
// output file : finv_table.coe
// format :
// MEMORY_INITIALIZATION_RADIX=2;
// MEMORY_INITIALIZATION_VECTOR=
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX,
// ...
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX;

uint32_t finv(uint32_t f1) {
	union hoge input;
	union hoge a0;
	union hoge a1;
	union hoge x1; // to derive initial number
	union hoge x2; // to derive initial number
	union hoge constant;
	union hoge gradient;
	union hoge one;

	uint32_t mant;
	uint32_t mantc;
	uint32_t mantg;
	uint32_t manto;
	union hoge out;

	input.i  = f1; // input 
	one.f    = 1.0;

	a0.i = one.i + (fromdownto(input.i, 22, 13) << 13);
	a1.i = one.i + fromdownto(input.i, 22,  0);

	x1.f = one.f / a0.f;
	x2.i = a0.i + ((uint32_t)1 << 13);
	x2.f = one.f / x2.f;

	constant.f = pow((sqrt(x1.f) + sqrt(x2.f)),2) / 2;
	gradient.f = x1.f * x2.f;

	mant  = fromdownto(a1.i      ,22,0) + ((uint32_t)1 << 23);
	mantc = fromdownto(constant.i,22,0) + ((uint32_t)1 << 23);
	if(fromdownto(gradient.i,30,23) == 126){
		mantg = (fromdownto(gradient.i,22,0) + ((uint32_t)1 << 23)) / 2;
	}else{
		mantg = (fromdownto(gradient.i,22,0) + ((uint32_t)1 << 23)) / 4;
	}
	manto = ((uint64_t)mantg * (uint64_t)mant) / 8388608;
	manto = mantc - manto;

	out.i = (fromdownto(input.i,31,31) << 31) + ((uint32_t)(253 - fromdownto(input.i,30,23)) << 23) + (fromdownto(manto,21,0) << 1);

	return out.i;
}
