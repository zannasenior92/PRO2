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
double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, double prob, double opt);
void selected_edges(instance *inst);
void fix_nodes_local_branching(CPXENVptr env, CPXLPptr lp, instance *inst, double k);
void delete_local_branching_constraint(CPXENVptr env, CPXLPptr lp);
double loop_local_branching(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, int k, double opt);


/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	inst->compact = 0;
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	build_model(inst, env, lp);
	//select_and_build_model(inst, env, lp);
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);									//Per visualizzare a video
	FILE* log = CPXfopen("log.txt", "w");
	
	
	
	/*------------------------------------USO LOCAL BRANCHING CON LAZYCALLBACK---------------------------------------*/
	inst->ncols = CPXgetnumcols(env, lp);
	//PARTO CON SOLUZIONE INIZIALE BANALE 1-2-3-...
	start_sol(inst);
	//stampo soluzione iniziale
	update_choosen_edge(inst);
	add_edge_to_file(inst);
	plot_gnuplot(inst);

	double opt_heu, opt_current;
	//Setto ottimo soluzione iniziale a infinito
	opt_heu = CPX_INFBOUND;
	//Codice di intro per callback
	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);								// let MIP callbacks work on the original model
	CPXsetlazyconstraintcallbackfunc(env, add_SEC_lazy, inst);
	int ncores = 1; CPXgetnumcores(env, &ncores);
	CPXsetintparam(env, CPX_PARAM_THREADS, ncores);
	CPXsetdblparam(env, CPX_PARAM_TILIM, 60);
	//printf("PARTO CON IL TEMPO=%f\n", (double)time(NULL));
	//Imposto timelimit e chiamo loop per euristico
	time_t timelimit1 = time(NULL) + 10;
	printf("-----------DIST HAMMING 2-----------\n");
	opt_current = loop_local_branching(inst, env, lp, timelimit1, 2, opt_heu);
	opt_heu = opt_current;
	printf("-----------DIST HAMMING 5-----------\n");
	time_t timelimit2 = time(NULL) + 10;
	opt_current = loop_local_branching(inst, env, lp, timelimit2, 5, opt_heu);
	printf("-----------DIST HAMMING 8-----------\n");
	
	time_t timelimit3 = time(NULL) + 10;
	opt_current = loop_local_branching(inst, env, lp, timelimit3, 10, opt_heu);
	opt_heu = opt_current;
	
	//pr226
	//hamming 2-5-8 con 300 sec all'uno 81216
	// hamming 2 con 900 87716
	// hamming 8 con 900 81128
	//printf("FINISCO CON IL TEMPO=%f\n", (double)time(NULL));
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