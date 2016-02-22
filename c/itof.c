#include <stdio.h>
#include <stdint.h>
#include "./utils.h"

float itof(int i){
	union hoge input;
	union hoge output;
	union hoge answer;
	int        sign;
	uint32_t   sint;
	uint32_t   expo;
	uint32_t   mant, mano;
	int        flag2;
	int        zflag = 0;
	int        err = 0;

	input.i = i;
	zflag = 0;
	answer.f = (float)(int)input.i;

	sign = fromdownto(input.i, 31, 31);

	if(sign == 1){
		sint = reverse(fromdownto(input.i, 30, 0));
	}else{
		sint = fromdownto(input.i, 30, 0);
	}
	//		printbit(" int  ",input.i,31,0);
	//		printbit("sint  ",sint,31,0);

	for(i = 0; i < 30; i++){
		if(fromdownto(sint, 30-i, 30-i) == 1){
			break;
		}
	}

	if(i < 4){
		if(fromdownto(sint, 4-i, 0) != 0){
			mant = (fromdownto(sint, 29-i, 5-i) << 1) + 1;
		}else{
			mant = fromdownto(sint, 29-i, 4-i);
		}
	}else if(i == 30){
		mant = 0;
		zflag = 1;
	}else{
		mant = fromdownto(sint, 29-i, 0) << (i-4);
	}
	//		printbit("mant  ",mant,31,0);

	if(fromdownto(mant,2,2) == 0 || fromdownto(mant,3,0) == 4){
		mano = fromdownto(mant,25,3);
	}else{
		if(fromdownto(mant,25,3) == 0x7fffff){
			flag2 = 1;
			mano = 0;
		}else{
			flag2 = 0;
			mano = fromdownto(mant,25,3)+1;
		}
	}
	//		printbit("mano  ",mano,31,0);

	if(flag2 == 1){
		expo = 158-i;
	}else{
		expo = 157-i;
	}

	if(zflag == 0){
		output.i = (sign << 31) + (expo << 23) + mano;
	}else{
		output.i = 0;
	}

	//		printf("input  = %d\noutput = %lf\n",input.i, output.f);
	//printbit("output",input.i,31,0); printf(" ");
	//printbit("output",output.i,31,0); // printf(",");
	/*		printbit("answer",answer.i,31,0); */ puts("");
	if(output.i != answer.i){
		err += 1;
		//break;
	}
	//	printf("err = %d\n",err);
	return output.f;
}
