#include "TSP.h"

void fix_nodes_local_branching(CPXENVptr env, CPXLPptr lp, instance *inst, double k)
{
	char **cname = (char **)calloc(1, sizeof(char *));									// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));
	int nnz = 0;
	double rhs = (double)(inst->nnodes)-k;
	char sense = 'G';
	int ncols = CPXgetnumcols(env, lp);

	int matbeg = 0;
	cname[0] = (char *)calloc(100, sizeof(char));
	sprintf(cname[0], "Local_branch");
	int *index = (int *)malloc(inst->nnodes * sizeof(int));
	double *value = (double *)malloc(inst->nnodes * sizeof(double));
	for (int i = 0; i < inst->ncols; i++) {
		if(inst->best_sol[i] > TOLERANCE){
			index[nnz] = i;
			value[nnz] = 1.0;
			nnz++;
		}	
	}
	if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &matbeg, index, value, NULL, cname)) print_error("wrong CPXaddrow");
	free(index);
	free(value);
	CPXwriteprob(env, lp, "model_localBranch.lp", NULL);
}

void delete_local_branching_constraint(CPXENVptr env, CPXLPptr lp) {
	int lastrow = CPXgetnumrows(env, lp);
	if (CPXdelrows(env, lp, lastrow-1, lastrow-1)) print_error("ERROR on deleting row");
}