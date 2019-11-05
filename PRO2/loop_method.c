#include "TSP.h"
#include <Windows.h>
#include <time.h>
/*-------------------------------------------LOOP METHOD FOR DEFAULT MODEL---------------------------------------*/


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/

void add_edge_to_file(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst);
int loop_method(CPXENVptr env, CPXLPptr lp, instance *inst, FILE *log);
void update_choosen_edges(instance *inst);
void plot_gnuplot_start(instance *inst, FILE * gnuplotPipe);
void plot_gnuplot(instance *inst, FILE * gnuplotPipe);
int xpos_compact(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);



/*****************************************************************************************************************/
int loop_method(CPXENVptr env, CPXLPptr lp, instance *inst, FILE* log) 
{
	
	int done = 0;

	while (!done) 
	{
		
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
		int ncols = CPXgetnumcols(env, lp);
		inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
		if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
		
		if (kruskal_sst(env, lp, inst) == 1)//VERIFY IF THERE IS ONLY ONE CONNECTED COMPONENT
		{
			done = 1;
		}

		else//--------------------------------OTHERWISE ADD CONSTRAINTS FOR ALL CONNECTED COMPONENT IN THE PROVVISORY TSP SOLUTION
		{
			add_SEC(env, lp, inst);
			
		}
		
	}
	/*-------------------------------------------------------------------------------*/
	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	
	//CPXfreeprob(env, &lp);
	//CPXcloseCPLEX(&env);
	return 0;
}