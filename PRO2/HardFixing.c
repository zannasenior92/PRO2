#include "TSP.h"
#include <ilcplex/cplex.h>

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
int xpos(int i, int j, instance *inst);


/*FUNZIONE CHE RESETTA TUTTI I LOWER BOUND DI TUTTE LE VARIABILI*/
void reset_lower_bound(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	printf("reset bounds \n");
	int *index0 = (int*)malloc(inst->nnodes * sizeof(int));				//ARRAY DI INDICI A CUI CAMBIARE IL BOUND
	double *bounds0 = (double*)calloc(inst->nnodes, sizeof(double));				//ARRAY CHE CONTIENE IL NUOVO VALORE DEL BOUND				
	char *lb0 = (char*)malloc(inst->nnodes * sizeof(char));				//ARRAY CHE SPECIFICA QUALE BOUND CAMBIARE PER OGNI VARIABILE

	int n = 0;
	for (int i = 0; i < inst->ncols; i++)//SET AN ARRAY OF INDEX REFERRED TO THE VARIABLES THAT I WANT TO CHANGE
	{
		if (inst->best_sol[i] == 1)//SETTO A UNO SOLO SE E' UNA VARIABILE DELLA SOLUZIONE DEL PROBLEMA
		{
			index0[n] = i;
			n++;
		}
	}
	for (int i = 0; i < inst->nnodes; i++)
	{
		lb0[i] = 'L';
	}
	CPXchgbds(env, lp, n, index0, lb0, bounds0);			//FUNZIONE PER MODIFICARE IL BOUND ALLE VARIABILI
	printf("SCRIVO MODEL RESET---------------------\n");
	CPXwriteprob(env, lp, "modelreset.lp", NULL);

	free(index0);
	free(bounds0);
	free(lb0);

}


/*-------------------FUNCTION TO SET THE LOWER BOUND OF THE SOLUTIONS'S VARIABLES--------------------*/
void hard_fixing(CPXENVptr env, CPXLPptr lp, instance *inst, int seed, double prob)
{
	srand(seed);
	printf("hard fix bounds \n");
	int *index_set = (int *)malloc(inst->nnodes * sizeof(int));			//ARRAY DI INDICI DELLE VARIABILI A CUI CAMBIARE IL BOUND
	double *bounds_set = (double *)calloc(inst->nnodes, sizeof(double));		//ARRAY CHE CONTIENE IL NUOVO VALORE DEL BOUND PER OGNI VARIABILE				
	char *lb_set = (char *)malloc(inst->nnodes * sizeof(char));				//ARRAY CHE SPECIFICA QUALE BOUND CAMBIARE PER OGNI VARIABILE
	int count = 0;
	for (int i = 0; i < inst->ncols; i++){
		if (inst->best_sol[i] == 1) {
			double random = (double)rand() / (double)RAND_MAX;
			if (random > prob) {
				printf("Seleziono lato %d\n",i);
				index_set[count] = i;
				bounds_set[count] = 1.0;
				lb_set[count] = 'L';
				count++;
			}
			
		}
	}
	printf("selezionati %d lati su %d\n", count, inst->nnodes);
	CPXchgbds(env, lp, count, index_set, lb_set, bounds_set);		//FUNZIONE PER MODIFICARE IL BOUND ALLE VARIABILI
	/*
	int n = 0;
	for (int i = 0; i < inst->ncols; i++)//SET AN ARRAY OF INDEX REFERRED TO THE VARIABLES THAT I WANT TO CHANGE
	{
		if (inst->best_sol[i] == 1)//SETTO A UNO SOLO SE E' UNA VARIABILE DELLA SOLUZIONE DEL PROBLEMA
		{
			index_set[n] = i;
			n++;
		}
	}
	for (int i = 0; i < inst->nnodes; i++)
	{
		if (inst->best_sol[i] == 1)//SETTO A UNO SOLO SE E' UNA VARIABILE DELLA SOLUZIONE DEL PROBLEMA
		{
			bounds_set[i] = rand() % 2;									//SETTO IL LOWER BOUND DI OGNI VARIABILE (0/1)
		}
	}

	for (int i = 0; i < inst->nnodes; i++)
	{
		lb_set[i] = 'L';
	}
	
	CPXchgbds(env, lp, inst->nnodes, index_set, lb_set, bounds_set);		//FUNZIONE PER MODIFICARE IL BOUND ALLE VARIABILI
	*/
	CPXwriteprob(env, lp, "modelchanged.lp", NULL);

	free(index_set);
	free(bounds_set);
	free(lb_set);
}

double loop_hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, double timelimit, double prob, double opt) {
	int fix = 1;
	int finded = 0;
	double opt_heu = opt;
	double opt_current;																//VALUE OPTIMAL SOL

	while (time(NULL) < timelimit) {
		if (fix == 1) {
			hard_fixing(env, lp, inst, time(NULL), prob);
			fix = 0;
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

		}
		else {
			if (CPXgetx(env, lp, inst->best_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");
			printf("Valori ottimi diversi, continuo\n");
			if (CPXgetobjval(env, lp, &opt_heu)) print_error("Error getting optimal value");
			printf("Object function optimal value is: %.0f\n", opt_heu);

		}
	}
	return opt_current;
}

/*-----------------------------FUNCTION TO UPDATE THE HEURISTIC SOLUTION-----------------------*/

void update_x_heu(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	double *current_sol = (double *)calloc(inst->ncols, sizeof(double));				//CURRENT SOLUTION 
	double opt_current_val;																//VALUE OPTIMAL SOL

	if (CPXgetobjval(env, lp, &opt_current_val)) print_error("Error getting optimal value");;													//OPTIMAL SOLUTION FOUND
	
	/*MI TROVO LA SOLUZIONE CORRENTE E LA SALVO IN UN ARRAY TEMPORANEO*/
	if (CPXgetx(env, lp, current_sol, 0, inst->ncols - 1)) print_error("no solution avaialable");

	/*SE IL VALORE DELLA FUNZIONE OBIETTIVO CORRENTE E' MIGLIORE DI QUELLA OTTIMA ALLORA LA AGGIORNO*/
	//if (opt_current_val < inst->best_obj_val)
	//{
		//printf("Best HEURISTIC solution founded: %lf", opt_current_val);
	for (int i = 0; i < inst->ncols; i++)
	{
		inst->best_sol[i] = current_sol[i];
	}
	//}
	free(current_sol);
}

/*FUNZIONE CHE SETTA LA SOLUZIONE DI PARTENZA(LA SOLUZIONE SARA' BANALE OVVERO  1->2->3->....->n)*/
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