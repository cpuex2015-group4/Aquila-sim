#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include "./utils.h"
#include "./simulator.h"
#include "./debugger.h"
#define handle_error(msg) \
	do { perror(msg); exit(EXIT_FAILURE); } while (0)
/*
 *
 * 88        88           88  88           
 * 88        88           88  88           
 * 88        88           88  88           
 * 88aaaaaaaa88   ,adPPYb,88  88,dPPYba,   
 * 88""""""""88  a8"    `Y88  88P'    "8a  
 * 88        88  8b       88  88       d8  
 * 88        88  "8a,   ,d88  88b,   ,a8"  
 * 88        88   `"8bbdP"Y8  8Y"Ybbd8"'   
 * ============================================================
 *
 * Hatsukoi DeBugger
 *
 * Command
 * ------------------------------------------------------------------
 * - next:
 *   run the next instruction
 *
 * - break n:
 *   set BreakPoint to the instruction whose PC is 'n'
 *
 * - continue:
 *   run the program until reaches a BreakPoint
 *
 */

extern int MEM_SIZE;
extern int SIGUNATURE; 
char* PROMPT = "Hdb > ";

static simulator sim_previous;
static instruction inst_previous;
static unsigned char operation_binary_previous;
static unsigned char fmt_binary_previous;
static unsigned char ft_binary_previous;
static unsigned char function_binary_previous;

extern int simulate_inst(simulator* , instruction, unsigned char, unsigned char, unsigned char, unsigned char);

int get_break_point(char* input)
{
	/*
	 * Args:
	 *	input ("return hoge")
	 * Returns:
	 *	pc
	 */
	int i;
	for(i = 0; i < 100; i++){
		if(input[i] == '\0')break;
	}
	//i is the length of input
	
	char* pc_buf = malloc(i - 6);
	int j;
	for(j = 0; j < (i - 6); j++){
		pc_buf[j] = input[6+j];
	}
	int pc = atoi(pc_buf); 
	free(pc_buf);
	return pc;
}

char* REGISTERS_G[] = {"\%zero", "\%at",
	"\%v0", "\%v1",
	"\%a0", "\%a1", "\%a2", "\%a3",
	"\%t0", "\%t1", "\%t2", "\%t3", "\%t4", "\%t5", "\%t6", "\%t7",
	"\%s0", "\%s1", "\%s2", "\%s3", "\%s4", "\%s5", "\%s6", "\%s7",
	"\%t8", "\%t9",
	"\%k0", "\%k1",
	"\%gp", "\%sp", "\%fp", "\%ra"};

char* REGISTERS_F[] = {"\%f0", "\%f1", "\%f2", "\%f3", "\%f4", "\%f5", "\%f6",
   	"\%f7", "\%f8", "\%f9", "\%f10", "\%f11", "\%f12",
   	"\%f13", "\%f14", "\%f15", "\%f16", "\%f17", "\%f18",
   	"\%f19", "\%f20", "\%f21", "\%f22", "\%f23", "\%f24",
	"\%f25", "\%f26", "\%f27", "\%f28", "\%f29", "\%f30",
   	"\%f31"};
/*
 * reference from simlator.c
 */
extern operands decode_R(instruction);

extern operands decode_I(instruction);

extern operands decode_J(instruction);

extern operands decode_FR(instruction);

extern operands decode_FI(instruction);

int print_inst_add(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "add %s, %s, %s\n", REGISTERS_G[ops.reg_d_idx], REGISTERS_G[ops.reg_s_idx], REGISTERS_G[ops.reg_t_idx]);
	return 1;
}

int print_inst_addi(simulator* sim_p, instruction inst)
{
	operands ops = decode_I(inst);
	fprintf(stderr, "addi %s, %s, $%d\n", REGISTERS_G[ops.reg_t_idx], REGISTERS_G[ops.reg_s_idx], ops.imm);
	return 1;
}

int print_inst_and(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "and %s, %s, %s\n", REGISTERS_G[ops.reg_d_idx], REGISTERS_G[ops.reg_s_idx], REGISTERS_G[ops.reg_t_idx]);
	return 1;
}

