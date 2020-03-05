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
void update_choosen_edge(instance* inst);
double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, double prob, double opt);
void hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int seed, double prob);
double nearest_neighborhood(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node, int seed);
double two_opt(instance *inst, CPXENVptr env, CPXLPptr lp);
double three_opt(instance *inst, CPXENVptr env, CPXLPptr lp);
void VNS(instance *inst, CPXENVptr env, CPXLPptr lp, double opt_current, double min_cost);
void genetic_alg(instance *inst, CPXENVptr env, CPXLPptr lp);
int cost_alg(instance* inst);

/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	//inst->compact = 0;
	int error;
	double opt_heu, opt_current;											//opt_heu = global optimal valure of heuristic :: opt_current = current valure of heuristic
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	
	build_model(inst, env, lp);
	
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);							//to visualize in video
	FILE* log = CPXfopen("log.txt", "w");
	

	/*------------------------HARD FIXING WITH LAZYCALLBACK--------------------------*/
	inst->ncols = CPXgetnumcols(env, lp);

	/*----TAKE THE BEST INITIAL SOLUTION WITH NEAREST NEIGHBORHOOD GRASP-------------*/
	inst->best_sol= (double*)calloc(inst->ncols, sizeof(double));
	opt_heu = INFINITY;
	double *minimum_solution = (double*)calloc(inst->ncols, sizeof(double));
	int start_node = 0;
	
	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < inst->nnodes; j++) {
			inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
			
			opt_current = nearest_neighborhood_GRASP(inst, env, lp, j, j);
			if (opt_current < opt_heu) {
				opt_heu = opt_current;
				for (int k = 0; k < inst->ncols; k++) {
					minimum_solution[k] = inst->best_sol[k];

				}
			}
		}
	}
	
	printf("\nBest Initial Cost After Nearest Neighborhood GRASP %f\n", opt_heu);
	for (int k = 0; k < inst->ncols; k++) {
		inst->best_sol[k]= minimum_solution[k];
	}
	/*-------------------------------------------------------------------------------*/

	
	/*-----------PRINT INITIAL SOLUTION-----------*/
	selected_edges(inst);
	/**********************************************/

	free(minimum_solution);	

	/*------------SET INITIAL OPTIMAL VALUE---------*/
	opt_current = opt_heu;
	printf("Opt current %f opt calc %d\n", opt_current, cost_alg(inst));

	/*------------SETTING OF CALLBACKS--------------*/
	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);								// let MIP callbacks work on the original model
	CPXsetdblparam(env, CPX_PARAM_TILIM, 30);
	CPXsetlazyconstraintcallbackfunc(env, add_SEC_lazy, inst);
	int ncores = 1; 
	CPXgetnumcores(env, &ncores);
	CPXsetintparam(env, CPX_PARAM_THREADS, ncores);


	/*---------------------START HARD FIXING TO FIX INITIAL SOLUTION-------------------------------------------*/

	hard_fixing(inst, env, lp, 1, 1);//--------SET ALL EDGES WITH A HARD SELECTION
	if (CPXmipopt(env, lp)) print_error("Error resolving model");
	if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");
	printf("Object function optimal value is: %.0f\n", opt_current);
	reset_lower_bound(inst, env, lp);
	
	/*-----------PRINT INITIAL SOLUTION-----------*/
	selected_edges(inst);
	/**********************************************/

	time_t time0 = time(NULL);
	opt_heu = opt_current;
	//SET TIMELIMIT AND USE HEURISTIC LOOP
	time_t timelimit1 = time(NULL) + 5;
	printf("-----------SET 60%%-----------\n");
	opt_current= loop_hard_fixing(inst, env, lp, (double)timelimit1, 0.6, opt_heu);
	opt_heu = opt_current;
	printf("-----------SET 40%%-----------\n");
	time_t timelimit2 = time(NULL) + 5;
	opt_current = loop_hard_fixing(inst, env, lp, (double)timelimit2, 0.4, opt_heu);
	opt_heu = opt_current;
	printf("-----------SET 20%%-----------\n");
	time_t timelimit3 = time(NULL) + 5;
	opt_current = loop_hard_fixing(inst, env, lp, (double)timelimit3, 0.2, opt_heu);

	printf("FINISH WITH TIME=%f\n", (double)(time(NULL)-time0));


	/*---------------PRINT SELECTED EDGES--------------------------------------------*/
	selected_edges(inst);
	/*-------------------------------------------------------------------------------*/
	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	printf("Object function optimal value is: %.0f\n", opt_current);
	
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