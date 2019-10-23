#include "TSP.h"
#include <ilcplex/cplex.h>
#include <time.h>
#pragma warning(disable : 4996)

/*---------------------------DEFINITION OF METHODS---------------------------------------*/
void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);
void plot_gnuplot(instance *inst);
int xpos_compact(int i, int j, instance *inst);
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
	char* instances[] = { "ulysses16.tsp",
						"ulysses22.tsp" };
	
	//parse_command_line(argc, argv, &inst);									//keep the arguments of the command line
	char name_file[100]="";
	for (int i = 0; i < 2; i++) {
		
		strcat(name_file, "C:\\Users\\marco\\Documents\\RO2\\");
		strcat(name_file, instances[i]);
		printf("%s\n", name_file);
		strcpy(inst.input_file, name_file);
		read_input(&inst);														//READ VARIABLES FROM INPUT AND SAVE INTO inst
		if (TSPopt(&inst, i)) print_error(" error within TSPopt()");
		strcpy(name_file, "");
	}


	if(VERBOSE>=200){
		for (int i = 0; i < inst.nnodes; i++) {
			printf("Capitale %d coord x:%.0f coord y:%.0f\n", i + 1, inst.xcoord[i], inst.ycoord[i]);
		}

	}
	if(VERBOSE>=500){
		double dProva = dist(31, 39, &inst);
		printf("Distanza tra 32 e 40 : %.2f\n", dProva);
	}
	

	//plot_gnuplot(&inst);													//PLOT COORDINATES IN GNUPLOT WINDOW
	free_instance(&inst);													//FREE MEMORY OCCUPIED BY instance TSP.h
	

	return 0;
}

/*--------------------------------------------------------*/

	