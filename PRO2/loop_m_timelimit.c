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


void loop_method_with_timelimit(CPXENVptr env, CPXLPptr lp, instance *inst) {

	int done = 0;
	time_t timelimit_max = time(NULL) + 3600;
	double ottimo;
	int resolved = 0;
	double timelimit = 10;															//SET TIMELIMIT 
	//printf("timelimit: %f second\n", timelimit);
	int resolved_in_time = 0;	//1 se è stato risolto il modello nel tempo e non è uscito per timelimit
	while ((!done) && (time(NULL) < timelimit_max)) {
		if (CPXsetdblparam(env, CPX_PARAM_TILIM, timelimit)) print_error("Error on setting parameter timelimit");

		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");

		int status = CPXgetstat(env, lp);											//GET STATUS END
		//printf("Status=%d\n", status);
		/*-------------------(108)STATUS ENDED BY TIMELIMIT, INFEASIBLE SOLUTION FOUNDED------------*/
		if (status == CPXMIP_TIME_LIM_INFEAS) {
			timelimit *= 2;
			//if (CPXsetdblparam(env, CPX_PARAM_TILIM, timelimit)) print_error("Error on setting parameter");
			printf("No solution, double the timelimit=%f\n", timelimit);
			add_SEC(env, lp, inst);
			if (inst->sec == 0) {
				done = 1;
			}

		}
		/*------------------------(103)STATUS ENDED BY IMPOSSIBLE SOLUTION-------------------------*/
		else if (status == CPXMIP_INFEASIBLE) {
			printf("Impossible solution!\n");
			exit(0);
		}

		/*---------------(107)STATUS ENDED BY TIMELIMIT BUT INTEGER SOLUTION FOUNDED----------------*/
		else if (status == CPXMIP_TIME_LIM_FEAS) {
			int ncols = CPXgetnumcols(env, lp);
			inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
			if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
			if (CPXgetobjval(env, lp, &ottimo)) print_error("Error getting optimal value");

			//printf("Best solution found(status ended by timelimit):%.0f\n", ottimo);
			if (kruskal_sst(env, lp, inst) == inst->nnodes) {
				if (CPXsetdblparam(env, CPX_PARAM_TILIM, timelimit * 2)) print_error("Error on setting parameter timelimit*2(107)");
				//printf("Ha n=%d comp conn\n", kruskal_sst(env, lp, inst));
			}
			else {
				if (kruskal_sst(env, lp, inst) == 1) {
					/*resolved = 1;
					done = 1;
					*/
					continue;
				}

				else {
					add_SEC(env, lp, inst);
					if (inst->sec == 0) {
						done = 1;
					}
				}
			}

		}
		/*------------(101)FOUNDED INTEGER SOLUTION OR (102)INTEGER SOLUTION WITH TOLERANCE---------------*/
		else if ((status == CPXMIP_OPTIMAL) || (status == CPXMIP_OPTIMAL_TOL)) {
			int ncols = CPXgetnumcols(env, lp);
			inst->best_sol = (double *)calloc(ncols, sizeof(double));
			if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
			if (CPXgetobjval(env, lp, &ottimo)) print_error("Error getting optimal value");
			if (kruskal_sst(env, lp, inst) == 1) {
				done = 1;
			}
			else {
				add_SEC(env, lp, inst);
			}
		}
		else {
			printf("------------------------------------STATUS NON GESTITO: %d\n", status);
			exit(0);
		}
	}
	
	/*-------------------------VERY USEFUL ITERATION WITHOUT TIMELIMIT----------------------*/
	if (timelimit_max - time(NULL) > 0) {
		if (CPXsetdblparam(env, CPX_PARAM_TILIM, timelimit_max - time(NULL))) print_error("Error on setting parameter");
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");
	}

}