int print_inst_andi(simulator* sim_p, instruction inst)
{
	operands ops = decode_I(inst);
	fprintf(stderr, "andi %s, %s, $%d\n", REGISTERS_G[ops.reg_t_idx], REGISTERS_G[ops.reg_s_idx], ops.imm);
	return 1;
}

int print_inst_beq(simulator* sim_p, instruction inst)
{
	operands ops = decode_I(inst);
	fprintf(stderr, "beq %s, %s, $%d\n", REGISTERS_G[ops.reg_t_idx], REGISTERS_G[ops.reg_s_idx], ops.imm);
	return 1;
}

int print_inst_bne(simulator* sim_p, instruction inst)
{
	operands ops = decode_I(inst);
	fprintf(stderr, "bne %s, %s, $%d\n", REGISTERS_G[ops.reg_t_idx], REGISTERS_G[ops.reg_s_idx], ops.imm);
	return 1;
}

int print_inst_j(simulator* sim_p, instruction inst)
{
	operands ops = decode_J(inst);
	fprintf(stderr, "j %d\n", ops.imm);
	return 1;
}

int print_inst_jal(simulator* sim_p, instruction inst)
{
	operands ops = decode_J(inst);
	fprintf(stderr, "jal %d\n", ops.imm);
	return 1;
}

int print_inst_jr(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "jr %s\n", REGISTERS_G[ops.reg_s_idx]);
	return 1;
}

int print_inst_jral(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "jral %s\n", REGISTERS_G[ops.reg_s_idx]);
	return 1;
}

int print_inst_lw(simulator* sim_p, instruction inst)
{
	operands ops = decode_I(inst);
	fprintf(stderr, "lw %s, %d(%s)\n", REGISTERS_G[ops.reg_t_idx], ops.imm, REGISTERS_G[ops.reg_s_idx]);
	return 1;
}

int print_inst_nor(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "nor %s, %s, %s\n", REGISTERS_G[ops.reg_d_idx], REGISTERS_G[ops.reg_s_idx], REGISTERS_G[ops.reg_t_idx]);
	return 1;
}

int print_inst_or(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "or %s, %s, %s\n", REGISTERS_G[ops.reg_d_idx], REGISTERS_G[ops.reg_s_idx], REGISTERS_G[ops.reg_t_idx]);
	return 1;
}

int print_inst_ori(simulator* sim_p, instruction inst)
{
	operands ops = decode_I(inst);
	fprintf(stderr, "ori %s, %s, $%d\n", REGISTERS_G[ops.reg_t_idx], REGISTERS_G[ops.reg_s_idx], ops.imm);
	return 1;
}

int print_inst_slt(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "slt %s, %s, %s\n", REGISTERS_G[ops.reg_d_idx], REGISTERS_G[ops.reg_s_idx], REGISTERS_G[ops.reg_t_idx]);
	return 1;
}

int print_inst_slti(simulator* sim_p, instruction inst)
{
	operands ops = decode_I(inst);
	fprintf(stderr, "slti %s, %s, $%d\n", REGISTERS_G[ops.reg_t_idx], REGISTERS_G[ops.reg_s_idx], ops.imm);
	return 1;
}

int print_inst_sll(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "sll %s, %s, $%d\n", REGISTERS_G[ops.reg_d_idx], REGISTERS_G[ops.reg_s_idx], ops.shamt);
	return 1;
}

int print_inst_srl(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "slr %s, %s, $%d\n", REGISTERS_G[ops.reg_d_idx], REGISTERS_G[ops.reg_s_idx], ops.shamt);
	return 1;
}

int print_inst_sw(simulator* sim_p, instruction inst)
{
	operands ops = decode_I(inst);
	fprintf(stderr, "sw %s, %d(%s)\n", REGISTERS_G[ops.reg_t_idx], ops.imm, REGISTERS_G[ops.reg_s_idx]);
	return 1;
}

int print_inst_sub(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "sub %s, %s, %s\n", REGISTERS_G[ops.reg_d_idx], REGISTERS_G[ops.reg_s_idx], REGISTERS_G[ops.reg_t_idx]);
	return 1;
}

