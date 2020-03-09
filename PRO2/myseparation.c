#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
int xpos(int i, int j, instance *inst);

/*------------------------------MYSEPARATION METHOD TO ---------------------------------*/
int myseparation(instance *inst, double *xstar, CPXCENVptr env, void *cbdata, int wherefrom) {
	int c1, c2 = 0;
	int n_connected_comp = 0;
	int max = -1;
	int *comp = (int*)calloc(inst->nnodes, sizeof(int));
	int *mycomp = (int*)calloc(inst->nnodes, sizeof(int));

	/*---------------------COUNTING CONNECTED COMPONENTS------------------*/

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
		if (MYSEPARATION >= 100) {printf("Componente %d\n", inst->comp[i]);}
		mycomp[comp[i]] = 1;
	}
	int n = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		if (mycomp[i] != 0) {
			n++;
		}
	}

	if (n == 1)//IF IT HAS A SINGLE CONNECTED COMPONENT I DON'T ADD CONSTRAINTS AND EXIT
	{
		if (MYSEPARATION >=100){printf("%d componenti connesse qui\n", n);}
		return 0;
	}
	if (MYSEPARATION >=100){printf("%d componenti connesse", n);}

	/*-----------------------ADD CONSTRAINTS-------------------------*/
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
			count++;

			/*ADD CONSTRAINTS*/
			if (CPXcutcallbackadd(env, cbdata, wherefrom, nnz, rhs, sense, index, value, 0)) print_error("USER_separation: CPXcutcallbackadd error");
		}
	}
	free(index);
	free(value);

	if (MYSEPARATION >=100){printf("    Aggiunti %d vincoli\n", count);}

	return count;
}