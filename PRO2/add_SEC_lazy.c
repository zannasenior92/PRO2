#include "TSP.h"

/*------------------------ADD SUBTOUR ELIMINATION CONSTRAINTS------------------------*/

static int CPXPUBLIC add_SEC_lazy(CPXCENVptr env, void *cbdata, int wherefrom, void *cbhandle, int *useraction_p) {
	*useraction_p = CPX_CALLBACK_DEFAULT;			//Dico che non ho fatto niente 
	instance* inst = (instance *)cbhandle; 			// casting of cbhandle to have the instance

	// get solution xstar
	double *xstar = (double*)calloc(inst->ncols, sizeof(double));
	//Call the callback
	if (CPXgetcallbacknodex(env, cbdata, wherefrom, xstar, 0, inst->ncols - 1)) print_error("Error in callback");

	//apply cut separator and possibly add violated cuts
	int ncuts = myseparation(inst, xstar, env, cbdata, wherefrom);
	//Free space in xstar
	free(xstar);

	if (ncuts >= 1) *useraction_p = CPX_CALLBACK_SET; 		// tell CPLEX that cuts have been created
	return 0;
}

int myseparation(instance *inst, double *xstar, CPXCENVptr env, void *cbdata, int wherefrom) {
	int c1, c2 = 0;
	int n_connected_comp = 0;
	int max = -1;
	int *comp = (int*)calloc(inst->nnodes, sizeof(int));
	int *mycomp = (int*)calloc(inst->nnodes, sizeof(int));
	/*---------------------COUNTING CONNECTED COMPONENTS-----------*/
	/*---------------------INIZIALIZATION----------------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		comp[i] = i;
	}
	/*-----------------------COMPONENTS UNION------------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (xstar[xpos(i, j, inst)] > TOLERANCE) {
				if (comp[i] != comp[j]) {
					c1 = comp[i];
					c2 = comp[j];
				}
				for (int v = 0; v < inst->nnodes; v++) {
					if (comp[v] == c2)
						comp[v] = c1;
				}
			}
		}
	}
	/*--------------------COUNT COMPONENTS---------------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		if (VERBOSE >= 100) {
			printf("Componente %d\n", inst->comp[i]);
		}
		mycomp[comp[i]] = 1;
	}
	int n = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		if (mycomp[i] != 0) {
			n++;
		}
	}
	//Se ha una sola componente connesse non aggiungo vincoli ed esco
	if (n == 1) {
		printf("%d componenti connesse qui\n", n);
		return 0;
	}
	printf("%d componenti connesse", n);

	/*add constraints*/
	int nnz = 0;
	double rhs = -1.0;
	char sense = 'L';
	int *index = (int *)malloc(inst->ncols * sizeof(int));
	double *value = (double *)malloc(inst->ncols * sizeof(double));
	int count = 0;
	for (int h = 0; h < inst->nnodes; h++) {
		if (mycomp[h] != 0) {
			for (int i = 0; i < inst->nnodes; i++) {
				if (comp[i] != h) continue;
				rhs++;

				for (int j = i + 1; j < inst->nnodes; j++) {
					if (comp[j] == h) {
						index[nnz] = xpos(i, j, inst);
						value[nnz] = 1;
						nnz++;
					}
				}
			}
			//Aggiungo i vincoli
			count++;
			if (CPXcutcallbackadd(env, cbdata, wherefrom, nnz, rhs, sense, index, value, 0)) print_error("USER_separation: CPXcutcallbackadd error");
		}
	}
	free(index);
	free(value);
	printf("    Aggiunti %d vincoli\n", count);
	return count;
}