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

// to <= from(up downto down);

uint32_t fmul(uint32_t f1, uint32_t f2) {
	union hoge inputa;
	union hoge inputb;
	// 仮数部の計算
	uint32_t   manta;
	uint32_t   mantb;
	uint32_t   anshh;
	uint32_t   anshl;
	uint32_t   anslh;
	uint32_t   ans;
	uint32_t   manto;
	uint32_t   expoa;
	uint32_t   expob;
	int        expo;
	uint32_t   sign;

	union hoge out;

  // 入力値をここにいれてね
	inputa.i = f1;
	inputb.i = f2; 

	manta    = fromdownto(inputa.i,22,0) + ((uint32_t)1 << 23);
	expoa    = fromdownto(inputa.i,30,23);
	mantb    = fromdownto(inputb.i,22,0) + ((uint32_t)1 << 23);
	expob    = fromdownto(inputb.i,30,23);

	anshh    = fromdownto(manta,23,11) * fromdownto(mantb,23,11);
	anshl    = fromdownto(manta,23,11) * fromdownto(mantb,10, 0);
	anslh    = fromdownto(manta,10, 0) * fromdownto(mantb,23,11);

	ans      = anshh + fromdownto(anshl,23,11) + fromdownto(anslh,23,11) + 2;

	// 仮数部の判定,指数部の加算
	if(fromdownto(ans,25,25) == 1){
	  manto = fromdownto(ans,24,2);
		expo  = expoa + expob - 126;
	}else{
	  manto = fromdownto(ans,23,1);
		expo  = expoa + expob - 127;
	}

	sign = fromdownto(inputa.i,31,31) ^ fromdownto(inputb.i,31,31);

  // このへんはvhdl実装ではカバーしてないけどまぁレイトレでは関係ないらしいし
	if(expo < 1 || expoa == 0 || expob == 0){ // アンダーフロー
	  manto = 0;
		expo  = 0;
	}else if(expo > 254){ // オーバーフロー
	  manto = 0;
		expo  = 255;
	}

	out.i = (sign << 31) + (fromdownto((uint32_t)expo,7,0) << 23) + manto;

	return out.i;
}
