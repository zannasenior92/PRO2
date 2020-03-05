/*--------------------------TSP RESOLVE & CREATE THE MODEL---------------------------*/
#include "TSP.h"
#include <ilcplex/cplex.h>


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
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
void reset_lower_bound(instance *inst, CPXENVptr env, CPXLPptr lp);
void hard_fixing(CPXENVptr env, CPXLPptr lp, instance *inst, int seed, double prob);
void start_sol(instance *inst);
void update_choosen_edge(instance* inst);

void fix_nodes_local_branching(CPXENVptr env, CPXLPptr lp, instance *inst, double k);
void delete_local_branching_constraint(CPXENVptr env, CPXLPptr lp);
double loop_local_branching(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, int k, double opt);
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node, int seed);
	

/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst, int i)
{
	int error;
	double opt_heu, opt_current;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 123456);

	build_model(inst, env, lp);
	
	/*------------------------------------USO LOCAL BRANCHING CON LAZYCALLBACK---------------------------------------*/
	inst->ncols = CPXgetnumcols(env, lp);
	/*----TAKE THE BEST INITIAL SOLUTION WITH NEAREST NEIGHBORHOOD GRASP-------------*/
	inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
	double cost, min_cost;
	min_cost = INFINITY;
	double *minimum_solution = (double*)calloc(inst->ncols, sizeof(double));
	int start_node = 0;

	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < inst->nnodes; j++) {
			inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));

			cost = nearest_neighborhood_GRASP(inst, env, lp, j, j);
			if (cost < min_cost) {
				min_cost = cost;
				for (int k = 0; k < inst->ncols; k++) {
					minimum_solution[k] = inst->best_sol[k];

				}
			}
		}
	}
	printf("\nBest Initial Cost After Nearest Neighborhood GRASP %f\n", min_cost);
	for (int k = 0; k < inst->ncols; k++) {
		inst->best_sol[k] = minimum_solution[k];
	}
	/*-------------------------------------------------------------------------------*/

	free(minimum_solution);


	opt_current = min_cost;

	/*------------SETTING OF CALLBACKS--------------*/
	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);								// let MIP callbacks work on the original model
	CPXsetdblparam(env, CPX_PARAM_TILIM, 30);
	CPXsetlazyconstraintcallbackfunc(env, add_SEC_lazy, inst);
	int ncores = 1;
	CPXgetnumcores(env, &ncores);
	CPXsetintparam(env, CPX_PARAM_THREADS, ncores);
	//Setto ottimo soluzione iniziale a infinito
	opt_heu = opt_current;
	
	time_t timelimit1 = time(NULL) + 1200;
	printf("-----------DIST HAMMING 2-----------\n");
	opt_current = loop_local_branching(inst, env, lp, timelimit1, 2, opt_heu);
	printf("Object function optimal value is: %.0f\n", opt_current);
	opt_heu = opt_current;
	printf("-----------DIST HAMMING 5-----------\n");
	time_t timelimit2 = time(NULL) + 1200;
	opt_current = loop_local_branching(inst, env, lp, timelimit2, 5, opt_heu);
	printf("Object function optimal value is: %.0f\n", opt_current);
	opt_heu = opt_current;
	printf("-----------DIST HAMMING 10-----------\n");
	time_t timelimit3 = time(NULL) + 1200;
	opt_current = loop_local_branching(inst, env, lp, timelimit3, 10, opt_heu);
	opt_heu = opt_current;
	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	printf("Object function optimal value is: %.0f\n", opt_heu);


	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT------------*/
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
	//STAMPA
	//update_choosen_edges(inst);

	fprintf(output, "LocalBranching,%s,%f,123456", inst->input_file_name, opt_heu);

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
