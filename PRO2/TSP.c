/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void select_and_build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFlow1(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFlow2(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFischetti(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst);


/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	
	inst->compact = 0;
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	
	select_and_build_model(inst, env, lp);
	FILE* log = CPXfopen("log.txt", "w");


	/*------------------------TWO POSSIBILITY FOR SOLVE THE MODEL(DEFAULT SEC / OTHER MODEL)-------------------*/
	/***********************************************************************************************************/

	/*-------------------------------------LOOP METHOD FOR DEFAULT MODEL---------------------------------------*/
	if (inst->model_type == 0)
	{

		int done = 0;
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
				add_SEC(env,lp,inst);
				if (VERBOSE >= 100) {
					printf("Aggiunti vincoli\n");
				}
			}
		
		}

		int ncols = CPXgetnumcols(env, lp);
		if(VERBOSE>=200){
			for (int i = 0; i < ncols - 1; i++){
				printf("Best %f\n", inst->best_sol[i]);
			}
		}
		int count = 0;
		int n = 0;
		/*-------------------PRINT SELECTED EDGES(remember cplex tolerance)--------------*/
		if (inst->compact == 1) {
			for (int i = 0; i < inst->nnodes; i++) {
				for (int j = 0; j < inst->nnodes; j++) {
						if (inst->best_sol[xpos_compact(i, j, inst)] > TOLERANCE) {

							if (VERBOSE >= 100) {
								printf("Il nodo (%d,%d) e' selezionato\n", i + 1, j + 1);
							}
							/*--ADD EDGES(VECTOR LENGTH = 2*nnodes TO SAVE NODES OF EVERY EDGE)--*/
							inst->choosen_edge[n] = i;
							inst->choosen_edge[n + 1] = j;
							n += 2;
							count++;
						}
				}
			}
		}
		else {
			for (int i = 0; i < inst->nnodes; i++) {
				for (int j = i+1; j < inst->nnodes; j++) {
					if (inst->best_sol[xpos(i, j, inst)] > TOLERANCE) {

						if (VERBOSE >= 100) {
							printf("Il nodo (%d,%d) e' selezionato\n", i + 1, j + 1);
						}
						/*--ADD EDGES(VECTOR LENGTH = 2*nnodes TO SAVE NODES OF EVERY EDGE)--*/
						inst->choosen_edge[n] = i;
						inst->choosen_edge[n + 1] = j;
						n += 2;
						count++;
					}
				}
			}
		}
		add_edge_to_file(inst);

		if (VERBOSE >= 100) {
			printf("Selected nodes: %d \n", count);
		}
		/*-------------------------------------------------------------------------------*/
		/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
		double *opt_val = 0;																//VALUE OPTIMAL SOL
		if (CPXgetobjval(env, lp, &opt_val)) print_error("Error getting optimal value");;													//OPTIMAL SOLUTION FOUND
		printf("Object function optimal value is: %.0f\n", opt_val);
		/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT------------*/
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env);
		return 0;
	}

	/*--------------------------------RESOLVE WITH OTHER MODEL----------------------------------------*/
	else
	{

		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
		if (CPXsetlogfile(env, log)) print_error("Error in log file");
		int ncols = CPXgetnumcols(env, lp);
		inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
		if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
	}
}


