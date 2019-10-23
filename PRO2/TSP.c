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
void update_choosen_edges(instance *inst);
void add_edge_to_file(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst);
int loop_method(CPXENVptr env, CPXLPptr lp, instance *inst, FILE* log);
int xpos_compact(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void print_error(const char *err);


/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	
	inst->compact = 0;
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
	select_and_build_model(inst, env, lp);
	FILE* log = CPXfopen("log.txt", "w");

	/***********************************************************************************************************/
	/*------------------------TWO POSSIBILITY FOR SOLVE THE MODEL(DEFAULT SEC / OTHER MODEL)-------------------*/
	/***********************************************************************************************************/

	/*-------------------------------------LOOP METHOD FOR DEFAULT MODEL---------------------------------------*/
	if (inst->model_type == 0)
	{
		loop_method(env, lp, inst, log);
	}

	/*-----------------------------------------RESOLVE WITH OTHER MODEL----------------------------------------*/
	else
	{
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
		if (CPXsetlogfile(env, log)) print_error("Error in log file");
		int ncols = CPXgetnumcols(env, lp);
		inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
		if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");

		update_choosen_edges(inst);

		add_edge_to_file(inst);
		/*-------------------------------------------------------------------------------*/
		/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
		double opt_val;																//VALUE OPTIMAL SOL
		if (CPXgetobjval(env, lp, &opt_val)) print_error("Error getting optimal value");;													//OPTIMAL SOLUTION FOUND
		printf("Object function optimal value is: %.0f\n", opt_val);
		/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT------------*/
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env);
		return 0;

	}

}


