#include "TSP.h"
#include <ilcplex/cplex.h>

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
int xpos(int i, int j, instance *inst);
void print_error(const char *err);
void plot_gnuplot(instance *inst);
void hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int seed, double prob);
double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, double prob, double opt);


/*FUNZIONE CHE RESETTA TUTTI I LOWER BOUND DI TUTTE LE VARIABILI*/
void reset_lower_bound(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	printf("reset bounds \n");
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
	printf("SCRIVO MODEL RESET---------------------\n");
	CPXwriteprob(env, lp, "modelreset.lp", NULL);

	free(index0);
	free(bounds0);
	free(lb0);

}

/*---------------------------------------HARD FIXING LOOP--------------------------------------------*/
double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, double prob, double opt) {
	int fix = 1;
	int finded = 0;
	double opt_heu = opt;
	double opt_current;																//VALUE OPTIMAL SOL

	while (time(NULL) < timelimit) {
		if (fix == 1) {
			hard_fixing(inst, env, lp, time(NULL), prob);
			fix = 0;
		}
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");
		if (CPXgetstat(env, lp) == CPXMIP_INFEASIBLE) {
			printf("IMPOSSIBLE PROBLEM\n");
			reset_lower_bound(inst, env, lp);
			return -1;
		}
		if (CPXgetobjval(env, lp, &opt_current)) print_error("Error getting optimal value");
		printf("Object function optimal value is: %.0f\n", opt_current);
		if (opt_heu == opt_current) {
			printf("Equal optimal values, reset\n");
			reset_lower_bound(inst, env, lp);
			fix = 1;
		}
		else {
			if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
			printf("Different optimal values, continue\n");
			if (CPXgetobjval(env, lp, &opt_heu)) print_error("Error getting optimal value");
			printf("Object function optimal value is: %.0f\n", opt_heu);
		}
	}
	return opt_current;
}

/*-------------------FUNCTION TO SET THE LOWER BOUND OF THE SOLUTIONS'S VARIABLES--------------------*/
void hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int seed, double prob)
{
	srand(seed);														//SEED FOR RANDOM FUNCTION
	printf("hard fix bounds \n");
	int *index_set = (int *)malloc(inst->nnodes * sizeof(int));			//ARRAY OF INDEXES TO CHANGE BOUND
	double *bounds_set = (double *)calloc(inst->nnodes, sizeof(double));//ARRAY THAT CONTAIN THE NEW VALUE OF THE BOUND					
	char *lb_set = (char *)malloc(inst->nnodes * sizeof(char));			//ARRAY THAT SPECIFIES WHAT BOUND CHANGE FOR EACH VARIABLE
	int count = 0;
	for (int i = 0; i < inst->ncols; i++){
		if (inst->best_sol[i] == 1) {
			double random = (double)rand() / (double)RAND_MAX;
			if (random <= prob) {
				if (VERBOSE >=400)
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
	printf("Selected %d edges in %d\n", count, inst->nnodes);
	CPXchgbds(env, lp, count, index_set, lb_set, bounds_set);			//FUNCTION TO MODIFY BOUNDS TO THE VARIABLES
	
	CPXwriteprob(env, lp, "modelchanged.lp", NULL);
	
	free(index_set);
	free(bounds_set);
	free(lb_set);
}

/*-----------------------------FUNCTION TO UPDATE THE HEURISTIC SOLUTION-----------------------*/

void update_x_heu(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	double *current_sol = (double *)calloc(inst->ncols, sizeof(double));				//CURRENT SOLUTION 
	double opt_current_val;																//VALUE OPTIMAL SOL

	if (CPXgetobjval(env, lp, &opt_current_val)) print_error("Error getting optimal value");;													//OPTIMAL SOLUTION FOUND
	
	/*FIND CURRENT SOLUTION AND SAVE IN A TEMPORARY ARRAY*/
	if (CPXgetx(env, lp, current_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");

	/*IF THE VALUE OF THE OBJECTIVE FUNCTION IS BETTER RATHER THAN THE BEST SOLUTION THEN I UPDATE IT*/
	if (opt_current_val < inst->best_obj_val)
	{
		printf("Best HEURISTIC solution founded: %lf", opt_current_val);
		for (int i = 0; i < inst->ncols; i++)
		{
			inst->best_sol[i] = current_sol[i];
		}
	}
	free(current_sol);
}

/*FUNCTION THAT SET THE START SOLUTION (THE SOLUTION WILL BE TRIVIAL THAT IS  1->2->3->....->n)*/
void start_sol(instance *inst)
{
	printf("Set of the initial Heuristic Best Solution \n\n");
	inst->best_sol = (double *)calloc(inst->ncols, sizeof(double));
	for (int i = 0; i < inst->nnodes-1; i++)
	{
		inst->best_sol[xpos(i,i+1,inst)] = 1;
	}
	inst->best_sol[xpos(0, inst->nnodes - 1, inst)] = 1;

	if (VERBOSE >= 200) {
		for (int i = 0; i < inst->nnodes; i++) {
			for (int j = i + 1; j < inst->nnodes; j++) {
				if (inst->best_sol[xpos(i, j, inst)] > TOLERANCE) 
				{
					printf("Node (%d,%d) is selected \n", i + 1, j + 1);
				}
			}
		}
	}
}