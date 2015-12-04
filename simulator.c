#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
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
int INST_ADD_IDX = 0;
int INST_ADDI_IDX = 1;
int INST_AND_IDX = 2;
int INST_BEQ_IDX = 3;
int INST_BNE_IDX = 4;
int INST_J_IDX = 5;
int INST_JAL_IDX = 6;
int INST_JR_IDX = 7;
int INST_JRAL_IDX = 8;
int INST_LW_IDX = 9;
int INST_NOR_IDX = 10;
int INST_OR_IDX = 11;
int INST_ORI_IDX = 12;
int INST_SLT_IDX = 13;
int INST_SLTI_IDX = 14;
int INST_SLL_IDX = 15;
int INST_SRL_IDX = 16;
int INST_SW_IDX = 17;
int INST_SUB_IDX = 18;
int INST_BCLT_IDX = 19;
int INST_BCLF_IDX = 20;
int INST_DIV_IDX = 21;
int INST_ADDS_IDX = 22;
int INST_CSEQ_IDX = 23;
int INST_CSLT_IDX = 24;
int INST_CSLE_IDX = 25;
int INST_MULS_IDX = 26;
int INST_INVS_IDX = 27;
int INST_SUBS_IDX = 28;
int INST_LWS_IDX = 29;
int INST_MULT_IDX = 30;
int INST_SWS_IDX = 31;
int INST_IN__IDX = 32;
int INST_OUT_IDX = 33;
int INST_HLT_IDX = 34;

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

operands decode_R(instruction inst)
{
	operands ops;
	ops.reg_s_idx = get_binary_unsigned(inst, 6, 11);
	ops.reg_t_idx = get_binary_unsigned(inst, 11, 16);
	ops.reg_d_idx = get_binary_unsigned(inst, 16, 21);
	ops.shamt = get_binary_unsigned(inst, 21, 26);
	return ops;
}

operands decode_I(instruction inst)
{
	operands ops;
	ops.reg_s_idx = get_binary_unsigned(inst, 6, 11);
	ops.reg_t_idx = get_binary_unsigned(inst, 11, 16);
	ops.imm = get_binary_signed(inst, 16, 32);
	return ops;
}

operands decode_J(instruction inst)
{
	operands ops;
	ops.imm = get_binary_signed(inst, 6, 32);
	return ops;
}

operands decode_FR(instruction inst)
{
	operands ops;
	ops.ft_idx = get_binary_unsigned(inst, 11, 16);
	ops.fs_idx = get_binary_unsigned(inst, 16, 21);
	ops.fd_idx = get_binary_unsigned(inst, 21, 26);
	return ops;
}

operands decode_FI(instruction inst)
{
	operands ops;
	ops.ft_idx = get_binary_unsigned(inst, 11, 16);
	ops.imm = get_binary_signed(inst, 16, 32);
	return ops;
}

int inst_add(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_ADD_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	int reg_d = reg_s + reg_t;
	sim_p->reg[ops.reg_d_idx] = reg_d;
	sim_p->pc++;
	return 1;
}

int inst_addi(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_ADDI_IDX]++;
	operands ops = decode_I(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	sim_p->reg[ops.reg_t_idx] = ops.imm + reg_s;
	sim_p->pc++; 
	return 1;
}

int inst_and(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_AND_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	int reg_d = reg_s & reg_t;
	sim_p->reg[ops.reg_d_idx] = reg_d;
	sim_p->pc++;
	return 1;
}

int inst_andi(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_AND_IDX]++;
	operands ops = decode_I(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	sim_p->reg[ops.reg_t_idx] = ops.imm & reg_s;
	sim_p->pc++;
	return 1;
}

int inst_beq(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_BEQ_IDX]++;
	operands ops = decode_I(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	if(reg_s == reg_t){
		sim_p->pc += ops.imm;
	}
	sim_p->pc++;
	return 1;
}

int inst_bne(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_BNE_IDX]++;
	operands ops = decode_I(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	if(reg_s != reg_t){
		sim_p->pc += ops.imm;
	}
	sim_p->pc++;
	return 1;
}

int inst_j(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_J_IDX]++;
	operands ops = decode_J(inst);
	sim_p->pc = ops.imm;
	return 1;
}

int inst_jal(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_JAL_IDX]++;
	sim_p->reg[31] = ((sim_p->pc + 1));
	operands ops = decode_J(inst);
	sim_p->pc = ops.imm;
	return 1;
}

int inst_jr(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_JR_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	sim_p->pc = reg_s;
	return 1;
}

int inst_jral(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_JRAL_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	sim_p->reg[31] = (sim_p->pc + 1);
	sim_p->pc = reg_s;
	return 1;
}

