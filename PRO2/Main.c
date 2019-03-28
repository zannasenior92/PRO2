#include "TSP.h"
#include <ilcplex/cplex.h>
#pragma warning(disable : 4996)

/*---------------------------DEFINITION OF METHODS---------------------------------------*/
void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);
void plot_gnuplot(instance *inst);
int xpos(int i, int j, instance *inst);
int TSPopt(instance *inst);
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
	parse_command_line(argc, argv, &inst);									//keep the arguments of the command line
	read_input(&inst);														//READ VARIABLES FROM INPUT AND SAVE INTO inst
	printf("Input usato: %s\n", inst.input_file_name);


	if(VERBOSE>=200){
		printf("Il file di input e': %s\n", inst.input_file);
		for (int i = 0; i < inst.nnodes; i++) {
			printf("Capitale %d coord x:%.0f coord y:%.0f\n", i + 1, inst.xcoord[i], inst.ycoord[i]);
		}

	}

	if (TSPopt(&inst)) print_error(" error within TSPopt()");
	plot_gnuplot(&inst);													//PLOT COORDINATES IN GNUPLOT WINDOW
	free_instance(&inst);													//FREE MEMORY OCCUPIED BY instance TSP.h
	return 0;
}






/*--------------------------------------------------------*/