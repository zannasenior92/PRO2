#include "TSP.h"


void fix_nodes_local_branching(CPXENVptr env, CPXLPptr lp, instance *inst, double k);
void delete_local_branching_constraint(CPXENVptr env, CPXLPptr lp);

/*----------------------------------------LOCAL BRANCHING----------------------------------------------*/
double loop_local_branching(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, int k, double opt) {


	double opt_heu = opt;
	double opt_current;																		//VALUE OPTIMAL SOL

	/*INSERT THE CONSTRAINT RELATED TO THE NUMBER OF VARIABLES THAT HAVE TO BE SELECTED*/
	fix_nodes_local_branching(env, lp, inst, k);
	
	while (time(NULL) < timelimit) {
		
		int random = rand();
		CPXsetintparam(env, CPX_PARAM_RANDOMSEED, random);
		
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");
		if (CPXgetstat(env, lp) == CPXMIP_INFEASIBLE) {
			printf("PROBLEMA IMPOSSIBILE\n");
			reset_lower_bound(inst, env, lp);
			return -1;
		}
		if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");

		if (VERBOSE > 0) printf("Object function optimal value is: %.0f\n", opt_current);
		
		if (opt_current < opt_heu) {
			if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
			printf("Aggiorno minimo\n");
			opt_heu = opt_current;
		}
		reset_lower_bound(inst, env, lp);
		delete_local_branching_constraint(env, lp);

		fix_nodes_local_branching(env, lp, inst, k);

		/*if (opt_heu == opt_current) {

			if (VERBOSE > 50) printf("Valori ottimi uguali, resetto\n");
			reset_lower_bound(inst, env, lp);
		}
		else {
			if (opt_current < opt_heu) {
				if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
				printf("Valori ottimi diversi, continuo\n");
				if (CPXgetobjval(env, lp, &opt_heu)) print_error("Error getting optimal value");

			}
			else {
				reset_lower_bound(inst, env, lp);
			}
			//printf("Object function optimal value is: %.0f\n", opt_heu);
			
		}*/
	}

	delete_local_branching_constraint(env, lp);
	return opt_current;
}



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