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
	while (!done) {
		inst->ncols = CPXgetnumcols(env, lp);
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model
		if (CPXsetlogfile(env, log)) print_error("Error in log file");
		
		inst->best_sol = (double *)calloc(inst->ncols, sizeof(double));				//best objective solution
		if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
		if (kruskal_sst(env, lp, inst) == 1) {
			done = 1;
		}
		
		else {
			add_SEC(env,lp,inst);
			if (VERBOSE >= 10) {
				printf("Aggiunti vincoli\n");
			}
		}
		
	}


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

/*---------------------------DEFINING CONNECTED COMPONENTS---------------------------*/
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst) {
	int c1, c2 = 0;
	int n_connected_comp = 0;
	int max = -1;
	inst->mycomp = (int*)calloc(inst->nnodes, sizeof(int));

	/*---------------------INIZIALIZATION----------------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		inst->comp[i] = i;
	}
	/*-----------------------COMPONENTS UNION------------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (inst->best_sol[xpos(i, j, inst)] > TOLERANCE) {
				if (inst->comp[i] != inst->comp[j]) {
					c1 = inst->comp[i];
					c2 = inst->comp[j];
				}
				for (int v = 0; v < inst->nnodes; v++) {
					if (inst->comp[v] == c2)
						inst->comp[v] = c1;
				}
				
			}

		}
	}
	

	/*--------------------COUNT COMPONENTS---------------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		if (VERBOSE >= 100) {
			printf("Componente %d\n", inst->comp[i]);
		}
		inst->mycomp[inst->comp[i]] = 1;
	}
	int n = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		if (inst->mycomp[i]!=0) {
			n++;
		}
	}
	if(VERBOSE>=100){
		printf("Componenti connesse %d\n", n);
	}
	inst->n_connected_comp = n;
	return n;
	/*---------------------------------------------------------*/

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

static int CPXPUBLIC add_SEC_lazy(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p) {
	*useraction_p = CPX_CALLBACK_DEFAULT;   //Dico che non ho fatto niente 
	instance* inst = (instance *)cbhandle; 			// casting of cbhandle    
	//da qui abbiamo di nuovo il puntatore all'instanza

	// get solution xstar
	printf("ncols=%d\n", inst->ncols);
	

	double *xstar = (double*)malloc(inst->ncols * sizeof(double));
		
	if (CPXgetcallbacknodex(env, cbdata, wherefrom, xstar, 0, inst->ncols - 1)) return 1; // y = current y from CPLEX-- y starts from position 0
	//Praticamente la getx, da la soluzione per la quale la soluzione è stata chiamata
	//Ripasso i parametri riempi posizione da 0 a numero di colonne (forse ncols-1) mi salvo prima il numero di colonne cosi per averle qua
	/*for (int i = 0; i < inst->ncols; i++)
		printf("Xstar[%d]=%d\n", i, xstar[i]);
		*/
	double zbest;	if(CPXgetcallbackinfo(env, cbdata, wherefrom, CPX_CALLBACK_INFO_BEST_INTEGER, &zbest)) print_error("Error getting zbest"); 	//valore dell'ottimo intero	printf("zbest=%f\n", zbest);
	//apply cut separator and possibly add violated cuts
	int ncuts = myseparation(inst, xstar, env, cbdata, wherefrom);	    //separatore per aggiungere vincoli e restituisce quanti tagli ha aggiunto
	free(xstar);							//IMPORTANTE!!!!! seno esauriamo la memoria

	if (ncuts >= 1) *useraction_p = CPX_CALLBACK_SET; 		// tell CPLEX that cuts have been created
	return 0;
}

int myseparation(instance *inst, double *xstar, CPXCENVptr env, void *cbdata, int wherefrom) {
	int nnz = 0;
	double rhs = -1.0;
	char sense = 'L';
	int *index = (int *)malloc(inst->ncols * sizeof(int));
	double *value = (double *)malloc(inst->ncols * sizeof(double));
	int count = 0;
	for (int h = 0; h < inst->nnodes; h++) {
		if (inst->mycomp[h] != 0) {
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
			count++;
			if (CPXcutcallbackadd(env, cbdata, wherefrom, nnz, rhs, sense, index, value, 0)) print_error("USER_separation: CPXcutcallbackadd error");
		}

	}	return count;
}

