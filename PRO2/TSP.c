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
	

	/*------------------------HARD FIXING WITH LAZYCALLBACK--------------------------*/
	inst->ncols = CPXgetnumcols(env, lp);
	//START WITH TRIVIAL INITIAL SOLUZION 1->2->3-...->n-1->1
	//start_sol(inst);
	inst->best_sol= (double*)calloc(inst->ncols, sizeof(double));
	double cost, min_cost;
	min_cost = INFINITY;
	int start_node = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		cost = nearest_neighborhood(inst, env, lp, i);
		if (cost < min_cost) {
			min_cost = cost;
			start_node = i;
		}
	}
	printf("\nCOSTO INIZIALE MIGLIORE %f\n", min_cost);
	inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
	cost = nearest_neighborhood(inst, env, lp, start_node);
	hard_fixing(inst, env, lp, 1, 1);//FISSO TUTTI I LATI
	if (CPXmipopt(env, lp)) print_error("Error resolving model");
	if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");
	printf("Object function optimal value is: %.0f\n", opt_current);
	reset_lower_bound(inst, env, lp);
	/*-------PRINT INITIAL SOLUTION--------*/
	update_choosen_edge(inst);
	add_edge_to_file(inst);
	plot_gnuplot(inst);
	//exit(0);
	//SET INITIAL OPTIMAL VALUE TO INFINITE
	opt_heu = opt_current;
	//SETTING OF CALLBACKS
	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);								// let MIP callbacks work on the original model
	CPXsetdblparam(env, CPX_PARAM_TILIM, 30);
	CPXsetlazyconstraintcallbackfunc(env, add_SEC_lazy, inst);
	int ncores = 1; CPXgetnumcores(env, &ncores);
	CPXsetintparam(env, CPX_PARAM_THREADS, ncores);
	
	time_t time0 = time(NULL);

	//SET TIMELIMIT AND USE HEURISTIC LOOP
	time_t timelimit1 = time(NULL) + 90;
	printf("-----------FISSO 70%%-----------\n");
	opt_current= loop_hard_fixing(inst, env, lp, (double)timelimit1, 0.6, opt_heu);
	opt_heu = opt_current;
	printf("-----------FISSO 50%%-----------\n");
	time_t timelimit2 = time(NULL) + 90;
	opt_current = loop_hard_fixing(inst, env, lp, (double)timelimit2, 0.4, opt_heu);
	opt_heu = opt_current;
	printf("-----------FISSO 20%%-----------\n");
	time_t timelimit3 = time(NULL) + 90;
	opt_current = loop_hard_fixing(inst, env, lp, (double)timelimit3, 0.2, opt_heu);

	printf("FINISCO CON IL TEMPO=%f\n", (double)(time(NULL)-time0));


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