int inst_lw(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_LW_IDX]++;
	operands ops = decode_I(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	sim_p->reg[ops.reg_t_idx] = sim_p->mem[reg_s + ops.imm];
	sim_p->pc++;
	return 1;
}

int inst_nor(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_NOR_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	sim_p->reg[ops.reg_d_idx] = ~(reg_s | reg_t);
	sim_p->pc++;
	return 1;
}

int inst_or(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_OR_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	sim_p->reg[ops.reg_d_idx] = (reg_s | reg_t);
	sim_p->pc++;
	return 1;
}

int inst_ori(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_ORI_IDX]++;
	operands ops = decode_I(inst);
	int imm = get_binary_signed(inst, 16, 32);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	sim_p->reg[ops.reg_t_idx] = (reg_s | imm);
	sim_p->pc++;
	return 1;
}

int inst_slt(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_SLT_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	if(reg_s < reg_t){
		sim_p->reg[ops.reg_d_idx] = 1;
	}else{
		sim_p->reg[ops.reg_d_idx] = 0;
	}
	sim_p->pc++;
	return 1;
}

int inst_slti(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_SLTI_IDX]++;
	operands ops = decode_I(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	if(reg_s < ops.imm){
		sim_p->reg[ops.reg_t_idx] = 1;
	}else{
		sim_p->reg[ops.reg_t_idx] = 0;
	}
	sim_p->pc++;
	return 1;
}

int inst_sll(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_SLL_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	sim_p->reg[ops.reg_d_idx] = reg_s << ops.shamt;
	sim_p->pc++;
	return 1;
}

int inst_srl(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_SRL_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	sim_p->reg[ops.reg_d_idx] = reg_s >> ops.shamt;
	sim_p->pc++;
	return 1;
}

int inst_sw(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_SW_IDX]++;
	operands ops = decode_I(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	sim_p->mem[reg_s + ops.imm] = reg_t; 
	sim_p->pc++;
	return 1;
}

int inst_sub(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_SUB_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	int reg_d = reg_s - reg_t;
	sim_p->reg[ops.reg_d_idx] = reg_d;
	sim_p->pc++;
	return 1;
}

int inst_bclt(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_BCLT_IDX]++;
	operands ops = decode_FI(inst);
	if(sim_p->fpcond == 1){
		sim_p->pc += ops.imm;
	}
	sim_p->pc++;
	return 1;
}
int inst_bclf(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_BCLF_IDX]++;
	operands ops = decode_FI(inst);
	if(sim_p->fpcond == 0){
		sim_p->pc += ops.imm;
	}
	sim_p->pc ++;
	return 1;
}

int inst_div(simulator* sim_p, instruction inst)
{
	/*
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	sim_p->Lo = reg_s / reg_t;
	sim_p->Hi = reg_s % reg_t;
	sim_p->pc++;
	return 1;
	*/
	if(INST_CNT)inst_cnt_arr[INST_DIV_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	int reg_d = reg_s / reg_t;
	sim_p->reg[ops.reg_d_idx] = reg_d;
	sim_p->pc++;
	return 1;
}

extern uint32_t fadd(uint32_t, uint32_t);

int inst_adds(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_ADDS_IDX]++;
	operands ops = decode_FR(inst);
	float ft = sim_p->f_reg[ops.ft_idx];
	float fs = sim_p->f_reg[ops.fs_idx];
	float fd = int2float(fadd(float2int(fs),  float2int(ft)));
	sim_p->f_reg[ops.fd_idx] = fd;
	sim_p->pc++;
	return 1;
}

int inst_cseq(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_CSEQ_IDX]++;
	operands ops = decode_FR(inst);
	float ft = sim_p->f_reg[ops.ft_idx];
	float fs = sim_p->f_reg[ops.fs_idx];
	sim_p->fpcond = fs==ft? 1 : 0;
	sim_p->pc++;
	return 1;
}

int inst_cslt(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_CSLT_IDX]++;
	operands ops = decode_FR(inst);
	float ft = sim_p->f_reg[ops.ft_idx];
	float fs = sim_p->f_reg[ops.fs_idx];
	sim_p->fpcond = fs<ft? 1 : 0;
	sim_p->pc++;
	return 1;
}

int inst_csle(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_CSLE_IDX]++;
	operands ops = decode_FR(inst);
	float ft = sim_p->f_reg[ops.ft_idx];
	float fs = sim_p->f_reg[ops.fs_idx];
	if(fs <= ft){
		sim_p->fpcond = 1;
	}else{
		sim_p->fpcond = 0;
	}
	//sim_p->fpcond = fs<=ft? 1 : 0;
	sim_p->pc++;
	return 1;
}

