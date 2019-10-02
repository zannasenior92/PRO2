/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>
#define CPX_PARAM_EPAGAP = 0.05;


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void select_and_build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFlow1(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFischetti(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst);
void update_choosen_edge(instance* inst);
void loop_method_with_timelimit(CPXENVptr env, CPXLPptr lp, instance *inst, FILE* log, double *time1);
void print_error(const char *err);
void selected_edges(instance *inst);

/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	
	inst->compact = 0;
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	double *time1, *time2;
	if (CPXgettime(env, &time1)) print_error("time error");									//START TIME
	select_and_build_model(inst,env,lp);
	FILE* log = CPXfopen("log.txt", "w");
	
	/*---------------------------------LOOP METHOD WITH TIMELIMIT-------------------*/
	if (inst->model_type==0)
	{
		loop_method_with_timelimit(env, lp, inst, log, &time1);

		/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT-----------*/
		if (CPXgettime(env, &time2)) print_error("time error");

		printf("Total time %f\n", time2 - time1);												//END TIME
		/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT------------*/
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env);
		return 0;
	}
	/*-----------------------------------------RESOLVE WITH OTHER MODEL----------------------------------------*/
	else
	{
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
		if (CPXsetlogfile(env, log)) print_error("Error in log file");
		int ncols = CPXgetnumcols(env, lp);
		inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
		if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");

		selected_edges(inst);
		/*-------------------------------------------------------------------------------*/
		/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
		double opt_val;																		//VALUE OPTIMAL SOL
		if (CPXgetobjval(env, lp, &opt_val)) print_error("Error getting optimal value");;	//OPTIMAL SOLUTION FOUND
		printf("Object function optimal value is: %.0f\n", opt_val);
		/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT------------*/
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env);
		return 0;

	}
}
