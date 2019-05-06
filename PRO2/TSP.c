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
void hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int seed, double prob);
void start_sol(instance *inst);
//int loop_method(CPXENVptr env, CPXLPptr lp, instance *inst, FILE* log);
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst);
void update_choosen_edge(instance* inst);
double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, int s, double prob, double opt);

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
	
	
	
	/*------------------------------------USO HARD FIXING CON LAZYCALLBACK---------------------------------------*/
	inst->ncols = CPXgetnumcols(env, lp);
	/*
	if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
	if (CPXsetlogfile(env, log)) print_error("Error in log file");
	inst->best_sol = (double *)calloc(inst->ncols, sizeof(double));				//best objective solution
	if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
	*/
	//if (CPXsetdblparam(env, CPX_PARAM_TILIM, 120)) print_error("Error on setting parameter");
	//if (CPXsetintparam(env, CPX_PARAM_NODELIM, 0)) print_error("Error on setting parameter");
	int done = 0;
	//METODO LOOP PER 5 COLPI per trovare soluzione iniziale
	while (done < 2) {
		done++;
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
		if (CPXsetlogfile(env, log)) print_error("Error in log file");
		int ncols = CPXgetnumcols(env, lp);
		inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
		if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
		
		int comp = kruskal_sst(env, lp, inst);
		if(done<2)
			add_SEC(env, lp, inst);

		update_choosen_edge(inst);
		add_edge_to_file(inst);
		plot_gnuplot(inst);
		
	}
	double opt_heu, opt_current;
	if (CPXgetobjval(env, lp, &opt_heu)) print_error("Error getting optimal value");;
	printf("Valore di partenza della funzione obiettivo: %.0f\n", opt_heu);
	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);								// let MIP callbacks work on the original model
	CPXsetlazyconstraintcallbackfunc(env, add_SEC_lazy, inst);
	int ncores = 1; CPXgetnumcores(env, &ncores);
	CPXsetintparam(env, CPX_PARAM_THREADS, ncores);
	if (CPXsetdblparam(env, CPX_PARAM_TILIM, 5)) print_error("Error on setting parameter");
	
	//printf("PARTO CON IL TEMPO=%f\n", (double)time(NULL));
	time_t timelimit = time(NULL)+5;
	int seed = 1;
	opt_current= loop_hard_fixing(inst, env, lp, timelimit, seed, 0.3, opt_heu);
	if (opt_current == -1) {
		time_t timelimit = time(NULL) + 5;
		printf("Riprovo fissando meno archi\n");
		opt_current = loop_hard_fixing(inst, env, lp, timelimit, seed+1, 0.7, opt_heu);
	}

	/*while (time(NULL) < timelimit) {
		if(fix==1){
			hard_fixing(env, lp, inst, seed, 0.9);
			fix = 0;
			seed += seed;
		}
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");
		if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");
		printf("Object function optimal value is: %.0f\n", opt_current);
		if (opt_heu == opt_current) {
			printf("Valori ottimi uguali, resetto\n");
			reset_lower_bound(inst, env, lp);
			fix = 1;
			finded++;
			printf("AGGIORNO FINDED=%d\n", finded);

		}
		else {
			//update_x_heu(inst, env, lp);
			if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
			printf("Valori ottimi diversi, continuo\n");
			if (CPXgetobjval(env, lp, &opt_heu)) print_error("Error getting optimal value");
			printf("Object function optimal value is: %.0f\n", opt_heu);
			printf("AZZERO FINDED\n");
			finded = 0;
			//reset_lower_bound(inst, env, lp);
			//fix = 1;
		}
		if (finded == 5) {
			printf("FINDED=%d\n",finded);
			break;
		}
		
	}*/
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
	//double opt_val;																//VALUE OPTIMAL SOL
	//if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");;													//OPTIMAL SOLUTION FOUND
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

static int CPXPUBLIC add_SEC_lazy_hard_fix(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p) {
	*useraction_p = CPX_CALLBACK_DEFAULT;			//Dico che non ho fatto niente 
	instance* inst = (instance *)cbhandle; 			// casting of cbhandle to have the instance

	/*-------------GET XSTAR SOLUTION--------------------------*/
	double *xstar = (double*)calloc(inst->ncols, sizeof(double));

	/*--------------CALL THE CALLBACK--------------------------------------------------------------------------*/
	if (CPXgetcallbacknodex(env, cbdata, wherefrom, xstar, 0, inst->ncols - 1)) print_error("Error in callback");
	
	/*APPLY CUT SEPARATOR-ADD CONSTRAINTS FOR EVERY CONNECTED COMPONENT AND RETURN NUMBER OF ADDED CONSTRAINTS*/
	/*int ncuts = myseparation(inst, xstar, env, cbdata, wherefrom);
	//Free space in xstar
	free(xstar);

	if (ncuts >= 1) {
		*useraction_p = CPX_CALLBACK_SET; 		// tell CPLEX that cuts have been created
	}*/
	return 0;
}

void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst) {
	int nnz = 0;
	double rhs = -1.0;
	char sense = 'L';
	int ncols = CPXgetnumcols(env, lp);
	int *index = (int *)malloc(ncols * sizeof(int));
	double *value = (double *)malloc(ncols * sizeof(double));
	int matbeg = 0;
	char **cname = (char **)calloc(1, sizeof(char *));							// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));

	for (int h = 0; h < inst->nnodes; h++) {

		if (inst->mycomp[h] != 0) {
			sprintf(cname[0], "SEC(%d)", h);
			for (int i = 0; i < inst->nnodes; i++) {
				if (inst->comp[i] != h) continue;
				rhs++;

				for (int j = i + 1; j < inst->nnodes; j++) {
					if (inst->comp[j] == h) {
						index[nnz] = xpos(i, j, inst);
						value[nnz] = 1;
						nnz++;
					}
				}
			}
			if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &matbeg, index, value, NULL, cname)) print_error("wrong CPXaddrow");
			if (VERBOSE >= 10) {
				CPXwriteprob(env, lp, "model.lp", NULL);
			}
		}

	}
}
void update_choosen_edge(instance* inst) {
	int n = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (inst->best_sol[xpos(i, j, inst)] > 0.5) {

				if (VERBOSE >= 100) {
					printf("Il nodo (%d,%d) e' selezionato\n", i + 1, j + 1);
				}
				/*--ADD EDGES(VECTOR LENGTH = 2*nnodes TO SAVE NODES OF EVERY EDGE)--*/
				inst->choosen_edge[n] = i;
				inst->choosen_edge[n + 1] = j;
				n += 2;

			}
		}
	}
}

double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, int s , double prob, double opt){
	int fix = 1;
	int finded = 0;
	double opt_heu = opt;
	int seed = s;
	double opt_current;																//VALUE OPTIMAL SOL

	while (time(NULL) < timelimit) {
		if (fix == 1) {
			hard_fixing(env, lp, inst, seed, prob);
			fix = 0;
			seed += seed;
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
			finded++;
			printf("AGGIORNO FINDED=%d\n", finded);

		}
		else {
			//update_x_heu(inst, env, lp);
			if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
			printf("Valori ottimi diversi, continuo\n");
			if (CPXgetobjval(env, lp, &opt_heu)) print_error("Error getting optimal value");
			printf("Object function optimal value is: %.0f\n", opt_heu);
			printf("AZZERO FINDED\n");
			finded = 0;
			//reset_lower_bound(inst, env, lp);
			//fix = 1;
		}
		if (finded == 4) {
			printf("FINDED=%d\n", finded);
			break;
		}
	}
	return opt_current;
}