extern uint32_t fmul(uint32_t, uint32_t);

typedef union myfloat_{
	uint32_t muint;
	float mfloat;
}myfloat;

int inst_muls(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_MULS_IDX]++;
	operands ops = decode_FR(inst);
	float ft = sim_p->f_reg[ops.ft_idx];
	float fs = sim_p->f_reg[ops.fs_idx];
	myfloat mf1;
	myfloat mf2;
	myfloat mf_ans;
	mf1.mfloat = fs;
	mf2.mfloat = ft;
	mf_ans.muint = fmul(mf1.muint, mf2.muint);
	float fd = mf_ans.mfloat;
	sim_p->f_reg[ops.fd_idx] = fd;
	sim_p->pc++;
	return 1;
}

extern uint32_t finv(uint32_t f1);

int inst_invs(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_INVS_IDX]++;
	operands ops = decode_FR(inst);
	float ft = sim_p->f_reg[ops.ft_idx];
	myfloat mf;
	mf.mfloat = ft;
	mf.muint = finv(mf.muint);
	float fd = (mf.mfloat);
	sim_p->f_reg[ops.fd_idx] = fd;
	sim_p->pc++;
	return 1;
}

int inst_divs(simulator* sim_p, instruction inst)
{
	//if(INST_CNT)inst_cnt_arr[INST_DIVS_IDX]++;
	operands ops = decode_FR(inst);
	float ft = sim_p->f_reg[ops.ft_idx];
	float fs = sim_p->f_reg[ops.fs_idx];
	float fd = fs / ft;
	sim_p->f_reg[ops.fd_idx] = fd;
	sim_p->pc++;
	return 1;
}

int inst_subs(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_SUBS_IDX]++;
	operands ops = decode_FR(inst);
	float ft = sim_p->f_reg[ops.ft_idx];
	float fs = sim_p->f_reg[ops.fs_idx];
	float fd = int2float(fadd(float2int(fs), float2int(-ft)));
	sim_p->f_reg[ops.fd_idx] = fd;
	sim_p->pc++;
	return 1;
}

int inst_lws(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_LWS_IDX]++;
	operands ops = decode_I(inst);
	float ft = int2float(sim_p->mem[sim_p->reg[ops.reg_s_idx] + ops.imm]);
	sim_p->f_reg[ops.reg_t_idx] = ft;
	sim_p->pc++;
	return 1;
}

int inst_mult(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_MULT_IDX]++;
	operands ops = decode_R(inst);
	int reg_s = sim_p->reg[ops.reg_s_idx];
	int reg_t = sim_p->reg[ops.reg_t_idx];
	int reg_d = reg_s * reg_t;
	sim_p->reg[ops.reg_d_idx] = reg_d;
	sim_p->pc++;
	return 1;
}

int inst_sws(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_SWS_IDX]++;
	operands ops = decode_I(inst);
	float ft = sim_p->f_reg[ops.reg_t_idx];
	sim_p->mem[sim_p->reg[ops.reg_s_idx] + ops.imm] = float2int(ft);
	sim_p->pc++;
	return 1;
}

int inst_in_(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_IN__IDX]++;
	operands ops = decode_R(inst);
	char in_;
	in_ = getchar();
	sim_p->reg[ops.reg_t_idx] = (unsigned int)in_ & 0xff;
	sim_p->pc++;
	return 1;
}

int inst_out(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_OUT_IDX]++;
	operands ops = decode_R(inst);
	printf("%c", (char)sim_p->reg[ops.reg_t_idx]);
	sim_p->pc++;
	return 1;
}

int inst_hlt(simulator* sim_p, instruction inst)
{
	if(INST_CNT)inst_cnt_arr[INST_HLT_IDX]++;
	//print_reg(sim_p);
	return 0;
}

/*
 * Function Pointer Array
 * -----------------
 *  for function  inst_hoge(simulator* sim_p, instruction inst);
 */

