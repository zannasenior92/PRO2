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
	
	//printf("PARTO CON IL TEMPO=%f\n", (double)time(NULL));
	//Imposto timelimit e chiamo loop per euristico
	time_t timelimit1 = time(NULL) + 30;
	printf("-----------DIST HAMMING 2-----------\n");
	opt_current = loop_local_branching(inst, env, lp, timelimit1, 2, opt_heu);
	opt_heu = opt_current;
	printf("-----------DIST HAMMING 5-----------\n");
	time_t timelimit2 = time(NULL) + 30;
	opt_current = loop_local_branching(inst, env, lp, timelimit2, 5, opt_heu);
	printf("-----------DIST HAMMING 8-----------\n");
	time_t timelimit3 = time(NULL) + 30;
	opt_current = loop_local_branching(inst, env, lp, timelimit3, 10, opt_heu);
	opt_heu = opt_current;
	
	//printf("FINISCO CON IL TEMPO=%f\n", (double)time(NULL));
	int count = 0;
	int n = 0;
	/*-------------------PRINT SELECTED EDGES(remember cplex tolerance)--------------*/
	if (inst->compact == 1) {
		for (int i = 0; i < inst->nnodes; i++) {
			for (int j = 0; j < inst->nnodes; j++) {
					if (inst->best_sol[xpos_compact(i, j, inst)] > TOLERANCE) {

						if (VERBOSE >= 100) {
							printf("Il nodo (%d,%d) e' selezionato\n", i + 1, j + 1);
						}
						/*--ADD EDGES(VECTOR LENGTH = 2*nnodes TO SAVE NODES OF EVERY EDGE)--*/
						inst->choosen_edge[n] = i;
						inst->choosen_edge[n + 1] = j;
						n += 2;
						count++;
					}
			}
		}
	}
	else {
		for (int i = 0; i < inst->nnodes; i++) {
			for (int j = i+1; j < inst->nnodes; j++) {
				if (inst->best_sol[xpos(i, j, inst)] > TOLERANCE) {

					if (VERBOSE >= 100) {
						printf("Il nodo (%d,%d) e' selezionato\n", i + 1, j + 1);
					}
					/*--ADD EDGES(VECTOR LENGTH = 2*nnodes TO SAVE NODES OF EVERY EDGE)--*/
					inst->choosen_edge[n] = i;
					inst->choosen_edge[n + 1] = j;
					n += 2;
					count++;
				}
			}
		}
	}
	add_edge_to_file(inst);

	if (VERBOSE >= 100) {
		printf("Selected nodes: %d \n", count);
	}
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

double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit,  double prob, double opt){
	int fix = 1;
	int finded = 0;
	double opt_heu = opt;
	double opt_current;																//VALUE OPTIMAL SOL

	while (time(NULL) < timelimit) {
		if (fix == 1) {
			hard_fixing(env, lp, inst, time(NULL), prob);
			fix = 0;
		}
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");
		if (CPXgetstat(env, lp) == CPXMIP_INFEASIBLE) {
			printf("PROBLEMA IMPOSSIBILE\n");
			reset_lower_bound(inst, env, lp);
			return -1;
		}
		if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");
		printf("Object function optimal value is: %.0f\n", opt_current);
		if (opt_heu == opt_current) {
			printf("Valori ottimi uguali, resetto\n");
			reset_lower_bound(inst, env, lp);
			fix = 1;

		}
		else {
			if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
			printf("Valori ottimi diversi, continuo\n");
			if (CPXgetobjval(env, lp, &opt_heu)) print_error("Error getting optimal value");
			printf("Object function optimal value is: %.0f\n", opt_heu);
			
		}
	}
	return opt_current;
}

double loop_local_branching(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, int k, double opt) {
	int fix = 1;
	int finded = 0;
	double opt_heu = opt;
	double opt_current;																//VALUE OPTIMAL SOL

	while (time(NULL) < timelimit) {
		if (fix == 1) {
			fix_nodes_local_branching(env, lp, inst, k);
			fix = 0;
		}
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");
		if (CPXgetstat(env, lp) == CPXMIP_INFEASIBLE) {
			printf("PROBLEMA IMPOSSIBILE\n");
			reset_lower_bound(inst, env, lp);
			return -1;
		}
		if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");
		printf("Object function optimal value is: %.0f\n", opt_current);
		if (opt_heu == opt_current) {
			printf("Valori ottimi uguali, resetto\n");
			delete_local_branching_constraint(env, lp);
			fix = 1;

		}
		else {
			if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
			printf("Valori ottimi diversi, continuo\n");
			if (CPXgetobjval(env, lp, &opt_heu)) print_error("Error getting optimal value");
			printf("Object function optimal value is: %.0f\n", opt_heu);

		}
	}
	return opt_current;
}