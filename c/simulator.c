#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include "./utils.h"
#include "./simulator.h"

extern int MEM_SIZE;
extern int INST_CNT;
extern int STATISTICS;
extern int IS_DEBUG;

char* REG_ARR[] = {""};

/*
 * Assign the Constant Number to the Instructions
 */
enum INST{
	INST_ADD_IDX,
	INST_ADDI_IDX,
	INST_AND_IDX,
	INST_BEQ_IDX,
	INST_BNE_IDX,
	INST_J_IDX,
	INST_JAL_IDX,
	INST_JR_IDX,
	INST_JRAL_IDX,
	INST_LW_IDX,
	INST_NOR_IDX,
	INST_OR_IDX,
	INST_ORI_IDX,
	INST_SLT_IDX,
	INST_SLTI_IDX,
	INST_SLL_IDX,
	INST_SRL_IDX,
	INST_SW_IDX,
	INST_SUB_IDX,
	INST_BCLT_IDX,
	INST_BCLF_IDX,
	INST_DIV_IDX,
	INST_ADDS_IDX,
	INST_CSEQ_IDX,
	INST_CSLT_IDX,
	INST_CSLE_IDX,
	INST_MULS_IDX,
	INST_INVS_IDX,
	INST_SUBS_IDX,
	INST_LWS_IDX,
	INST_MULT_IDX,
	INST_SWS_IDX,
	INST_IN__IDX,
	INST_OUT_IDX,
	INST_HLT_IDX,
};

int inst_cnt_arr[35];
char* inst_name_arr[] = { "add", "addi", "and", "beq", "bne", "j", "jal", "jr", "jral", 
	"lw", "nor", "or", "ori", "slt", "slti", "sll", "srl", "sw", "sub", "bclt", "bclf", 
	"div", "adds", "cseq", "cslt", "csle", "muls", "invs", "subs", "lws", "mult", "sws", 
	"in_", "out", "hlt"};

simulator *init_sim()
{
	int i;
	simulator *sim = (simulator *)malloc(sizeof(simulator));
	sim->pc = 0;
	sim->dynamic_inst_cnt = 0;
	sim->fpcond = 0;
	sim->reg = (int*)malloc(sizeof(int) * 32);
	memset(sim->reg, 0, (sizeof(int) * 32));
	sim->f_reg = (float*)malloc(sizeof(float) * 32);

	for(i = 0; i < 32; i++){
		sim->f_reg[i] = 0.0;
	}

	//memset(sim->f_reg, 0, (sizeof(float) * 32));
	sim->reg[29] = 1048575;  //stack pointer 0xfffff
	sim->mem = calloc(sizeof(int), MEM_SIZE);
	return sim;
}

void free_sim(simulator *sim) {
	SAFE_DELETE(sim->reg);
	SAFE_DELETE(sim->f_reg);
	SAFE_DELETE(sim->mem);
	if(STATISTICS) SAFE_DELETE(sim->called_count_table);
	SAFE_DELETE(sim);
}


void print_inst_cnt(void)
{
	int i;
	for(i = 0; i < 35; i++){
		fprintf(stderr, "%s : %d\n", inst_name_arr[i], inst_cnt_arr[i]);
	}
	return;
}

instruction load_char(instruction inst, unsigned char c, int idx)
{
	unsigned int tmp = (unsigned int)c;
	tmp = tmp << 24;
	tmp = tmp >> 8 * idx;
	inst = inst | tmp;
	return inst;
}

unsigned char inst2char(instruction inst, int idx)
{
	inst = inst << 8 * (idx);
	inst = inst >> 24;
	return (unsigned char)inst;
}

