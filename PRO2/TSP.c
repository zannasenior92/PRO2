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
double nearest_neighborhood(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);
void hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int seed, double prob);
double nearest_neighborhood_GRASP(instance *inst, int start_node);
double two_opt(instance *inst);
double tabu_search(instance *inst);
/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	inst->compact = 0;
	int error;
	double opt_heu, opt_current;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	build_model(inst, env, lp);
	//select_and_build_model(inst, env, lp);
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);									//Per visualizzare a video
	FILE* log = CPXfopen("log.txt", "w");
	

	
	inst->ncols = CPXgetnumcols(env, lp);
	//START WITH TRIVIAL INITIAL SOLUZION 1->2->3-...->n-1->1

	inst->best_sol= (double*)calloc(inst->ncols, sizeof(double));
	double cost, min_cost;
	min_cost = INFINITY;
	double *minimum_solution = (double*)calloc(inst->ncols, sizeof(double));
	int start_node = 0;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < inst->nnodes; j++) {
			inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
			cost = nearest_neighborhood_GRASP(inst, j);
			if (cost < min_cost) {
				min_cost = cost;
				minimum_solution = inst->best_sol;
			}
		}
	}

	printf("\nBest Initial Cost After Nearest Neighborhood %f\n", min_cost);
	inst->best_sol = minimum_solution;
	
	/*-------PRINT INITIAL SOLUTION--------*/
	selected_edges(inst);
	
	/*---------------------SET INITIAL OPTIMAL VALUE TO INFINITE---------------------*/

	opt_current = min_cost;
	
	int local_minimum = 0;
	double min_2opt = 0;
	time_t timelimit = time(NULL) + 300;
	printf("--------------------2-OPT-------------------\n");
	/*----------------RUN 2-OPT TO FIND LOCAL MINIMUM--------------*/
	while (time(NULL) < timelimit) {
		
		double delta = two_opt(inst);
		printf("Delta: %f\n", delta);
		opt_current += delta;
		printf("New objective function: %f\n", opt_current);
		if (delta == 0.0) {
			min_2opt = opt_current;
			break;
		}
		

	}
	/*------------------------TABU SEARCH---------------------*/
	printf("--------------------TABU-------------------\n");

	timelimit = time(NULL) + 300;
	int size = 0;
	inst->tabu_list = (int*)calloc(200, sizeof(int));
	inst->tabu_index = 0;
	inst->tabu_flag = 0;
	inst->tabu_list_complete = 0;
	double best_solution = INFINITY;
	int* edges = (int*)calloc(inst->ncols, sizeof(int));
	while (time(NULL) < timelimit) {

		double delta = tabu_search(inst);

		if (VERBOSE > 200) { printf("Delta: %f\n", delta); }

		opt_current += delta;
		printf("New objective function: %f\n\n", opt_current);
		if (opt_current < best_solution) {
			best_solution = opt_current;
			//WRITE ARCS OF SOLUTION INTO AN ARRAY
			for (int i = 0; i < inst->ncols; i++) {
				edges[i] = inst->best_sol[i];
			}
		}
	}

	//WRITE BEST SOLUTION FOUNDED IN THE INSTANCE
	for (int k = 0; k < inst->ncols; k++) {
		inst->best_sol[k] = edges[k];
	}
		
	/*-------------------------------------------------------------------------------*/
	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	printf("Starting object function is: %.0f\n", min_cost);
	printf("Object function after 2-OPT is: %.0f\n", min_2opt);
	printf("Best object function value is: %.0f\n", best_solution);
	/*---------------PRINT SELECTED EDGES--------------------------------------------*/
	selected_edges(inst);
	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT------------*/
	CPXfclose(log);																//CLOSE LOG FILE
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}