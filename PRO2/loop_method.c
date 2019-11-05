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
	/*FILE * gnuplotPipe = _popen("C:/gnuplot/bin/gnuplot.exe", "w");	//"-persistent" KEEPS THE PLOT OPEN EVEN AFTER YOUR C PROGRAM QUIT
	fprintf(gnuplotPipe, "%s \n", "set terminal windows 1");
	fprintf(gnuplotPipe, "%s \n", "set title \"Punti TSP ");*/
	if (CPXsetdblparam(env, CPX_PARAM_TILIM, 900)) print_error("Error on setting parameter");

	int done = 0;

	while (!done) 
	{
		
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
		if (CPXsetlogfile(env, log)) print_error("Error in log file");
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
		/*update_choosen_edges(inst);
		add_edge_to_file(inst);
		fprintf(gnuplotPipe, "%s \n", "plot 'connected_components.txt' with lp ls 1 lc variable, ''  with point pointtype 7 lc variable");
		/*----------------FOR SMALL INSTANCES--------------*/
		/*if(inst->nnodes< 100)	Sleep(500);
		fflush(gnuplotPipe);
		printf("Componente connesse %d\n", inst->n_connected_comp);*/
	}
	//_pclose(gnuplotPipe);

	/*-------------------------------------------------------------------------------*/
	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	double opt_val;																		//VALUE OPTIMAL SOL
	if (CPXgetobjval(env, lp, &opt_val)) print_error("Error getting optimal value");;	//OPTIMAL SOLUTION FOUND
	//printf("Object function optimal value is: %.0f\n", opt_val);
	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT------------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}