void load_header(simulator* sim, unsigned char* buf)
{
	sim->text_size = load_char(sim->text_size, buf[4], 0);
	sim->text_size = load_char(sim->text_size, buf[5], 1);
	sim->text_size = load_char(sim->text_size, buf[6], 2);
	sim->text_size = load_char(sim->text_size, buf[7], 3);
	
	sim->data_size = load_char(sim->data_size, buf[8], 0);
	sim->data_size = load_char(sim->data_size, buf[9], 1);
	sim->data_size = load_char(sim->data_size, buf[10], 2);
	sim->data_size = load_char(sim->data_size, buf[11], 3);

	sim->entry_point = load_char(sim->entry_point, buf[12], 0);
	sim->entry_point = load_char(sim->entry_point, buf[13], 1);
	sim->entry_point = load_char(sim->entry_point, buf[14], 2);
	sim->entry_point = load_char(sim->entry_point, buf[15], 3);
	return;
}

void load_instruction(simulator* sim, unsigned char* buf)
{
	int OFFSET = 16;
	int i;
	for(i = 0; i < sim->text_size; i++){
		(sim->inst_mem)[i] = load_char(sim->inst_mem[i], buf[i * 4 + 0 + OFFSET], 0);
		(sim->inst_mem)[i] = load_char(sim->inst_mem[i], buf[i * 4 + 1 + OFFSET], 1);
		(sim->inst_mem)[i] = load_char(sim->inst_mem[i], buf[i * 4 + 2 + OFFSET], 2);
		(sim->inst_mem)[i] = load_char(sim->inst_mem[i], buf[i * 4 + 3 + OFFSET], 3);
	}
	return;
}

void load_data(simulator* sim, unsigned char* buf)
{
	int OFFSET = 16;
	int i;
	int data;
	for(i = 0; (i + sim->text_size) * 4 + OFFSET < sim->binary_size; i++){
		memset(&data, 0, 4);
		data = load_char(data, buf[(i + sim->text_size) * 4 + 0 + OFFSET], 0);
		data = load_char(data, buf[(i + sim->text_size) * 4 + 1 + OFFSET], 1);
		data = load_char(data, buf[(i + sim->text_size) * 4 + 2 + OFFSET], 2);
		data = load_char(data, buf[(i + sim->text_size) * 4 + 3 + OFFSET], 3);
		sim->mem[(sim->text_size + i)] = data;
	}
	return;
}

void load_binary(simulator* sim, FILE* fp)
{
	unsigned char buf[1000000];
	int binary_size;
	binary_size = fread(buf, sizeof(unsigned char), 1000000, fp);
	sim->binary_size = binary_size;
	sim->inst_mem = malloc(sizeof(unsigned char) * binary_size);


	memset(sim->inst_mem, 0, binary_size);
	load_header(sim, buf);
	load_instruction(sim, buf);
	load_data(sim, buf);

	sim->reg[28] = sim->text_size + sim->data_size; //heap pointer 
	sim->reg[29] = 1048575;  //stack pointer 0xfffff

	if(STATISTICS) {
		sim->called_count_table = calloc(sizeof(unsigned int), sim->text_size);
	} else {
		sim->called_count_table = NULL;
	}

	return;
}

int opt_int(int x, int opt){
	switch(opt){
		case 0:
			return x;
		case 1:
			return x > 0 ? x : -x;
		case 2:
			return -x;
		default:
			return x;
	}
}

int opt_float(float x, int opt){
	switch(opt){
		case 0:
			return x;
		case 1:
			return x > 0 ? x : -x;
		case 2:
			return -x;
		default:
			return x;
	}
}

operands decode_X(instruction inst)
{
	operands ops;
	ops.i = get_binary_unsigned(inst, 0, 1);
	ops.d_idx = get_binary_unsigned(inst, 6, 11);
	ops.s_idx = get_binary_unsigned(inst, 11, 16);
	ops.t_idx = get_binary_unsigned(inst, 16, 21);
	ops.funct = get_binary_unsigned(inst, 21, 31);
	return ops;
}