int print_inst_bclt(simulator* sim_p, instruction inst)
{
	operands ops = decode_FI(inst);
	fprintf(stderr, "bclt %d\n", ops.imm);
	return 1;
}

int print_inst_bclf(simulator* sim_p, instruction inst)
{
	operands ops = decode_FI(inst);
	fprintf(stderr, "bclf %d\n", ops.imm);
	return 1;
}

int print_inst_div(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "div %s, %s, %s\n", REGISTERS_G[ops.reg_d_idx], REGISTERS_G[ops.reg_s_idx], REGISTERS_G[ops.reg_t_idx]);
	return 1;
}

extern uint32_t fadd(uint32_t, uint32_t);

int print_inst_adds(simulator* sim_p, instruction inst)
{
	operands ops = decode_FR(inst);
	fprintf(stderr, "fadd %s, %s, %s\n", REGISTERS_F[ops.fd_idx], REGISTERS_F[ops.fs_idx], REGISTERS_F[ops.ft_idx]);
	return 1;
}

int print_inst_cseq(simulator* sim_p, instruction inst)
{
	operands ops = decode_FR(inst);
	fprintf(stderr, "c.eq.s %s, %s\n", REGISTERS_F[ops.ft_idx], REGISTERS_F[ops.fs_idx]);
	return 1;
}

int print_inst_cslt(simulator* sim_p, instruction inst)
{
	operands ops = decode_FR(inst);
	fprintf(stderr, "c.lt.s %s, %s\n", REGISTERS_F[ops.ft_idx], REGISTERS_F[ops.fs_idx]);
	return 1;
}

int print_inst_csle(simulator* sim_p, instruction inst)
{
	operands ops = decode_FR(inst);
	fprintf(stderr, "c.le.s %s, %s\n", REGISTERS_F[ops.ft_idx], REGISTERS_F[ops.fs_idx]);
	return 1;
}

extern uint32_t fmul(uint32_t, uint32_t);

typedef union myfloat_{
	uint32_t muint;
	float mfloat;
}myfloat;

int print_inst_muls(simulator* sim_p, instruction inst)
{
	operands ops = decode_FR(inst);
	fprintf(stderr, "fmul %s, %s, %s\n", REGISTERS_F[ops.fd_idx], REGISTERS_F[ops.fs_idx], REGISTERS_F[ops.ft_idx]);
	return 1;
}

int print_inst_invs(simulator* sim_p, instruction inst)
{
	operands ops = decode_FR(inst);
	fprintf(stderr, "finv %s, %s\n", REGISTERS_F[ops.fd_idx], REGISTERS_F[ops.ft_idx]);
	return 1;
}

int print_inst_divs(simulator* sim_p, instruction inst)
{
	//FDIV does not exist
	//operands ops = decode_FR(inst);
	return 1;
}

int print_inst_subs(simulator* sim_p, instruction inst)
{
	operands ops = decode_FR(inst);
	fprintf(stderr, "fsub %s, %s, %s\n", REGISTERS_F[ops.fd_idx], REGISTERS_F[ops.fs_idx], REGISTERS_F[ops.ft_idx]);
	return 1;
}

int print_inst_lws(simulator* sim_p, instruction inst)
{
	operands ops = decode_I(inst);
	fprintf(stderr, "flw %s, %d(%s)\n", REGISTERS_F[ops.ft_idx], ops.imm, REGISTERS_F[ops.fs_idx]);
	return 1;
}

int print_inst_mult(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "mult %s, %s, %s\n", REGISTERS_G[ops.reg_d_idx], REGISTERS_G[ops.reg_s_idx], REGISTERS_G[ops.reg_t_idx]);
	return 1;
}

int print_inst_sws(simulator* sim_p, instruction inst)
{
	operands ops = decode_I(inst);
	fprintf(stderr, "fsw %s, %d(%s)\n", REGISTERS_F[ops.ft_idx], ops.imm, REGISTERS_F[ops.fs_idx]);
	return 1;
}

int print_inst_in_(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "in %s\n", REGISTERS_F[ops.fs_idx]);
	return 1;
}

