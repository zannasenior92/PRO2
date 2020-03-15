/*--------------------------TSP RESOLVE & CREATE THE MODEL---------------------------*/
#include "TSP.h"
#include <ilcplex/cplex.h>
#include <time.h>

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void plot_gnuplot(instance *inst);
void select_and_build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFlow1(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFischetti(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);
static int CPXPUBLIC add_SEC_lazy(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p);
int myseparation(instance *inst, double *xstar, CPXCENVptr env, void *cbdata, int wherefrom);
int xpos(int i, int j, instance *inst);
int xpos_compact(int i, int j, instance *inst);
void print_error(const char *err);
void selected_edges(instance *inst);
void reset_lower_bound(instance *inst, CPXENVptr env, CPXLPptr lp);
void start_sol(instance *inst);
void update_choosen_edge(instance* inst);
double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, double prob, double opt);
void hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int seed, double prob);
double nearest_neighborhood(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node, int seed);
double two_opt(instance *inst, CPXENVptr env, CPXLPptr lp);
double three_opt(instance *inst, CPXENVptr env, CPXLPptr lp);
double vns(instance *inst, CPXENVptr env, CPXLPptr lp);
void genetic_alg(instance *inst, CPXENVptr env, CPXLPptr lp);
int cost_alg(instance* inst);

/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	inst->compact = 0;
	int error;
	double opt_heu, opt_current;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	build_model(inst, env, lp);
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);							//to visualize in video
	FILE* log = CPXfopen("log.txt", "w");
	

	/*------------------------HARD FIXING WITH LAZYCALLBACK--------------------------*/
	inst->ncols = CPXgetnumcols(env, lp);

	//----START WITH TRIVIAL INITIAL SOLUZION 1->2->3-...->n-1->1
	inst->best_sol= (double*)calloc(inst->ncols, sizeof(double));
	inst->choosen_nodes = (int *)malloc(inst->nnodes * sizeof(int));

	double cost, min_cost;
	min_cost = INFINITY;
	/************************************************************************************/
	/************************************************************************************/
	/*GENETICO*/
	genetic_alg(inst, env, lp);
	/************************************************************************************/
	
	
	/*-------PRINT INITIAL SOLUTION--------*/
	selected_edges(inst);
	
	
	CPXfclose(log);																//CLOSE LOG FILE
	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT------------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}



/*------------------------ADD SUBTOUR ELIMINATION CONSTRAINTS------------------------*/

static int CPXPUBLIC add_SEC_lazy(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p) {
	*useraction_p = CPX_CALLBACK_DEFAULT;			//I say that I haven' t done anything in CPLEX
	instance* inst = (instance *)cbhandle; 			//casting of cbhandle to have the instance

	/*-------------GET XSTAR SOLUTION--------------------------*/
	double *xstar = (double*)calloc(inst->ncols, sizeof(double));

	/*--------------CALL THE CALLBACK--------------------------------------------------------------------------*/
	if (CPXgetcallbacknodex(env, cbdata, wherefrom, xstar, 0, inst->ncols - 1)) print_error("Error in callback");

	/*APPLY CUT SEPARATOR-ADD CONSTRAINTS FOR EVERY CONNECTED COMPONENT AND RETURN NUMBER OF ADDED CONSTRAINTS*/
	int ncuts = myseparation(inst, xstar, env, cbdata, wherefrom);

	free(xstar);//Free space in xstar


	if (ncuts >= 1) {
		*useraction_p = CPX_CALLBACK_SET; 		// tell CPLEX that cuts have been created
	}
	return 0;
}


int cost_alg(instance* inst) {
	double cost = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i+1; j < inst->nnodes; j++) {
			if (inst->best_sol[xpos(i, j, inst)] > TOLERANCE) {
				cost += dist(i, j, inst);
			}
		}
	}
	return cost;
}