operands decode_B(instruction inst)
{
	operands ops;
	ops.i = get_binary_unsigned(inst, 0, 1);
	ops.rorf = get_binary_unsigned(inst, 1, 2);
	ops.opt = get_binary_unsigned(inst, 2, 4);
	ops.bit_image = get_binary_unsigned(inst, 4, 6);
	ops.d_idx = get_binary_unsigned(inst, 6, 11);
	ops.s_idx = get_binary_unsigned(inst, 11, 16);
	ops.imm = get_binary_unsigned(inst, 16, 32);
	return ops;
}

operands decode_R(instruction inst)
{
	operands ops;
	ops.i = get_binary_unsigned(inst, 0, 1);
	ops.rorf = get_binary_unsigned(inst, 1, 2);
	ops.opt = get_binary_unsigned(inst, 2, 4);
	ops.bit_image = get_binary_unsigned(inst, 4, 6);
	ops.d_idx = get_binary_unsigned(inst, 6, 11);
	ops.s_idx = get_binary_unsigned(inst, 11, 16);
	ops.t_idx = get_binary_unsigned(inst, 16, 21);
	return ops;
}

operands decode_I(instruction inst)
{
	operands ops;
	ops.i = get_binary_unsigned(inst, 0, 1);
	ops.rorf = get_binary_unsigned(inst, 1, 2);
	ops.opt = get_binary_unsigned(inst, 2, 4);
	ops.bit_image = get_binary_unsigned(inst, 4, 6);
	ops.d_idx = get_binary_unsigned(inst, 6, 11);
	ops.s_idx = get_binary_unsigned(inst, 11, 16);
	ops.imm = get_binary_unsigned(inst, 16, 32);
	return ops;
}

extern uint32_t fadd(uint32_t, uint32_t);
extern uint32_t fmul(uint32_t, uint32_t);
extern uint32_t finv(uint32_t f1);
typedef union myfloat_{
	uint32_t muint;
	float mfloat;
}myfloat;

int inst_add(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_ADD_IDX]++;
	if(ops.rorf == 0){
		int reg_s = sim_p->reg[ops.s_idx];
		int reg_t = sim_p->reg[ops.t_idx];
		int reg_d = ops.i == 0 ? reg_s + reg_t : reg_s + ops.imm;
		sim_p->reg[ops.d_idx] = opt_int(reg_d, ops.opt);
	}else{
		float reg_s = sim_p->f_reg[ops.s_idx];
		float reg_t = sim_p->f_reg[ops.t_idx];
		float reg_d = reg_s + reg_t;
		sim_p->f_reg[ops.d_idx] = opt_float(reg_d, ops.opt);
	}
	sim_p->pc++;
	return 1;
}

int inst_sub(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_SUB_IDX]++;
	if(ops.rorf == 0){
		int reg_s = sim_p->reg[ops.s_idx];
		int reg_t = sim_p->reg[ops.t_idx];
		int reg_d = ops.i == 0 ? reg_s - reg_t : reg_s - ops.imm;
		sim_p->reg[ops.d_idx] = opt_int(reg_d, ops.opt);
	}else{
		float reg_s = sim_p->f_reg[ops.s_idx];
		float reg_t = sim_p->f_reg[ops.t_idx];
		float reg_d = reg_s - reg_t;
		sim_p->f_reg[ops.d_idx] = opt_float(reg_d, ops.opt);
	}
	sim_p->pc++;
	return 1;
}

int inst_mul(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_MULS_IDX]++;
	if(ops.rorf == 0){
		int reg_s = sim_p->reg[ops.s_idx];
		int reg_t = sim_p->reg[ops.t_idx];
		int reg_d = ops.i == 0 ? reg_s * reg_t : reg_s * ops.imm;
		sim_p->reg[ops.d_idx] = opt_int(reg_d, ops.opt);
	}else{
		float reg_s = sim_p->f_reg[ops.s_idx];
		float reg_t = sim_p->f_reg[ops.t_idx];
		float reg_d = reg_s * reg_t;
		sim_p->f_reg[ops.d_idx] = opt_float(reg_d, ops.opt);
	}
	sim_p->pc++;
	return 1;
}