int print_inst_out(simulator* sim_p, instruction inst)
{
	operands ops = decode_R(inst);
	fprintf(stderr, "out %s\n", REGISTERS_F[ops.fs_idx]);
	return 1;
}

int print_inst_hlt(simulator* sim_p, instruction inst)
{
	return 0;
}

int print_inst(simulator* sim_p, instruction inst, unsigned char operation_binary, unsigned char fmt_binary, unsigned char ft_binary, unsigned char function_binary)
{

	if(operation_binary == 0 && function_binary == 32) return print_inst_add(sim_p, inst);

	if(operation_binary == 8) return print_inst_addi(sim_p, inst);

	if(operation_binary == 0 && function_binary == 36) return print_inst_and(sim_p, inst);

	if(operation_binary == 12) return print_inst_andi(sim_p, inst);

	if(operation_binary == 4) return print_inst_beq(sim_p, inst);

	if(operation_binary == 5) return print_inst_bne(sim_p, inst);

	if(operation_binary == 2) return print_inst_j(sim_p, inst);

	if(operation_binary == 3) return print_inst_jal(sim_p, inst);

	if(operation_binary == 0 && function_binary == 8) return print_inst_jr(sim_p, inst);

	if(operation_binary == 0 && function_binary == 9) return print_inst_jral(sim_p, inst);

	if(operation_binary == 35) return print_inst_lw(sim_p, inst);

	if(operation_binary == 0 && function_binary == 39) return print_inst_nor(sim_p, inst);

	if(operation_binary == 0 && function_binary == 37) return print_inst_or(sim_p, inst);

	if(operation_binary == 13) return print_inst_ori(sim_p, inst);

	if(operation_binary == 0 && function_binary == 42) return print_inst_slt(sim_p, inst);

	if(operation_binary == 10) return print_inst_slti(sim_p, inst);

	if(operation_binary == 0 && function_binary == 0) return print_inst_sll(sim_p, inst);

	if(operation_binary == 0 && function_binary == 2) return print_inst_srl(sim_p, inst);

	if(operation_binary == 43) return print_inst_sw(sim_p, inst);

	if(operation_binary == 0 && function_binary == 34) return print_inst_sub(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 8 && ft_binary == 1) return print_inst_bclt(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 8 && ft_binary == 0) return print_inst_bclf(sim_p, inst);

	if(operation_binary == 0 && function_binary == 26) return print_inst_div(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 16 && function_binary == 0) return print_inst_adds(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 16 &&function_binary == 50) return print_inst_cseq(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 16 && function_binary == 60) return print_inst_cslt(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 16 && function_binary == 62) return print_inst_csle(sim_p, inst);

	if(operation_binary ==17 && fmt_binary == 16  && function_binary == 2) return print_inst_muls(sim_p, inst);

	//if(operation_binary == 22 && function_binary == 22) return print_inst_invs(sim_p, inst);
	/*
	 * MIPS say 17, 16, 3 is FDIV
	 * but map this to FINV
	 */
	if(operation_binary == 17 && fmt_binary == 16 && function_binary == 3) return print_inst_invs(sim_p, inst);

	if(operation_binary == 17 && fmt_binary == 16 && function_binary == 1) return print_inst_subs(sim_p, inst);

	if(operation_binary == 49) return print_inst_lws(sim_p, inst);

	if(operation_binary == 0 && function_binary == 24) return print_inst_mult(sim_p, inst);

	if(operation_binary == 57) return print_inst_sws(sim_p, inst);

	if(operation_binary == 26) return print_inst_in_(sim_p, inst);

	if(operation_binary == 27) return print_inst_out(sim_p, inst);

	if(operation_binary == 63 && function_binary == 63) return print_inst_hlt(sim_p, inst);

	return -1;
}

/*
 * Debugger Global Variable
 */
int is_running;
int breakpoint_cnt = 0; // if True, run unless pc=BreakPoint
int breakpoint[10000000] = {}; // breakpoint[pc] is 1 if pc==breakpoint else 0

