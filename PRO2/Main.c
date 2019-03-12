#include "TSP.h"
#include <ilcplex/cplex.h>
//#include "comandiGnuplot.c" //INCLUDE LA FUNZIONE PER STAMPARE IL GRAFICO
#pragma warning(disable : 4996)


void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);
void plot_coord(instance *inst);
double dist(int i, int j, instance *inst);
void free_instance(instance *inst){
	free(inst->xcoord);
	free(inst->ycoord);
}
void print_error(const char *err) { printf("\n\n ERROR: %s \n\n", err); fflush(NULL); exit(1); } 

//-------------------------MAIN-------------------------------
int main(int argc, char **argv) {
	//we create the variable inst of type instance
	instance inst;

	//we keep the arguments of the command line
	parse_command_line(argc, argv, &inst);
	printf("Il file di input e': %s\n", inst.input_file);
	//we use the name of the variale related to the instance to access on the field of the instance
	read_input(&inst);


	for (int i = 0; i < inst.nnodes; i++) {
		printf("Capitale %d coord x:%.0f coord y:%.0f\n", i + 1, inst.xcoord[i], inst.ycoord[i]);
	}
	if(VERBOSE>=100){
		double dProva = dist(31, 39, &inst);
		printf("Distanza tra 32 e 40 : %.2f\n", dProva);
	}

	plot_coord(&inst);
	if (TSPopt(&inst)) print_error(" error within TSPopt()");
	free_instance(&inst);//libero la memoria occupata dall'istanza creata nel file TSP
	
	return 0;
}




