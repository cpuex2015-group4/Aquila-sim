#include <stdio.h>
#include <stdint.h>

uint32_t fromdownto(uint32_t from, int up, int down){
	uint32_t to = 0;
	int i;

	for(i = up; i > down-1; i--){
		to = (to << 1) | ((from >> i) & (uint32_t)1);
	}

	return to;
}

// to(up downto down) <= from;
uint32_t todownto(uint32_t from, uint32_t to, int up, int down){
	uint32_t mask = 0; // 代入すべき箇所を0で初期化
	int i;
	for(i = 0; i < mask; i++){
		mask = mask | ((uint32_t)1 << i);
	}
	for(i = up+1; i < 64; i++){
		mask = mask | ((uint32_t)1 << i);
	}
	to = to & mask;
	for(i = down; i < up+1; i++){
		to = to | ((from & (uint32_t)1) << i);
		from = from >> 1;
	}
	return to;
}

void printbit(char *name, uint32_t f, int up, int down){
	int i,j;
	printf("%s =",name);
	j = down;

	for(i = 0; down < up+1; down++){
		if(up == 31 && j == 0){
			if(i == 0 || i == 1 || i == 9) printf(" ");
			i++;
		}
		printf("%d", (int)((f & ((uint32_t)1 << (up-down))) >> (up-down)));
	}
	puts("");
}	
