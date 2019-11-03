#include "TSP.h"
#include <time.h>													//TO MESURE PROGRAM TIME
#include <ilcplex/cplex.h>
#pragma warning(disable : 4996)

/*---------------------------DEFINITION OF METHODS---------------------------------------*/
void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);
void plot_gnuplot(instance *inst);
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
	char* instances[] = { "att48.tsp", "berlin52.tsp", "burma14.tsp","eil51.tsp","eil76.tsp","eil101.tsp",
		"rat99.tsp","ulysses16.tsp","ulysses22.tsp"
	};

	/*"a280.tsp","ali535.tsp","att532.tsp","bier127.tsp","ch130.tsp",
	"ch150.tsp","d198.tsp","d493.tsp","d657.tsp","fl417.tsp","gil262.tsp","gr202.tsp",
	"gr229.tsp","gr431.tsp","kroA100.tsp","kroA150.tsp","kroA200.tsp","kroB100.tsp","kroB150.tsp","kroB200.tsp","kroC100.tsp",
	"kroD100.tsp","kroE100.tsp","lin105.tsp","lin318.tsp","p654.tsp","pcb442.tsp","pr76.tsp","pr107.tsp","pr124.tsp","pr136.tsp",
	"pr144.tsp","pr152.tsp","pr226.tsp","pr264.tsp","pr299.tsp","pr439.tsp","rat195.tsp","rat575.tsp","rat783.tsp",
	"rd100.tsp","rd400.tsp","st70.tsp","u159.tsp","u574.tsp","u724.tsp",*/

	//parse_command_line(argc, argv, &inst);									//keep the arguments of the command line
	char name_file[100] = "";
	for (int i = 0; i < sizeof(instances) / sizeof(instances[0]); i++) {

		strcat(name_file, "C:\\Users\\marco\\Documents\\RO2\\");
		strcat(name_file, instances[i]);
		printf("%s\n", name_file);
		strcpy(inst.input_file, name_file);
		read_input(&inst);														//READ VARIABLES FROM INPUT AND SAVE INTO inst
		if (TSPopt(&inst, i)) print_error(" error within TSPopt()");
		strcpy(name_file, "");
	}


	if (VERBOSE >= 200) {
		for (int i = 0; i < inst.nnodes; i++) {
			printf("Capitale %d coord x:%.0f coord y:%.0f\n", i + 1, inst.xcoord[i], inst.ycoord[i]);
		}

	}
	if (VERBOSE >= 500) {
		double dProva = dist(31, 39, &inst);
		printf("Distanza tra 32 e 40 : %.2f\n", dProva);
	}


	//plot_gnuplot(&inst);													//PLOT COORDINATES IN GNUPLOT WINDOW
	free_instance(&inst);													//FREE MEMORY OCCUPIED BY instance TSP.h


	return 0;
}