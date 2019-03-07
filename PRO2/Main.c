#include "TSP.h"

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
	double dProva = dist(0,1, &inst);
	printf("Distanza tra 0 e 1 : %.2f\n", dProva);
	plot_coord(&inst);
	free_instance(&inst);
	
	return 0;
}