int inst_div(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_DIV_IDX]++;
	if(ops.rorf == 0){
		int reg_s = sim_p->reg[ops.s_idx];
		int reg_t = sim_p->reg[ops.t_idx];
		int reg_d = ops.i == 0 ? reg_s / reg_t : reg_s / ops.imm;
		sim_p->reg[ops.d_idx] = opt_int(reg_d, ops.opt);
	}else{
		float reg_s = sim_p->f_reg[ops.s_idx];
		float reg_t = sim_p->f_reg[ops.t_idx];
		float reg_d = reg_s / reg_t;
		sim_p->f_reg[ops.d_idx] = opt_float(reg_d, ops.opt);
	}
	sim_p->pc++;
	return 1;
}

int inst_invs(simulator* sim_p, operands ops)
{
	float reg_s = sim_p->f_reg[ops.s_idx];
	float reg_d = 1.0 / reg_s;
	sim_p->f_reg[ops.d_idx] = opt_float(reg_d, ops.opt);
	sim_p->pc++;
	return 1;
}

int inst_sqrts(simulator* sim_p, operands ops)
{
	float reg_s = sim_p->f_reg[ops.s_idx];
	float reg_d = sqrt(reg_s);
	sim_p->f_reg[ops.d_idx] = opt_float(reg_d, ops.opt);
	sim_p->pc++;
	return 1;
}

int inst_beq(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_BEQ_IDX]++;
	if(ops.rorf == 0){
		int reg_d = sim_p->reg[ops.d_idx];
		int reg_s = sim_p->reg[ops.s_idx];
		if(reg_d == reg_s){
			sim_p->pc += ops.imm;
		}
	}else{
		float reg_d = sim_p->f_reg[ops.d_idx];
		float reg_s = sim_p->f_reg[ops.s_idx];
		if(reg_d == reg_s){
			sim_p->pc += ops.imm;
		}
	}
	sim_p->pc++;
	return 1;
}

int inst_blt(simulator* sim_p, operands ops)
{
	if(ops.rorf == 0){
		int reg_d = sim_p->reg[ops.d_idx];
		int reg_s = sim_p->reg[ops.s_idx];
		if(reg_d < reg_s){
			sim_p->pc += ops.imm;
		}
	}else{
		float reg_d = sim_p->f_reg[ops.d_idx];
		float reg_s = sim_p->f_reg[ops.s_idx];
		if(reg_d < reg_s){
			sim_p->pc += ops.imm;
		}
	}
	sim_p->pc++;
	return 1;
}

int inst_ble(simulator* sim_p, operands ops)
{
	if(ops.rorf == 0){
		int reg_d = sim_p->reg[ops.d_idx];
		int reg_s = sim_p->reg[ops.s_idx];
		if(reg_d <= reg_s){
			sim_p->pc += ops.imm;
		}
	}else{
		float reg_d = sim_p->f_reg[ops.d_idx];
		float reg_s = sim_p->f_reg[ops.s_idx];
		if(reg_d <= reg_s){
			sim_p->pc += ops.imm;
		}
	}
	sim_p->pc++;
	return 1;
}
int inst_j(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_J_IDX]++;
	sim_p->pc = ops.imm;
	if(ops.opt == 1){
		sim_p->reg[31] = ((sim_p->pc + 1));
	}
	return 1;
}

int inst_jr(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_JR_IDX]++;
	int reg_s = sim_p->reg[ops.s_idx];
	sim_p->pc = reg_s;
	if(ops.opt == 1){
		sim_p->reg[31] = ((sim_p->pc + 1));
	}
	return 1;
}