int simulate_inst_debug(simulator* sim_p, instruction inst, unsigned char operation_binary, unsigned char fmt_binary, unsigned char ft_binary, unsigned char function_binary)
{
	char input[100];
	int pc;
	while(1){
		if(is_running && !breakpoint[sim_p->pc])break; //if not breakpoint && some breakpoint is set, run continuously

		if(breakpoint[sim_p->pc]){
			breakpoint[sim_p->pc] = 0;
			breakpoint_cnt--;
			is_running = 0;
		}

		fprintf(stderr, "%lu : ", sim_p->pc);

		sim_previous = *sim_p;
		inst_previous = inst;
		operation_binary_previous = operation_binary;
		fmt_binary_previous = fmt_binary;
		ft_binary_previous = ft_binary;
		function_binary_previous = function_binary;

		print_inst(sim_p, inst, operation_binary, fmt_binary, ft_binary, function_binary);

		fprintf(stderr, "%s", PROMPT);

		scanf("%[^\n]", input);
		getchar(); //dummy to throw \n away

		if(strcmp(input, "next") == 0){
			break;
		}

		if(strcmp(input, "continue") == 0){
			is_running = 1;
			break;
		}

		if(strcmp("continue", input) < 0 && strcmp(input, "continue~") < 0){ //command == break && exists argument
			pc = get_break_point(input);
			if(breakpoint[pc]){
				continue;
			}
			breakpoint[pc] = 1;
			breakpoint_cnt++;
			continue;
		}

		if(strcmp("break", input) == 0){
			fprintf(stderr, "break nees a argument\n");
			continue;
		}

		fprintf(stderr, "invalid command\n");
		continue;
	}
	return simulate_inst(sim_p, inst, operation_binary, fmt_binary, ft_binary, function_binary);
}


void print_mem(simulator* sim)
{
	int i;
	fprintf(stderr, "------------------mem---------------\n");
	for(i = 0; i < MEM_SIZE; i++){
		if(sim->mem[i] != 0){
			fprintf(stderr, "mem[%d] = %d\n", i, sim->mem[i]);
		}
	}
	return;
}

void print_reg(simulator* sim)
{
	int i;
	fprintf(stderr, "------------------reg---------------\n");
	for(i = 0; i < 32; i++){
		fprintf(stderr, "reg[%d] = %d\n", i, sim->reg[i]);
	}
	return;
}

void print_f_reg(simulator* sim)
{
	int i;
	fprintf(stderr, "------------------freg---------------\n");
	for(i = 0; i < 32; i++){
		fprintf(stderr, "freg[%d] = %f\n", i, sim->f_reg[i]);
	}
	return;
}

void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
	/*
	 * catch SEGV and dump the previous instrction and register values
	 */
	
	fprintf(stderr, "SEGV at the instruction below\n");
	fprintf(stderr, "-------------------------------\n");
	print_mem(&sim_previous);
	print_reg(&sim_previous);
	print_f_reg(&sim_previous);
	print_inst(&sim_previous, inst_previous,operation_binary_previous, fmt_binary_previous, ft_binary_previous, function_binary_previous);
    exit(1);
}

void main_debugger(void)
{
	fprintf(stderr, "88        88           88  88          \n"); 
	fprintf(stderr, "88        88           88  88          \n"); 
	fprintf(stderr, "88        88           88  88          \n"); 
	fprintf(stderr, "88aaaaaaaa88   ,adPPYb,88  88,dPPYba,  \n"); 
	fprintf(stderr, "88\"\"\"\"\"\"\"\"88  a8\"    `Y88  88P'    \"8a \n"); 
	fprintf(stderr, "88        88  8b       88  88       d8 \n"); 
	fprintf(stderr, "88        88  \"8a,   ,d88  88b,   ,a8\" \n"); 
	fprintf(stderr, "88        88   `\"8bbdP\"Y8  8Y\"Ybbd8\"'  \n"); 
	fprintf(stderr, "=======================================\n"); 
	fprintf(stderr, "\n");

	struct sigaction sa;
	memset(&sa, 0, sizeof(sigaction));
	sigemptyset(&sa.sa_mask);
	sa.sa_sigaction = segfault_sigaction;
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL);
	
}

