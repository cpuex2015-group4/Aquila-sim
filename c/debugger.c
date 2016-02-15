#include <stdio.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
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
static unsigned char i_binary_previous;
static unsigned char operation_binary_previous;
static unsigned char function_binary_previous;
static unsigned char xs_binary_previous;

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

typedef union myfloat_{
	uint32_t muint;
	float mfloat;
}myfloat;

/*
 * reference from simlator.c
 */
extern operands decode_X(instruction);

extern operands decode_B(instruction);

extern operands decode_R(instruction);

extern operands decode_I(instruction);


int print_inst_add(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_add\n");
	return 1;
}

int print_inst_sub(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_sub\n");
	return 1;
}

int print_inst_mul(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_muls\n");
	return 1;
}

int print_inst_div(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_div\n");
	return 1;
}

int print_inst_invs(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_invs\n");
	return 1;
}

int print_inst_sqrts(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_sqrts\n");
	return 1;
}

int print_inst_beq(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_beq\n");
	return 1;
}

int print_inst_blt(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_blt\n");
	return 1;
}

int print_inst_ble(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_ble\n");
	return 1;
}

int print_inst_j(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_j\n");
	return 1;
}

int print_inst_jr(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_jr\n");
	return 1;
}

int print_inst_ld(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_ld\n");
	return 1;
}

int print_inst_st(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_st\n");
	return 1;
}

int print_inst_sll(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_sll\n");
	return 1;
}

int print_inst_srl(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_srl\n");
	return 1;
}

int print_inst_nop(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_nop\n");
	return 1;
}

int print_inst_itof(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_itof\n");
	return 1;
}

int print_inst_ftoi(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_ftoi\n");
	return 1;
}

int print_inst_in_(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_in_\n");
	return 1;
}

int print_inst_out(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_out\n");
	return 1;
}

