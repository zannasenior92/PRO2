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
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);
double two_opt(instance *inst, CPXENVptr env, CPXLPptr lp);
double three_opt(instance *inst, CPXENVptr env, CPXLPptr lp);
double vns(instance *inst, CPXENVptr env, CPXLPptr lp);
int cost_alg(instance* inst);


//void vns(instance *inst, CPXENVptr env, CPXLPptr lp);
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
	double *minimum_solution = (double*)calloc(inst->ncols, sizeof(double));
	int start_node = 0;

	for (int i = 0; i < 50; i++) {
		for (int j = 0; j < inst->nnodes; j++) {
			inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
			

			cost = nearest_neighborhood_GRASP(inst, env, lp, j);
			if (cost < min_cost) {
				min_cost = cost;
				for (int k = 0; k < inst->ncols; k++) {
					minimum_solution[k] = inst->best_sol[k];

				}
				//minimum_solution = inst->best_sol;
			}
		}
	}
	//CODICE PER TROVARE NEAREST NEIGHBOORHOOD CHE SCORRE TUTTI I NODI
	/*for (int i = 0; i < inst->nnodes; i++) {
		cost = nearest_neighborhood(inst, env, lp, i);
		if (cost < min_cost) {
			min_cost = cost;
			start_node = i;
		}
	}*/

	printf("\nBest Initial Cost After Nearest Neighborhood %f\n", min_cost);
	for (int k = 0; k < inst->ncols; k++) {
		inst->best_sol[k]= minimum_solution[k];

	}
	//inst->best_sol = minimum_solution;
	
	/*inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
	cost = nearest_neighborhood(inst, env, lp, start_node);
	hard_fixing(inst, env, lp, 1, 1);//SET ALL EDGES
	if (CPXmipopt(env, lp)) print_error("Error resolving model");
	if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");
	printf("Object function optimal value is: %.0f\n", opt_current);
	reset_lower_bound(inst, env, lp);
	*/
	/*-------PRINT INITIAL SOLUTION--------*/
	update_choosen_edge(inst);
	add_edge_to_file(inst);
	plot_gnuplot(inst);
	
	
	//SET INITIAL OPTIMAL VALUE TO INFINITE
	//opt_heu = min_cost;
	opt_current = min_cost;
	printf("Opt current %f   opt calc %d\n", opt_current, cost_alg(inst));
	//SETTING OF CALLBACKS
	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);								// let MIP callbacks work on the original model
	CPXsetdblparam(env, CPX_PARAM_TILIM, 30);
	CPXsetlazyconstraintcallbackfunc(env, add_SEC_lazy, inst);
	int ncores = 1; CPXgetnumcores(env, &ncores);
	CPXsetintparam(env, CPX_PARAM_THREADS, ncores);
	int local_minimum = 0;
	time_t timelimit = time(NULL) + 120;
	double delta, delta_vns;
	//min_cost = INFINITY;
	//minimum_solution = (double*)calloc(inst->ncols, sizeof(double));
	while (time(NULL) < timelimit) {
		
		delta = two_opt(inst, env, lp);
		printf("Delta: %f\n", delta);
		opt_current += delta;
		int cost_with_alg = cost_alg(inst);
		/*if (opt_current != cost_with_alg) {
			opt_current = cost_with_alg;
		}*/
		printf("New objective function: %f\n", opt_current);
		printf("COSTO CALCOLATO:%d\n", cost_alg(inst));
		if (delta == 0.0) {
			local_minimum++;
			//printf("delta=0\n");
		}
		
		if (local_minimum == 5) {
			//printf("MINIMO LOCALE: %f\n", opt_current);
			if (opt_current < min_cost) {
				//printf("Soluzione migliore\n");
				minimum_solution = (double*)calloc(inst->ncols, sizeof(double));
				for (int k = 0; k < inst->ncols; k++) {
					minimum_solution[k] = inst->best_sol[k];

				}
				//minimum_solution = inst->best_sol;
				/*for (int k = 0; k < inst->ncols; k++) {
					if (inst->best_sol[k] != minimum_solution[k])
						exit(-32);
				}*/
				min_cost = opt_current;
				printf("PRENDO IL MINIMO %f\n", min_cost);
				
				/*update_choosen_edge(inst);
				add_edge_to_file(inst);
				plot_gnuplot(inst);*/
			}
			else {
				//printf("Soluzione peggiore, prendo la precedente\n");
				int cost_before = cost_alg(inst);
				inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
				for (int k = 0; k < inst->ncols; k++) {
					inst->best_sol[k] = minimum_solution[k];
						
				}
				//inst->best_sol = minimum_solution;
				int cost_after = cost_alg(inst);
				printf("COSTO PRIMA %d      COSTO DOPO %d\n", cost_before, cost_after);
				opt_current = min_cost;
				
			}
			int done = 0;
			while (done == 0) {
				delta_vns = vns(inst, env, lp);
				if (delta_vns == -1) continue;
				else {
					printf("DELTA VNS=%f\n", delta_vns);
					opt_current += delta_vns;
					done = 1;
				}
			}
			
			printf("---------------------------------------------------CALCIO 3-OPT min=%f\n", min_cost);
			printf("Object function %f\n", opt_current);
			printf("Object calculated %d\n", cost_alg(inst));
			/*update_choosen_edge(inst);
			add_edge_to_file(inst);
			plot_gnuplot(inst);*/
			local_minimum = 0;

		}
		

	}
	if (opt_current < min_cost) {
		printf("FINAL Object function %f\n", opt_current);
		int cost_with_alg = cost_alg(inst);
		printf("FINAL Object function calculated %d\n", cost_with_alg);
	}
	else {
		printf("FINAL Object function %f\n", min_cost);
		inst->best_sol = minimum_solution;
		int cost_with_alg = cost_alg(inst);
		printf("FINAL Object function calculated %d\n", cost_with_alg);
	}
	
	hard_fixing(inst, env, lp, 1, 1);//SET ALL EDGES
	if (CPXmipopt(env, lp)) print_error("Error resolving model");
	if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");
	printf("Object function optimal value is: %.0f\n", opt_current);
	reset_lower_bound(inst, env, lp);


	update_choosen_edge(inst);
	add_edge_to_file(inst);
	plot_gnuplot(inst);
	free(minimum_solution);
	//exit(0);
	
	time_t time0 = time(NULL);
	opt_heu = opt_current;
	//SET TIMELIMIT AND USE HEURISTIC LOOP
	time_t timelimit1 = time(NULL) + 30;
	printf("-----------SET 70%%-----------\n");
	opt_current= loop_hard_fixing(inst, env, lp, (double)timelimit1, 0.6, opt_heu);
	opt_heu = opt_current;
	printf("-----------SET 50%%-----------\n");
	time_t timelimit2 = time(NULL) + 30;
	opt_current = loop_hard_fixing(inst, env, lp, (double)timelimit2, 0.4, opt_heu);
	opt_heu = opt_current;
	printf("-----------SET 20%%-----------\n");
	time_t timelimit3 = time(NULL) + 30;
	opt_current = loop_hard_fixing(inst, env, lp, (double)timelimit3, 0.2, opt_heu);

	printf("FINISH WITH TIME=%f\n", (double)(time(NULL)-time0));


	/*---------------PRINT SELECTED EDGES--------------------------------------------*/
	selected_edges(inst);
	/*-------------------------------------------------------------------------------*/
	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	printf("Object function optimal value is: %.0f\n", opt_current);
	/*update_choosen_edge(inst);
	add_edge_to_file(inst);
	plot_gnuplot(inst);
	*/
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