#include "TSP.h"
#include <ilcplex/cplex.h>
#include <time.h>

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
int xpos(int i, int j, instance *inst);
void print_error(const char *err);
void plot_gnuplot(instance *inst);
void hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int seed, double prob);
double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, double prob, double opt);
void reset_lower_bound(instance *inst, CPXENVptr env, CPXLPptr lp);




/*-------------------FUNCTION TO SET THE LOWER BOUND OF THE SOLUTIONS'S VARIABLES--------------------*/
void hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int seed, double prob)
{
	srand(seed);														//SEED FOR RANDOM FUNCTION
	int *index_set = (int *)malloc(inst->nnodes * sizeof(int));			//ARRAY OF INDEXES TO CHANGE BOUND
	double *bounds_set = (double *)calloc(inst->nnodes, sizeof(double));//ARRAY THAT CONTAIN THE NEW VALUE OF THE BOUND					
	char *lb_set = (char *)malloc(inst->nnodes * sizeof(char));			//ARRAY THAT SPECIFIES WHAT BOUND CHANGE FOR EACH VARIABLE
	int count = 0;
	for (int i = 0; i < inst->ncols; i++) {
		if (inst->best_sol[i] == 1) {
			double random = (double)rand() / (double)RAND_MAX;
			if (random <= prob) {
				if (HARD_FIXING >= 400)
				{
					printf("Selected edge %d\n", i);
				}
				index_set[count] = i;
				bounds_set[count] = 1.0;
				lb_set[count] = 'L';
				count++;
			}

		}
	}
	//printf("Selected %d edges in %d\n", count, inst->nnodes);
	CPXchgbds(env, lp, count, index_set, lb_set, bounds_set);			//FUNCTION TO MODIFY BOUNDS TO THE VARIABLES

	CPXwriteprob(env, lp, "modelchanged.lp", NULL);

	free(index_set);
	free(bounds_set);
	free(lb_set);
}

/*---------------------------------------HARD FIXING LOOP--------------------------------------------*/
double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, double prob, double opt) {
	double opt_heu = opt;
	double opt_current;																//VALUE OPTIMAL SOL

	while (time(NULL) < timelimit) {
		hard_fixing(inst, env, lp, time(NULL), prob);
		
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");
		if (CPXgetstat(env, lp) == CPXMIP_INFEASIBLE) {
			printf("IMPOSSIBLE PROBLEM\n");
			reset_lower_bound(inst, env, lp);
			return -1;
		}
		if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");
		//printf("Object function optimal value is: %.0f\n", opt_current);
		if (opt_current < opt_heu) {
			if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
			opt_heu = opt_current;
		}
		reset_lower_bound(inst, env, lp);
	}
	return opt_heu;
}

/*FUNZIONE CHE RESETTA TUTTI I LOWER BOUND DI TUTTE LE VARIABILI*/
void reset_lower_bound(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	//printf("reset bounds \n");
	int *index0 = (int*)malloc(inst->nnodes * sizeof(int));				//ARRAY OF INDEXES TO CHANGE BOUND
	double *bounds0 = (double*)calloc(inst->nnodes, sizeof(double));	//ARRAY THAT CONTAIN THE NEW VALUE OF THE BOUND				
	char *lb0 = (char*)malloc(inst->nnodes * sizeof(char));				//ARRAY THAT SPECIFIES WHAT BOUND CHANGE FOR EACH VARIABLE

	int n = 0;
	for (int i = 0; i < inst->ncols; i++)//SET AN ARRAY OF INDEX REFERRED TO THE VARIABLES THAT I WANT TO CHANGE
	{
		if (inst->best_sol[i] == 1)
		{
			index0[n] = i;
			n++;
		}
	}
	for (int i = 0; i < inst->nnodes; i++)
	{
		lb0[i] = 'L';
	}
	CPXchgbds(env, lp, n, index0, lb0, bounds0);//FUNCTION TO MODIFY BOUNDS TO THE VARIABLES
	//printf("SCRIVO MODEL RESET---------------------\n");
	CPXwriteprob(env, lp, "modelreset.lp", NULL);

	free(index0);
	free(bounds0);
	free(lb0);

}