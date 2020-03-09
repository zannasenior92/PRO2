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
	
	inst->best_sol= (double*)calloc(inst->ncols, sizeof(double));
	double cost;
	opt_heu = INFINITY;
	double *minimum_solution = (double*)calloc(inst->ncols, sizeof(double));
	int start_node = 0;
	for (int i = 0; i < 20; i++) {
		for (int j = 0; j < inst->nnodes; j++) {
			inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
			cost = nearest_neighborhood_GRASP(inst, env, lp, j);
			if (cost < opt_heu) {
				opt_heu = cost;
				minimum_solution = inst->best_sol;
			}
		}
	}
	
	printf("\nBest Initial Cost After Nearest Neighborhood %f\n", opt_heu);
	inst->best_sol = minimum_solution;
	
	opt_current = opt_heu;

	double min_2opt = 0;
	time_t timelimit = time(NULL) + 3600;
	printf("--------------------2-OPT-------------------\n");
	//ESEGUO 2-OPT per trovare minimo locale
	while (time(NULL) < timelimit) {
		
		double delta = two_opt(inst, env, lp);
		opt_current += delta;
		
		if (delta == 0.0) {
			min_2opt = opt_current;
			break;
		}
		

	}
	printf("New objective function: %f\n", opt_current);
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

	fprintf(output, "2-OPT,%s,%f,123456", inst->input_file_name, opt_current);

	fclose(output);
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
