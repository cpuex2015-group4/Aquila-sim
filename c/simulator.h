#ifndef SIMLULATOR_H
#define SIMLULATOR_H
#define SAFE_DELETE(x) {free(x);(x)=NULL;}

static const long PC_OFFSET = 0x400;
static const int RA_IDX = 4;
static const int BUF_IN_SIZE = 100000;

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
	FILE *fp;
	char* buf_in;
}simulator;


typedef struct operands_{
	unsigned int s_idx;
	unsigned int t_idx;
	unsigned int d_idx;
	unsigned int i;
	unsigned int rorf; // r:=0 , f:=1
	unsigned int opt;
	unsigned int bit_image;
	unsigned int funct;
	int imm;
}operands;


extern simulator *init_sim();

extern void load_binary(simulator* sim, FILE* fp);

extern void simulate(simulator* sim_p);

extern void free_sim(simulator *sim);

extern operands decode_X(instruction inst);

extern operands decode_B(instruction inst);

extern operands decode_R(instruction inst);

extern operands decode_I(instruction inst);

#endif