int print_inst_hlt(simulator* sim_p, operands ops)
{
	fprintf(stderr, "inst_hlt\n");
	return 0;
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

int print_inst(simulator* sim_p, instruction inst, unsigned char i_binary, unsigned char operation_binary, unsigned char function_binary, unsigned char xs_binary)
{
	unsigned char opt_binary = get_binary_unsigned(inst, 2, 4);
	operands ops; 

	print_int2bin(inst);
	if(i_binary == 0 && operation_binary == 0 && xs_binary == 0){
		/*
		 * Format X
		 */
		ops = decode_X(inst);
		switch(function_binary){
			case 0:
				return print_inst_hlt(sim_p, ops);
				break;
			case 1:
				return print_inst_nop(sim_p, ops);
				break;
			case 2:
				return print_inst_in_(sim_p, ops);
				break;
			case 3:
				return print_inst_out(sim_p, ops);
				break;
			case 4:
				return print_inst_itof(sim_p, ops);
				break;
			case 5:
				return print_inst_ftoi(sim_p, ops);
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
				return print_inst_beq(sim_p, ops);
				break;
			case 1:
				return print_inst_blt(sim_p, ops);
				break;
			case 2:
				return print_inst_ble(sim_p, ops);
				break;
		}
	}

	if(i_binary == 0){
		/*
		 * Format R
		 */
		ops = decode_R(inst);
		if(ops.bit_image == 0){
			return print_inst_add(sim_p, ops);
		}else if(ops.bit_image == 1){
			return print_inst_sub(sim_p, ops);
		}else if(ops.rorf == 1 && ops.bit_image == 2){
			return print_inst_mul(sim_p, ops);
		}else if(ops.rorf == 1 && ops.bit_image == 3){
			return print_inst_div(sim_p, ops);
		}
	}

	if(i_binary == 1){
		/*
		 * Format I
		 */
		ops = decode_I(inst);
		if(ops.opt == 3){
			if(ops.bit_image == 0){
				return print_inst_ld(sim_p, ops);
			}else if(ops.bit_image == 1){
				return print_inst_st(sim_p, ops);
			}else if(ops.rorf == 0 && ops.bit_image == 2){
				return print_inst_sll(sim_p, ops);
			}else if(ops.rorf == 0 && ops.bit_image == 3){
				return print_inst_srl(sim_p, ops);
			}else if(ops.rorf == 1 && ops.bit_image == 2){
				return print_inst_invs(sim_p, ops);
			}else if(ops.rorf == 1 && ops.bit_image == 3){
				return print_inst_sqrts(sim_p, ops);
			}
		}else{
			if(ops.bit_image == 0){
				return print_inst_add(sim_p, ops);
			}else if(ops.bit_image == 1){
				return print_inst_sub(sim_p, ops);
			}else if(ops.rorf == 0 && ops.bit_image == 2){
				return print_inst_j(sim_p, ops);
			}else if(ops.rorf == 0 && ops.bit_image == 3){
				return print_inst_jr(sim_p, ops);
			}else if(ops.rorf == 1 && ops.bit_image == 2){
				return print_inst_mul(sim_p, ops);
			}else if(ops.rorf == 1 && ops.bit_image == 3){
				return print_inst_div(sim_p, ops);
			}
		}
	}

	return -1;
}

/*
 * Debugger Global Variable
 */
int is_running;
int breakpoint_cnt = 0; // if True, run unless pc=BreakPoint
int breakpoint[10000000] = {}; // breakpoint[pc] is 1 if pc==breakpoint else 0

int simulate_inst_debug(simulator* sim_p, instruction inst, unsigned char i_binary, unsigned char operation_binary, unsigned char function_binary, unsigned char xs_binary)
{
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
		i_binary_previous = i_binary;
		operation_binary_previous = operation_binary;
		function_binary_previous = function_binary;
		xs_binary_previous = xs_binary;

		print_inst(sim_p, inst, i_binary, operation_binary, function_binary, xs_binary);

		fprintf(stderr, "%s", PROMPT);
		/*
		 * below process is passed to Lua script
		 */
		// Lua の言語エンジンを初期化
		lua_State *lua = luaL_newstate();
		// Lua のライブラリを使えるようにする
		luaL_openlibs(lua);
		// Lua のスクリプトを読み込む
		if( luaL_loadfile(lua, "./debugger.lua") || lua_pcall(lua, 0, 0, 0) ) {
			fprintf(stderr, "debugger.luaを開けませんでした\n");
			fprintf(stderr, "error : %s\n", lua_tostring(lua, -1) );
			return 1;
		}
		/*
		 * call the function 'interpret' in Lua Script
		 */
		 //add関数をスタックに積む
		lua_getglobal(lua, "interpret");
		if(lua_pcall(lua, 0, 2, 0) != 0) {
			fprintf(stderr, "関数呼び出し失敗\n");
			fprintf(stderr, "error : %s\n", lua_tostring(lua, -1) );
			return 1;
		}

		int ope = 0;
		int arg = 0;

		if(lua_isnumber(lua, -1)){
			arg = lua_tointeger(lua, -1);
			lua_pop(lua,1); //戻り値をポップ
		}

		if(lua_isnumber(lua, -1)){
			ope = lua_tointeger(lua, -1);
			lua_pop(lua,1); //戻り値をポップ
		}

		fprintf(stderr, "ope == %d, arg == %d\n", ope, arg);
		
		if(ope == 1){
			/*
			 * next
			 */
			break;
		}

		if(ope == 2){
			/*
			 * continue
			 */
			is_running = 1;
			break;
		}

		if(ope == 3){
			/*
			 * break
			 */
			if(breakpoint[arg]){
				continue;
			}
			breakpoint[arg] = 1;
			breakpoint_cnt++;
			continue;
		}

		if(ope == 4){
			/*
			 * dump
			 */
			if(arg == 0){
				print_reg(sim_p);
			}else if(arg == 1){
				print_f_reg(sim_p);
			}else if(arg == 2){
				print_reg(sim_p);
				print_f_reg(sim_p);
			}
			continue;
		}

		continue;
	}
	return simulate_inst(sim_p, inst, i_binary, operation_binary, function_binary, xs_binary);
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
	print_inst(&sim_previous, inst_previous, i_binary_previous, operation_binary_previous, function_binary_previous, xs_binary_previous);
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

