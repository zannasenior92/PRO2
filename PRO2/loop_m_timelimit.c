/**************************************************************************************************/
/*-------------------------------LOOP METHOD WITH TIMELIMIT---------------------------------------*/
/**************************************************************************************************/

#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void add_edge_to_file(instance *inst);
void update_choosen_edges(instance *inst);
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void print_error(const char *err);
int xpos_compact(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);


void loop_method_with_timelimit(CPXENVptr env, CPXLPptr lp, instance *inst, FILE* log, double *time1) {

	int done = 0;
	double ottimo;
	int resolved = 0;
	double ticks1, ticks2, time3, time4;
	double timelimit = 2;															//SET TIMELIMIT 
	printf("timelimit: %f second\n", timelimit);
	int resolved_in_time = 0;															//1 se � stato risolto il modello nel tempo e non � uscito per timelimit
	while (!done) {

		if (CPXgettime(env, &time3)) print_error("Error getting time\n");				//GET TIME
		if (CPXsetdblparam(env, CPX_PARAM_TILIM, timelimit)) print_error("Error on setting parameter");

		/******************************************************************************************/
		/*------------------------------------USE OF TIMELIMITS-----------------------------------*/
		/******************************************************************************************/

		resolved = 0;																	//UNTIL OUT STATE ISN'T OPTIMAL
		while (!resolved) {

			if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");

			int status = CPXgetstat(env, lp);											//GET STATUS END
			printf("Status=%d\n", status);

			/*-------------------STATUS ENDED BY TIMELIMIT, INFEASIBLE SOLUTION FUNDED------------*/
			if (status == CPXMIP_TIME_LIM_INFEAS) {
				timelimit *= 2;
				if (CPXsetdblparam(env, CPX_PARAM_TILIM, timelimit)) print_error("Error on setting parameter");
				printf("No solution, double the time\n");
				add_SEC(env, lp, inst);
				if (inst->sec == 0) {
					resolved = 1;
					done = 1;
				}

			}
			/*------------------------STATUS ENDED BY IMPOSSIBLE SOLUTION-------------------------*/
			else if (status == CPXMIP_INFEASIBLE) {
				printf("Impossible solution!\n");
				exit(0);
			}

			/*---------------STATUS ENDED BY TIMELIMIT BUT INTEGER SOLUTION FOUNDED----------------*/
			else if (status == CPXMIP_TIME_LIM_FEAS) {
				int ncols = CPXgetnumcols(env, lp);
				inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
				if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
				if (CPXgetobjval(env, lp, &ottimo)) print_error("Error getting optimal value");

				printf("Best solution found(status ended by timelimit):%.0f\n", ottimo);
				if (kruskal_sst(env, lp, inst) == inst->nnodes) {
					if (CPXsetdblparam(env, CPX_PARAM_TILIM, timelimit * 2)) print_error("Error on setting parameter");
					printf("Ha n=%d comp conn\n", kruskal_sst(env, lp, inst));
				}
				else {
					if (kruskal_sst(env, lp, inst) == 1) {

						resolved = 1;
						done = 1;
					}

					else {
						add_SEC(env, lp, inst);
						if (inst->sec == 0) {
							done = 1;
						}
						if (VERBOSE >= 10) {
							printf("Added constraints\n");
						}
					}
					update_choosen_edges(inst);
					add_edge_to_file(inst);
					plot_gnuplot(inst);
				}

			}
			/*------------FOUNDED INTEGER SOLUTION OR INTEGER SOLUTION WITH TOLERANCE---------------*/
			else if ((status == CPXMIP_OPTIMAL) || (status == CPXMIP_OPTIMAL_TOL)) {
				int ncols = CPXgetnumcols(env, lp);
				inst->best_sol = (double *)calloc(ncols, sizeof(double));
				if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
				if (CPXgetobjval(env, lp, &ottimo)) print_error("Error getting optimal value");
				printf("Best solution found(integer or integer with tolerance):%.0f\n", ottimo);
				resolved = 1;
				if (kruskal_sst(env, lp, inst) == 1) {
					done = 1;
					resolved = 1;
					resolved_in_time = 1;
				}

				else {
					add_SEC(env, lp, inst);
					if (VERBOSE >= 10) {
						printf("Added constraints\n");
					}
				}
				update_choosen_edges(inst);
				add_edge_to_file(inst);
				plot_gnuplot(inst);
			}
			else {
				printf("------------------------------------STATUS NON GESTITO: %d\n", status);
				exit(0);
			}
		}

		//if (CPXgetdettime(env, &ticks2)) print_error("Error getting time\n");
		if (CPXgettime(env, &time4)) print_error("Error getting time\n");
		//printf("Ticks=%f\n", ticks2-ticks1);
		printf("Time=%f\n", time4 - time3);

		if (CPXsetlogfile(env, log)) print_error("Error in log file");


	}
	/*-------------------------VERY USEFUL ITERATION WITHOUT TIMELIMIT----------------------*/
	if (CPXsetdblparam(env, CPX_PARAM_TILIM, 1e+75)) print_error("Error on setting parameter");
	if (CPXsetintparam(env, CPX_PARAM_HEURFREQ, 0)) print_error("Error on setting parameter");

	done = 0;
	if (resolved_in_time == 0) {
		while (!done) {
			if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
			if (CPXsetlogfile(env, log)) print_error("Error in log file");
			int ncols = CPXgetnumcols(env, lp);
			inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
			if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
			if (kruskal_sst(env, lp, inst) == 1) {
				done = 1;
			}

			else {
				add_SEC(env, lp, inst);
				if (VERBOSE >= 10) {
					printf("Added constraints without time limits \n");
				}
			}
		}
	}

	int ncols = CPXgetnumcols(env, lp);
	inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
	if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");

	if (VERBOSE >= 200) {
		for (int i = 0; i < ncols - 1; i++) {
			printf("Best %f\n", inst->best_sol[i]);
		}
	}
	
	update_choosen_edges(inst);
	add_edge_to_file(inst);

	/*-------------------------------------------------------------------------------*/
	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	double opt_val;																		//VALUE OPTIMAL SOL
	if (CPXgetobjval(env, lp, &opt_val)) print_error("Error getting optimal value");;	//OPTIMAL SOLUTION FOUND
	printf("Object function optimal value is: %.0f\n", opt_val);
}
