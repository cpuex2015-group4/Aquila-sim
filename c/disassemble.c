#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include "./utils.h"
#include "./simulator.h"
#include "./debugger.h"

char* option_string(int option)
{
	switch(option){
		case 0:
			return "";
			break;
		case 1:
			return ".asb";
			break;
		case 2:
			return ".neg";
			break;
		default:
			return "";
	}
}

char* jump_option_string(int jump_option)
{
	switch(jump_option){
		case 0:
			return "";
			break;
		case 1:
			return "al";
			break;
		default:
			return "";
	}
}

int disassemble(instruction inst, unsigned char i_binary, unsigned char operation_binary, unsigned char function_binary, unsigned char xs_binary)
{
	unsigned char opt_binary = get_binary_unsigned(inst, 2, 4);
	operands ops; 
	if(i_binary == 0 && operation_binary == 0 && xs_binary == 0){
		/*
		 * Format X
		 */
		ops = decode_X(inst);
		switch(function_binary){
			case 0:
				fprintf(stderr, "hlt\n");
				break;
			case 1:
				fprintf(stderr, "nop\n");
				break;
			case 2:
				fprintf(stderr, "in\n");
				break;
			case 3:
				fprintf(stderr, "out\n");
				break;
			case 4:
				fprintf(stderr, "itof\n");
				break;
			case 5:
				fprintf(stderr, "ftoi\n");
				break;
		}
		return 1;
	}

	if(i_binary == 0 && opt_binary == 3){
		/*
		 * Format B
		 */
		ops = decode_B(inst);
		switch(ops.bit_image){
			case 0:
				if(!ops.rorf){
					fprintf(stderr, "beq %s,%s,%d\n", REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], ops.imm);
				}else{
					fprintf(stderr, "beq.s %s,%s,%d\n", REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], ops.imm);
				}
				break;
			case 1:
				if(!ops.rorf){
					fprintf(stderr, "blt %s,%s,%d\n", REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], ops.imm);
				}else{
					fprintf(stderr, "blt.s %s,%s,%d\n", REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], ops.imm);
				}
				break;
			case 2:
				if(!ops.rorf){
					fprintf(stderr, "ble %s,%s,%d\n", REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], ops.imm);
				}else{
					fprintf(stderr, "ble.s %s,%s,%d\n", REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], ops.imm);
				}
				break;
		}
		return 1;
	}

	if(i_binary == 0){
		/*
		 * Format R
		 */
		ops = decode_R(inst);
		if(ops.bit_image == 0){
			if(!ops.rorf){
				fprintf(stderr, "add%s %s,%s,%s\n", option_string(ops.opt), REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], REGISTERS_G[ops.t_idx]);
			}else{
				fprintf(stderr, "add.s%s %s,%s,%s\n", option_string(ops.opt), REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], REGISTERS_F[ops.t_idx]);
			}
		}else if(ops.bit_image == 1){
			if(!ops.rorf){
				fprintf(stderr, "sub%s %s,%s,%s\n", option_string(ops.opt), REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], REGISTERS_G[ops.t_idx]);
			}else{
				fprintf(stderr, "sub.s%s %s,%s,%s\n", option_string(ops.opt), REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], REGISTERS_F[ops.t_idx]);
			}
		}else if(ops.rorf == 1 && ops.bit_image == 2){
			fprintf(stderr, "mul.s%s %s,%s,%s\n", option_string(ops.opt), REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], REGISTERS_F[ops.t_idx]);
		}else if(ops.rorf == 1 && ops.bit_image == 3){
			fprintf(stderr, "div.s%s %s,%s,%s\n", option_string(ops.opt), REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], REGISTERS_F[ops.t_idx]);
		}else if(ops.rorf == 0 && ops.bit_image == 2){
			fprintf(stderr, "jr%s %s\n", jump_option_string(ops.opt), REGISTERS_G[ops.t_idx]);
		}
		return 1;
	}

	if(i_binary == 1){
		/*
		 * Format I
		 */
		ops = decode_I(inst);
		if(ops.opt == 3){
			if(ops.bit_image == 0){
				if(!ops.rorf){
					fprintf(stderr, "ld %s,%s,%d\n", REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], ops.imm);
				}else{
					fprintf(stderr, "ld.s %s,%s,%d\n", REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], ops.imm);
				}
			}else if(ops.bit_image == 1){
				if(!ops.rorf){
					fprintf(stderr, "st %s,%s,%d\n", REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], ops.imm);
				}else{
					fprintf(stderr, "st.s %s,%s,%d\n", REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], ops.imm);
				}
			}else if(ops.rorf == 0 && ops.bit_image == 2){
				fprintf(stderr, "sll %s,%s,%s\n", REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], REGISTERS_G[ops.t_idx]);
			}else if(ops.rorf == 0 && ops.bit_image == 3){
				fprintf(stderr, "srl %s,%s,%s\n", REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], REGISTERS_G[ops.t_idx]);
			}else if(ops.rorf == 1 && ops.bit_image == 2){
				fprintf(stderr, "inv.s %s,%s\n", REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx]);
			}else if(ops.rorf == 1 && ops.bit_image == 3){
				fprintf(stderr, "sqrt.s %s,%s\n", REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx]);
			}
		}else{
			if(ops.bit_image == 0){
				if(!ops.rorf){
					fprintf(stderr, "addi%s %s,%s,%d\n", option_string(ops.opt), REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], ops.imm);
				}else{
					fprintf(stderr, "addi.s%s %s,%s,%d\n", option_string(ops.opt), REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], ops.imm);
				}
			}else if(ops.bit_image == 1){
				if(!ops.rorf){
					fprintf(stderr, "subi%s %s,%s,%d\n", option_string(ops.opt), REGISTERS_G[ops.d_idx], REGISTERS_G[ops.s_idx], ops.imm);
				}else{
					fprintf(stderr, "subi.s%s %s,%s,%d\n", option_string(ops.opt), REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], ops.imm);
				}
			}else if(ops.rorf == 1 && ops.bit_image == 2){
					fprintf(stderr, "muli.s%s %s,%s,%d\n", option_string(ops.opt), REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], ops.imm);
			}else if(ops.rorf == 1 && ops.bit_image == 3){
					fprintf(stderr, "divi.s%s %s,%s,%d\n", option_string(ops.opt), REGISTERS_F[ops.d_idx], REGISTERS_F[ops.s_idx], ops.imm);
			}else if(ops.rorf == 0 && ops.bit_image == 2){
					fprintf(stderr, "j%s %d\n", jump_option_string(ops.opt), ops.imm);
			}
		}
		return 1;
	}
	return -1;
}
