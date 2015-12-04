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

uint32_t fadd(uint32_t f1, uint32_t f2) {
	union hoge inputa;
	union hoge inputb;
	union hoge winner;
	union hoge loser;
	// 仮数部の計算
	uint32_t   manta;
	uint32_t   mantb;
	uint32_t   manto;
	uint32_t   expoa;
	uint32_t   expob;
	int        expo;
	uint32_t   sign;

	int i;

	union hoge out;

	// 入力値をここにいれてね
	inputa.i = f1;
	inputb.i = f2; 

	// cmpExp
	if(fromdownto(inputa.i,30,0) < fromdownto(inputb.i,30,0)){
		winner.i = inputb.i;
		loser.i  = inputa.i;
	}else{
		winner.i = inputa.i;
		loser.i  = inputb.i;
	}

	// shiftloser
	expoa = fromdownto(winner.i,30,23);
	expob = fromdownto(loser.i ,30,23);
	manta = (fromdownto(winner.i,22,0) + ((uint32_t)1 << 23)) << 3;
	mantb = (fromdownto(loser.i ,22,0) + ((uint32_t)1 << 23)) << 3;
	for(i = 0; i < expoa - expob; i++){
		mantb = (fromdownto(mantb,26,2) << 1) + (fromdownto(mantb,1,1) | fromdownto(mantb,0,0));
	}

	// mantadd *there is no underflow because manta grater than mantb
	if(fromdownto(winner.i,31,31) == fromdownto(loser.i,31,31)){
		manto = manta + mantb;
	}else{
		manto = manta - mantb;
	}
	expo = expoa;
	sign = fromdownto(winner.i,31,31);

	// moveup
	if(fromdownto(manto,27,27) != 0){ // when overflow
		manto = (fromdownto(manto,27,2) << 1) + (fromdownto(manto,1,1) | fromdownto(manto,0,0));
		expo  = expo + 1;
	}

	// shiftup
	for(i = 0; i < 27; i++){
		if(fromdownto(manto,26-i,26-i) == 1){
			if(i == 26){
				manto = 0;
			}else{
				manto = (fromdownto(manto,25-i,0) << i);
			}
			expo = expo - i;
			break;
		}
	}

	// round
	if(fromdownto(manto,2,2) == 0 || fromdownto(manto,3,0) == 4){
		manto = fromdownto(manto,25,3);
	}else{
		if(fromdownto(manto,25,3) == 8388607){
			manto = 0;
			expo = expo + 1;
		}else{
			manto = fromdownto(manto,25,3) + 1;
		}
	}

	// output
	if(fromdownto(winner.i,30,0) == fromdownto(loser.i,30,0) && fromdownto(winner.i,31,31) != fromdownto(loser.i,31,31)){ // 答えが0なら0を返す
		out.i = 0;
	}else if(expoa == 0 || expoa == 255 || expob == 0 || expob == 255){ // どちらかがzeroまたはinfならwinnerを返す
		out.i = winner.i;
	}else if(expo > 254){ // オーバーフロー
		out.i = (sign << 31) + ((uint32_t)255 << 23);
	}else if(expo < 1){ // アンダーフロー
		out.i = 0;
	}else{
		out.i = (sign << 31) + ((uint32_t)expo << 23) + manto;
	}
	return out.i;
}