int inst_ld(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_LW_IDX]++;
	int reg_s = sim_p->reg[ops.s_idx];
	if(ops.rorf == 0){
		sim_p->reg[ops.t_idx] = sim_p->mem[reg_s + ops.imm];
	}else{
		sim_p->f_reg[ops.t_idx] = int2float(sim_p->mem[reg_s + ops.imm]);
	}
	sim_p->pc++;
	return 1;
}

int inst_st(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_SW_IDX]++;
	int reg_s = sim_p->reg[ops.s_idx];
	if(ops.rorf == 0){
		sim_p->mem[reg_s + ops.imm] = sim_p->reg[ops.t_idx]; 
	}else{
		sim_p->mem[reg_s + ops.imm] = float2int(sim_p->f_reg[ops.t_idx]);
	}
	sim_p->pc++;
	return 1;
}

int inst_sll(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_SLL_IDX]++;
	int reg_s = sim_p->reg[ops.s_idx];
	sim_p->reg[ops.d_idx] = reg_s << ops.imm;
	sim_p->pc++;
	return 1;
}

int inst_srl(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_SRL_IDX]++;
	int reg_s = sim_p->reg[ops.s_idx];
	sim_p->reg[ops.d_idx] = reg_s >> ops.imm;
	sim_p->pc++;
	return 1;
}

int inst_in_(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_IN__IDX]++;
	char in_;
	in_ = getchar();
	sim_p->reg[ops.t_idx] = (unsigned int)in_ & 0xff;
	sim_p->pc++;
	return 1;
}

int inst_out(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_OUT_IDX]++;
	printf("%c", (char)sim_p->reg[ops.t_idx]);
	sim_p->pc++;
	return 1;
}

int inst_nop(simulator* sim_p, operands ops)
{
	sim_p->pc++;
	return 1;
}

int inst_itof(simulator* sim_p, operands ops)
{
	sim_p->pc++;
	return 1;
}

int inst_ftoi(simulator* sim_p, operands ops)
{
	sim_p->pc++;
	return 1;
}

int inst_hlt(simulator* sim_p, operands ops)
{
	if(INST_CNT)inst_cnt_arr[INST_HLT_IDX]++;
	return 0;
}


int simulate_inst(simulator* sim_p, instruction inst, unsigned char i_binary,  unsigned char operation_binary,  unsigned char function_binary, unsigned char xs_binary)
{
	sim_p->dynamic_inst_cnt++;
	if(STATISTICS) sim_p->called_count_table[sim_p->pc]++;

	unsigned char opt_binary = get_binary_unsigned(inst, 2, 4);
	operands ops; 

	if(i_binary == 0 && xs_binary == 0){
		/*
		 * Format X
		 */
		ops = decode_X(inst);
		switch(function_binary){
			case 0:
				return inst_hlt(sim_p, ops);
				break;
			case 1:
				return inst_nop(sim_p, ops);
				break;
			case 2:
				return inst_in_(sim_p, ops);
				break;
			case 3:
				return inst_out(sim_p, ops);
				break;
			case 4:
				return inst_itof(sim_p, ops);
				break;
			case 5:
				return inst_ftoi(sim_p, ops);
				break;
		}
	}

	if(i_binary == 0 && opt_binary == 3){
		/*
		 * Format B
		 */
		ops = decode_B(inst);
		switch(ops.bit_image){
			case 0:
				return inst_beq(sim_p, ops);
				break;
			case 1:
				return inst_blt(sim_p, ops);
				break;
			case 2:
				return inst_ble(sim_p, ops);
				break;
		}
	}

	if(i_binary == 0){
		/*
		 * Format R
		 */
		ops = decode_R(inst);
		if(ops.bit_image == 0){
			return inst_add(sim_p, ops);
		}else if(ops.bit_image == 1){
			return inst_sub(sim_p, ops);
		}else if(ops.rorf == 1 && ops.bit_image == 2){
			return inst_mul(sim_p, ops);
		}else if(ops.rorf == 1 && ops.bit_image == 3){
			return inst_div(sim_p, ops);
		}
	}

	if(i_binary == 1){
		/*
		 * Format I
		 */
		ops = decode_I(inst);
		if(ops.opt == 3){
			if(ops.bit_image == 0){
				return inst_ld(sim_p, ops);
			}else if(ops.bit_image == 1){
				return inst_st(sim_p, ops);
			}else if(ops.rorf == 0 && ops.bit_image == 2){
				return inst_sll(sim_p, ops);
			}else if(ops.rorf == 0 && ops.bit_image == 3){
				return inst_srl(sim_p, ops);
			}else if(ops.rorf == 1 && ops.bit_image == 2){
				return inst_invs(sim_p, ops);
			}else if(ops.rorf == 1 && ops.bit_image == 3){
				return inst_sqrts(sim_p, ops);
			}
		}else{
			if(ops.bit_image == 0){
				return inst_add(sim_p, ops);
			}else if(ops.bit_image == 1){
				return inst_sub(sim_p, ops);
			}else if(ops.rorf == 0 && ops.bit_image == 2){
				return inst_j(sim_p, ops);
			}else if(ops.rorf == 0 && ops.bit_image == 3){
				return inst_jr(sim_p, ops);
			}else if(ops.rorf == 1 && ops.bit_image == 2){
				return inst_mul(sim_p, ops);
			}else if(ops.rorf == 1 && ops.bit_image == 3){
				return inst_div(sim_p, ops);
			}
		}
	}

	/*
	 * Unexpected Instruction
	 */
	fprintf(stderr, "Not Found\n");
	fprintf(stderr, "operation_binary = %d\n", operation_binary);
	fprintf(stderr, "function_binary = %d\n", function_binary);
	fprintf(stderr, "inst_binary = ");
	print_int2bin(inst);
	return 0;
}

