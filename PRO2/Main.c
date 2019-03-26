#include "TSP.h"
#include <ilcplex/cplex.h>
#include <stdio.h>
#pragma warning(disable : 4996)

/*---------------------------DEFINITION OF METHODS---------------------------------------*/
void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);
void plot_gnuplot(instance *inst);
//void plot_edge(instance *inst);
int TSPopt(instance *inst);
double dist(int i, int j, instance *inst);
void print_error(const char *err) { printf("\n\n ERROR: %s \n\n", err); fflush(NULL); exit(1); } 
void free_instance(instance *inst) {
	free(inst->xcoord);
	free(inst->ycoord);
	free(inst->choosen_edge);
}
/*---------------------------------------------------------------------------------------*/


/*----------------------------------------MAIN-------------------------------------------*/
int main(int argc, char **argv) {

	/*------------------------VERBOSE FROM TERMINAL INPUT--------------------------------*/
	/*
	int VERBOSE = 0;
	printf("SET VERBOSE LEVEL: ");
	scanf("%d",&VERBOSE);
	*/
	/*-----------------------------------------------------------------------------------*/

	instance inst; //we create the variable inst of type instance

	parse_command_line(argc, argv, &inst);											//keep the arguments of the command line
	read_input(&inst);																//READ VARIABLES FROM INPUT AND SAVE INTO inst

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
	plot_gnuplot(&inst);															//PLOT COORDINATES IN GNUPLOT WINDOW
	free_instance(&inst);															//FREE MEMORY OCCUPIED BY instance TSP.h
	
	return 0;
}
