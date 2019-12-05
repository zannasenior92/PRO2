#include "TSP.h"
#include <ilcplex/cplex.h>
#pragma warning(disable : 4996)

/*---------------------------DEFINITION OF METHODS---------------------------------------*/
void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);
void plot_gnuplot(instance *inst);
void plot_final_gnuplot(instance *inst);
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
	
	instance inst;		
	char* instances[] = { "kroB150.tsp","pr107.tsp","pr299.tsp","pr76.tsp"
		//"rd400.tsp","st70.tsp","u159.tsp","u574.tsp","u724.tsp","ulysses16.tsp","ulysses22.tsp"
	};

	/*"a280.tsp","ali535.tsp","att48.tsp","att532.tsp","berlin52.tsp","bier127.tsp",
		"burma14.tsp","ch130.tsp",	"ch150.tsp","d198.tsp","d493.tsp","d657.tsp","eil51.tsp","eil76.tsp",
		"eil101.tsp","pr144.tsp","pr152.tsp","pr226.tsp","pr264.tsp","pr299.tsp","pr439.tsp","rat99.tsp","rat195.tsp","rat575.tsp","rat783.tsp",
	"rd100.tsp",
	"fl417.tsp","gil262.tsp","gr202.tsp",	"gr229.tsp","gr431.tsp","kroA100.tsp",
		"kroA150.tsp","kroA200.tsp","kroB100.tsp","kroB150.tsp","kroB200.tsp","kroC100.tsp",
	"kroD100.tsp","kroE100.tsp","lin105.tsp","lin318.tsp","p654.tsp","pcb442.tsp","pr76.tsp",
	"pr107.tsp","pr124.tsp","pr136.tsp"
	*/
	char name_file[100] = "";
	for (int i = 0; i < sizeof(instances) / sizeof(instances[0]); i++) {

		strcat(name_file, "C:\\Users\\marco\\Documents\\RO2\\");
		strcat(name_file, instances[i]);
		printf("%s\n", name_file);
		strcpy(inst.input_file, name_file);
		read_input(&inst);														//READ VARIABLES FROM INPUT AND SAVE INTO inst
		if (TSPopt(&inst, i+100)) print_error(" error within TSPopt()");
		strcpy(name_file, "");
	}												//PLOT COORDINATES IN GNUPLOT WINDOW
	free_instance(&inst);													//FREE MEMORY OCCUPIED BY instance TSP.h
	return 0;
}






/*--------------------------------------------------------*/