//extern int simulate_inst_debug(simulator* , instruction, unsigned char, unsigned char, unsigned char, unsigned char);

void simulate(simulator* sim_p)
{
	instruction inst;
	unsigned char i_binary;
	unsigned char operation_binary;
	unsigned char function_binary;
	unsigned char xs_binary;
	int res = 0;
	sim_p->pc = sim_p->entry_point;

	int (*simulate_inst_p)(simulator*, instruction, unsigned char, unsigned char, unsigned char, unsigned char);
	if(IS_DEBUG){
		//simulate_inst_p = simulate_inst_debug;
		simulate_inst_p = simulate_inst;
	}else{
		simulate_inst_p = simulate_inst;
	}
	while(1){
		inst = 0;
		inst = load_char(inst, inst2char(sim_p->inst_mem[sim_p->pc], 0), 0);
		inst = load_char(inst, inst2char(sim_p->inst_mem[sim_p->pc], 1), 1);
		inst = load_char(inst, inst2char(sim_p->inst_mem[sim_p->pc], 2), 2);
		inst = load_char(inst, inst2char(sim_p->inst_mem[sim_p->pc], 3), 3);

		i_binary = get_binary_unsigned(inst, 0, 1);
		operation_binary = get_binary_unsigned(inst, 1, 6);
		function_binary = get_binary_unsigned(inst, 21, 31);
		xs_binary = get_binary_unsigned(inst, 31, 32);

		res = simulate_inst_p(sim_p, inst, i_binary, operation_binary, function_binary, xs_binary);
		if(res == 0){
			break;
		}else{
			continue;
		}
	}

	/*
	 * Dynamic Instruction Count
	 */
	if(INST_CNT)print_inst_cnt();
	fprintf(stderr, "dynamic_inst_cnt = %lu\n", sim_p->dynamic_inst_cnt);

	/*
	 * Print Resut
	 * ---------------------------------------
	 *  Format:  "int:[int_res],float:[float_res]"
	 */
	fprintf(stderr, "int:%d,float:%.8f\n", sim_p->reg[2], sim_p->f_reg[2]);
}
