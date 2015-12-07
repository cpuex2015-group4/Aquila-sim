#ifndef SIMLULATOR_H
#define SIMLULATOR_H
#define SAFE_DELETE(x) {free(x);(x)=NULL;}

typedef unsigned int instruction;

typedef struct simulator_{
	unsigned long pc;
	unsigned long dynamic_inst_cnt;
	int* reg;
	float* f_reg;
	int fpcond;
	int Hi;
	int Lo; 
	unsigned long inst_cnt;
	instruction* inst_mem;
	int* mem;
	unsigned int *called_count_table;
	unsigned int binary_size;
	unsigned int data_size;
	unsigned int text_size;
	unsigned int entry_point;
}simulator;


typedef struct operands_{
	unsigned int s_idx;
	unsigned int t_idx;
	unsigned int d_idx;
	unsigned int i;
	unsigned int rorf; // r:=0 , f:=1
	unsigned int opt;
	unsigned int bit_image;
	int imm;
}operands;


simulator *init_sim();

void load_binary(simulator* sim, FILE* fp);

void simulate(simulator* sim_p);

void free_sim(simulator *sim);

#endif
