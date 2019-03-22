#include "TSP.h"
#include <time.h>													//TO MESURE PROGRAM TIME
#include <ilcplex/cplex.h>
#pragma warning(disable : 4996)

/*---------------------------DEFINITION OF METHODS---------------------------------------*/
void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);
void plot_gnuplot(instance *inst);
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
	
	double total_time;														
	clock_t start_t, end_t;													//START TIME
	start_t = clock();

	instance inst;															//CREATE VARIABLE inst OF TYPE instance

	
	parse_command_line(argc, argv, &inst);									//keep the arguments of the command line
	printf("Il file di input e': %s\n", inst.input_file);
	read_input(&inst);														//READ VARIABLES FROM INPUT AND SAVE INTO inst

	printf("Input usato: %s\n", inst.input_file_name);

	if(VERBOSE>=200){
		for (int i = 0; i < inst.nnodes; i++) {
			printf("Capitale %d coord x:%.0f coord y:%.0f\n", i + 1, inst.xcoord[i], inst.ycoord[i]);
		}

	}
	if(VERBOSE>=500){
		double dProva = dist(31, 39, &inst);
		printf("Distanza tra 32 e 40 : %.2f\n", dProva);
	}

	if (TSPopt(&inst)) print_error(" error within TSPopt()");
	plot_gnuplot(&inst);													//PLOT COORDINATES IN GNUPLOT WINDOW
	free_instance(&inst);													//FREE MEMORY OCCUPIED BY instance TSP.h
	
	
	end_t = clock();															//FINAL TIME
	total_time = ((double)(end_t - start_t)) / CLOCKS_PER_SEC;
	if (VERBOSE >= 200)
	{
		printf("The program time is: %f (s)", total_time);
	}

	return 0;
}






/*--------------------------------------------------------*/