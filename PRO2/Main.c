#include "TSP.h"
#include <ilcplex/cplex.h>
#pragma warning(disable : 4996)

/*---------------------------DEFINITION OF METHODS---------------------------------------*/
void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);
void plot_gnuplot(instance *inst);
int xpos(int i, int j, instance *inst);
int TSPopt(instance *inst, int i);
double dist(int i, int j, instance *inst);
void print_error(const char *err) { printf("\n\n ERROR: %s \n\n", err); fflush(NULL); exit(1); } 
void free_instance(instance *inst) {
	free(inst->xcoord);
	free(inst->ycoord);
}
/*---------------------------------------------------------------------------------------*/


/*----------------------------------------MAIN-------------------------------------------*/
int main(int argc, char **argv) {
	
	instance inst;															//CREATE VARIABLE inst OF TYPE instance
	inst.model_type = 0;
	char* instances[] = {
		/*
		0 "att532.tsp",
		1 "ali535.tsp",
		2 "d493.tsp",
		3 "fl417.tsp",
		4 "gr431.tsp",
		5 "lin318.tsp",
		6 "rd400.tsp",
		7 "pr439.tsp",*/
		/* to do "d657.tsp"
		8 "pcb442.tsp",
		9 "rat575.tsp",
		10 "u574.tsp",

		11 "u724.tsp",
		12 "p654.tsp",
		13 "d657.tsp"*/
		"ali535.tsp",
		"d493.tsp",
		"fl417.tsp",
		"gr431.tsp"
	};

	char name_file[100] = "";
	for (int i = 0; i < sizeof(instances) / sizeof(instances[0]); i++) {

		strcat(name_file, "C:\\Users\\marco\\Documents\\RO2\\");
		strcat(name_file, instances[i]);
		printf("%s\n", name_file);
		strcpy(inst.input_file, name_file);
		read_input(&inst);														//READ VARIABLES FROM INPUT AND SAVE INTO inst
		if (TSPopt(&inst, i+1)) print_error(" error within TSPopt()");
		strcpy(name_file, "");
	}
	free_instance(&inst);													//FREE MEMORY OCCUPIED BY instance TSP.h
	return 0;
}






/*--------------------------------------------------------*/