int simulate_inst(simulator* sim_p, instruction inst, unsigned char operation_binary, unsigned char fmt_binary, unsigned char ft_binary, unsigned char function_binary)
{
	sim_p->dynamic_inst_cnt++;
	if(STATISTICS) sim_p->called_count_table[sim_p->pc]++;
	if(operation_binary == 0 && function_binary == 32) return inst_add(sim_p, inst);

	if(operation_binary == 8) return inst_addi(sim_p, inst);

	if(operation_binary == 0 && function_binary == 36) return inst_and(sim_p, inst);

	if(operation_binary == 12) return inst_andi(sim_p, inst);

	if(operation_binary == 4) return inst_beq(sim_p, inst);

	if(operation_binary == 5) return inst_bne(sim_p, inst);

	if(operation_binary == 2) return inst_j(sim_p, inst);

	if(operation_binary == 3) return inst_jal(sim_p, inst);

	if(operation_binary == 0 && function_binary == 8) return inst_jr(sim_p, inst);

	if(operation_binary == 0 && function_binary == 9) return inst_jral(sim_p, inst);

	if(operation_binary == 35) return inst_lw(sim_p, inst);

	if(operation_binary == 0 && function_binary == 39) return inst_nor(sim_p, inst);

	if(operation_binary == 0 && function_binary == 37) return inst_or(sim_p, inst);

	if(operation_binary == 13) return inst_ori(sim_p, inst);

	if(operation_binary == 0 && function_binary == 42) return inst_slt(sim_p, inst);

	if(operation_binary == 10) return inst_slti(sim_p, inst);

	if(operation_binary == 0 && function_binary == 0) return inst_sll(sim_p, inst);

	if(operation_binary == 0 && function_binary == 2) return inst_srl(sim_p, inst);

	if(operation_binary == 43) return inst_sw(sim_p, inst);

	if(operation_binary == 0 && function_binary == 34) return inst_sub(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 8 && ft_binary == 1) return inst_bclt(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 8 && ft_binary == 0) return inst_bclf(sim_p, inst);

	if(operation_binary == 0 && function_binary == 26) return inst_div(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 16 && function_binary == 0) return inst_adds(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 16 &&function_binary == 50) return inst_cseq(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 16 && function_binary == 60) return inst_cslt(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 16 && function_binary == 62) return inst_csle(sim_p, inst);

	if(operation_binary ==17 && fmt_binary == 16  && function_binary == 2) return inst_muls(sim_p, inst);

	//if(operation_binary == 22 && function_binary == 22) return inst_invs(sim_p, inst);
	/*
	 * MIPS say 17, 16, 3 is FDIV
	 * but map this to FINV
	 */
	if(operation_binary == 17 && fmt_binary == 16 && function_binary == 3) return inst_invs(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 16 && function_binary == 1) return inst_subs(sim_p, inst);

	if(operation_binary == 49) return inst_lws(sim_p, inst);

	if(operation_binary == 0 && function_binary == 24) return inst_mult(sim_p, inst);

	if(operation_binary == 57) return inst_sws(sim_p, inst);

	if(operation_binary == 26) return inst_in_(sim_p, inst);

	if(operation_binary == 27) return inst_out(sim_p, inst);

	if(operation_binary == 63 && function_binary == 63) return inst_hlt(sim_p, inst);

	/*
	 * Unexpected Instruction
	 */
	fprintf(stderr, "Not Found\n");
	fprintf(stderr, "operation_binary = %d\n", operation_binary);
	fprintf(stderr, "fmt_binary = %d\n", fmt_binary);
	fprintf(stderr, "ft_binary = %d\n", ft_binary);
	fprintf(stderr, "function_binary = %d\n", function_binary);
	fprintf(stderr, "inst_binary = ");
	print_int2bin(inst);
	return 0;
}

extern int simulate_inst_debug(simulator* , instruction, unsigned char, unsigned char, unsigned char, unsigned char);

void simulate(simulator* sim_p)
{
	instruction inst;
	unsigned char operation_binary;
	unsigned char fmt_binary;
	unsigned char ft_binary;
	unsigned char function_binary;
	int res = 0;
	sim_p->pc = sim_p->entry_point;

	int (*simulate_inst_p)(simulator*, instruction, unsigned char, unsigned char, unsigned char, unsigned char);
	if(IS_DEBUG){
		simulate_inst_p = simulate_inst_debug;
	}else{
		simulate_inst_p = simulate_inst;
	}
	while(1){
		inst = 0;
		inst = load_char(inst, inst2char(sim_p->inst_mem[sim_p->pc], 0), 0);
		inst = load_char(inst, inst2char(sim_p->inst_mem[sim_p->pc], 1), 1);
		inst = load_char(inst, inst2char(sim_p->inst_mem[sim_p->pc], 2), 2);
		inst = load_char(inst, inst2char(sim_p->inst_mem[sim_p->pc], 3), 3);

		operation_binary = get_binary_unsigned(inst, 0, 6);
		fmt_binary = get_binary_unsigned(inst, 6, 11);
		ft_binary = get_binary_unsigned(inst, 11, 16); 
		function_binary = get_binary_unsigned(inst, 26, 32);

		res = simulate_inst_p(sim_p, inst, operation_binary, fmt_binary, ft_binary, function_binary);
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
