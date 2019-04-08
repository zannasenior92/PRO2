/*--------------------------------KRUSKAL AND SEC METHODS----------------------------*/
/*************************************************************************************/

#include "TSP.h"

/*---------------------------DEFINING CONNECTED COMPONENTS---------------------------*/
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst) {
	int c1, c2 = 0;
	int n_connected_comp = 0;
	int max = -1;
	inst->mycomp = (int*)calloc(inst->nnodes, sizeof(int));

	/*INIZIALIZZAZIONE*/
	for (int i = 0; i < inst->nnodes; i++) {
		inst->comp[i] = i;
	}
	/*UNIONE COMPONENTI CONNESSE*/
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (inst->best_sol[xpos(i, j, inst)] > 0.5) {
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

	for (int i = 0; i < inst->nnodes; i++) {
		if (VERBOSE >= 100) {
			printf("Componente %d\n", inst->comp[i]);
		}
		inst->mycomp[inst->comp[i]] = 1;
	}

	int n = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		if (inst->mycomp[i] != 0) {
			n++;
		}
	}
	if (VERBOSE >= 10) {
		printf("Componenti connesse %d\n", n);
	}
	inst->n_connected_comp = n;
	return n;
}


/*------------------------ADD SUBTOUR ELIMINATION CONSTRAINTS------------------------*/
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst) {
	inst->sec = 0;

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
			inst->sec = 1;
			if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &matbeg, index, value, NULL, cname)) print_error("wrong CPXaddrow");
			if (VERBOSE >= 200) {
				CPXwriteprob(env, lp, "model.lp", NULL);
			}
		}

	}

}