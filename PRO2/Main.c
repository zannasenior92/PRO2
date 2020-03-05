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
		//0 "att532.tsp"
		//1 "ali535.tsp",
		//2"d493.tsp",	 
		//3 "d657.tsp" TO DO
		/* 4"fl417.tsp",
		5 "gr431.tsp",
		6 "lin318.tsp",
		7 "p654.tsp"
		
		
		
		"rat575.tsp",
		"u574.tsp",
		"u724.tsp"*/

		/*"rd400.tsp",
		"pr439.tsp", 
		"rat575.tsp",
		"pcb442.tsp"

		"ali535.tsp",
		"d493.tsp",
		"fl417.tsp",
		"gr431.tsp",
		"lin318.tsp",
		"p654.tsp", DA FARE
		"rd400.tsp",
		"pr439.tsp",
		"rat575.tsp",
		"pcb442.tsp",*/
		"rat575.tsp",
		"u574.tsp",
		"u724.tsp",
		"p654.tsp"
	};

	char name_file[100] = "";
	for (int i = 0; i < sizeof(instances) / sizeof(instances[0]); i++) {

		strcat(name_file, "C:\\Users\\marco\\Documents\\RO2\\");
		strcat(name_file, instances[i]);
		printf("%s\n", name_file);
		strcpy(inst.input_file, name_file);
		read_input(&inst);														//READ VARIABLES FROM INPUT AND SAVE INTO inst
		if (TSPopt(&inst, i+10)) print_error(" error within TSPopt()");
		strcpy(name_file, "");
	}
	free_instance(&inst);													//FREE MEMORY OCCUPIED BY instance TSP.h
	return 0;
}






/*--------------------------------------------------------*/