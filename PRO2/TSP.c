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
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);
double two_opt(instance *inst, CPXENVptr env, CPXLPptr lp);
double tabu_search(instance *inst, CPXENVptr env, CPXLPptr lp);
/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst, int i)
{
	inst->compact = 0;
	int error;
	double opt_heu, opt_current;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	build_model(inst, env, lp);


	inst->ncols = CPXgetnumcols(env, lp);
	//START WITH TRIVIAL INITIAL SOLUZION 1->2->3-...->n-1->1
	//start_sol(inst);
	inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
	double cost, min_cost;
	min_cost = INFINITY;
	double *minimum_solution = (double*)calloc(inst->ncols, sizeof(double));
	int start_node = 0;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < inst->nnodes; j++) {
			inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
			cost = nearest_neighborhood_GRASP(inst, env, lp, j);
			if (cost < min_cost) {
				min_cost = cost;
				minimum_solution = inst->best_sol;
			}
		}
	}


	printf("\nBest Initial Cost After Nearest Neighborhood %f\n", min_cost);
	inst->best_sol = minimum_solution;

	opt_current = min_cost;

	int local_minimum = 0;
	double min_2opt = 0.0;
	time_t timelimit = time(NULL) + 1200;/*
	//ESEGUO 2-OPT per trovare minimo locale
	while (time(NULL) < timelimit) {

		double delta = two_opt(inst, env, lp);
		//printf("Delta: %f\n", delta);
		opt_current += delta;
		//printf("New objective function: %f\n", opt_current);
		if (delta == 0.0) {
			min_2opt = opt_current;
			break;
		}
	}
	if (min_2opt == 0.0) {
		min_2opt = opt_current;
	}
	printf("Objective function 2-opt: %f\n", min_2opt);
	//ESEGUO ORA TABU SEARCH
	//printf("--------------------TABU-------------------\n");
	*/

	timelimit = time(NULL) + 3600;
	int size = 0;
	inst->tabu_list = (int*)calloc(200, sizeof(int));
	inst->tabu_index = 0;
	inst->tabu_flag = 0;
	inst->tabu_list_complete = 0;
	double tabu_opt = opt_current;
	opt_current = opt_current;
	int* edges = (int*)calloc(inst->ncols, sizeof(int));
	while (time(NULL) < timelimit) {

		double delta = tabu_search(inst, env, lp);
		//printf("Delta: %f\n", delta);
		opt_current += delta;
		//printf("New objective function: %f\n\n", opt_current);
		if (opt_current < tabu_opt) {
			tabu_opt = opt_current;
			//scrivo gli archi della soluzione ottima in un array
			for (int i = 0; i < inst->ncols; i++) {
				edges[i] = inst->best_sol[i];
			}
		}
	}

	//scrivo la miglior soluzione trovata nell'instanza
	for (int k = 0; k < inst->ncols; k++) {
		inst->best_sol[k] = edges[k];
	}


	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/

	printf("MINIMUM SOLUTION: %f\n", tabu_opt);
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);

	inst->input_file_name[strlen(inst->input_file_name) - 1] = '\0';
	char out_file[100] = "";
	strcat(out_file, "file");
	char iter[5] = "";
	sprintf(iter, "%d", i);
	strcat(out_file, iter);
	strcat(out_file, ".txt");
	FILE* output = fopen(out_file, "w");

	fprintf(output, "TabuSearch,%s,%f,123456", inst->input_file_name, tabu_opt);

	fclose(output);

	return 0;
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}



/*------------------------ADD SUBTOUR ELIMINATION CONSTRAINTS------------------------*/

static int CPXPUBLIC add_SEC_lazy(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p) {
	*useraction_p = CPX_CALLBACK_DEFAULT;			//Dico che non ho fatto niente 
	instance* inst = (instance *)cbhandle; 			// casting of cbhandle to have the instance

	/*-------------GET XSTAR SOLUTION--------------------------*/
	double *xstar = (double*)calloc(inst->ncols, sizeof(double));

	/*--------------CALL THE CALLBACK--------------------------------------------------------------------------*/
	if (CPXgetcallbacknodex(env, cbdata, wherefrom, xstar, 0, inst->ncols - 1)) print_error("Error in callback");

	/*APPLY CUT SEPARATOR-ADD CONSTRAINTS FOR EVERY CONNECTED COMPONENT AND RETURN NUMBER OF ADDED CONSTRAINTS*/
	int ncuts = myseparation(inst, xstar, env, cbdata, wherefrom);
	//Free space in xstar
	free(xstar);

	if (ncuts >= 1) {
		*useraction_p = CPX_CALLBACK_SET; 		// tell CPLEX that cuts have been created
	}
	return 0;
}
