/*--------------------------TSP RESOLVE & CREATE THE MODEL---------------------------*/
#include "TSP.h"
#include <ilcplex/cplex.h>


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFlow1(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFischetti(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst);
static int CPXPUBLIC add_SEC_lazy(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p);
int myseparation(instance *inst, double *xstar, CPXCENVptr env, void *cbdata, int wherefrom);


/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	
	inst->compact = 0;
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	
	build_model(inst, env, lp);
	FILE* log = CPXfopen("log.txt", "w");
	CPXsetintparam(env, CPX_PARAM_MIPCBREDLP, CPX_OFF);								// let MIP callbacks work on the original model
	CPXsetlazyconstraintcallbackfunc(env, add_SEC_lazy, inst);
	int ncores = 1; CPXgetnumcores(env, &ncores);
	CPXsetintparam(env, CPX_PARAM_THREADS, ncores);
	/*------------------------------------METODO LOOP---------------------------------------*/
	int done = 0;
	inst->ncols = CPXgetnumcols(env, lp);
	if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
	if (CPXsetlogfile(env, log)) print_error("Error in log file");
	
	inst->best_sol = (double *)calloc(inst->ncols, sizeof(double));				//best objective solution
	if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");


	if(VERBOSE>=200){
		for (int i = 0; i < inst->ncols - 1; i++){
			printf("Best %f\n", inst->best_sol[i]);
		}
	}
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
	double *opt_val = 0;																//VALUE OPTIMAL SOL
	if (CPXgetobjval(env, lp, &opt_val)) print_error("Error getting optimal value");;													//OPTIMAL SOLUTION FOUND
	printf("Object function optimal value is: %.0f\n", opt_val);
	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT------------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}


/*------------------------ADD SUBTOUR ELIMINATION CONSTRAINTS------------------------*/
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst) {
	int nnz = 0;
	double rhs = -1.0;
	char sense = 'L';
	int ncols = CPXgetnumcols(env, lp);
	int *index = (int *)malloc( ncols* sizeof(int));
	double *value = (double *)malloc(ncols * sizeof(double));
	int matbeg = 0;
	char **cname = (char **)calloc(1, sizeof(char *));							// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));

	for(int h=0; h < inst->nnodes; h++){
		if(inst->mycomp[h]!=0){
			for (int i = 0; i < inst->nnodes; i++) {
				if (inst->comp[i] != h) continue;
				rhs++;
				sprintf(cname[0], "SEC(%d)", i);

				for (int j = i + 1; j < inst->nnodes; j++) {
					if (inst->comp[j] == h) {
						index[nnz] = xpos(i, j, inst);
						value[nnz] = 1;
						nnz++;
					}
				}
			}
			if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &matbeg, index, value, NULL, cname)) print_error("wrong CPXaddrow");
			if(VERBOSE>=200){
				CPXwriteprob(env, lp, "model.lp", NULL);
			}
			}

	}
}

