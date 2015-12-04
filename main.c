/*
 * USAGE
 * -------------------------
 * /path/to/this/dir/csim -f [objectfile] 
 *
 * OPTION
 * -------------------------
 *  NEEDS ARGUMENT:
 *  -f [objectfile] Essential
 *  -s [signature count] 
 *
 *  NOT NEED ARGUMENT:
 *  -c Instruction Count Flag
 *  -d Debug Flag
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "./simulator.h"
#include "./debugger.h"

int IS_DEBUG = 0;
int INST_CNT = 0;
int SIGNATURE = 0;
int STATISTICS = 0;
unsigned long MEM_SIZE = 100000000000L;

extern void debugger_main(void);

int main(int argc, char* argv[])
{
	FILE* fp_binary = NULL;
	int result;
	while((result=getopt(argc,argv,"cdSf:s:"))!=-1){
		switch(result){
			/* 
			 * Option that does not need arg
			 */
			case 'd':
				main_debugger();
				IS_DEBUG = 1;
				break;

			case 'c':
				fprintf(stderr, "INST CNT ON\n");
				INST_CNT = 1;
				break;

			case 'S':
				STATISTICS = 1;
				break;

			/*
			 * Option that needs arg
			 */
			case 'f':
				if ((fp_binary = fopen(optarg, "rb")) == NULL) {
					fprintf(stderr, "file open error!!\n");
					exit(EXIT_FAILURE);
				}
				if(IS_DEBUG)fprintf(stderr,"Object File : %s\n", optarg);
				break;
			
			case 's':
				SIGNATURE = atoi(optarg);
				fprintf(stderr, "SIGUNATURE = %d\n", SIGNATURE);
				break;

			case '?':
				fprintf(stderr,"unknown\n");
				break;
		}
	}

	simulator *sim = init_sim();
	load_binary(sim, fp_binary);
	simulate(sim);
	fclose(fp_binary);

	if(STATISTICS) {
		FILE *fp = fopen("statistics.csv", "w");
		int i;
		for(i = 0; i < sim->text_size; i++) {
			fprintf(fp, "%d,%d\n", i, sim->called_count_table[i]);
		}
		fclose(fp);
	}

	free_sim(sim);
	return 0;
}
