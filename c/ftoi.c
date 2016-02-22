#include <stdio.h>
#include <stdint.h>
#include "./utils.h"

int ftoi(float f)
{
	union hoge input;
	int        sign;
	uint32_t   into;
	uint32_t   output;
	uint32_t   expo;
	uint32_t   mant;

	input.f = f;
	// 入力値

	sign = fromdownto(input.i, 31, 31);
	expo = fromdownto(input.i, 30, 23);
	mant = fromdownto(input.i, 22,  0);

	// todo:rv,ov into:34bit -> 32bit
	if(expo < 126){
		into = 0;
	}else if(expo == 126){
		into = 0;
	}else if(expo == 127){
		into = 1;
	}else if(expo > 157){
		into = 1 << 31; // overflow
	}else if(expo > 149){
		into = (1 << (expo - 127)) + (fromdownto(mant, 22, 0) << (expo - 150));
	}else if(expo == 149){
		into = (1 << (expo - 127)) + fromdownto(mant, 22, (150 - expo));
	}else{
		into = (1 << (expo - 127)) + fromdownto(mant, 22, (150 - expo));
	}

	if(into == 0x80000000){ // when overflow
		output = into;
	}else{
		if(sign == 1){ // 符号処理
			into = reverse(into);
		}
		if(into == 0){ // when 0
			output = into;
		}else{
			output = (sign << 31) + fromdownto(into,30,0);
		}
	}
	return output;
}
