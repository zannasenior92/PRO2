#include "TSP.h"
#include <ilcplex/cplex.h>

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
int xpos(int i, int j, instance *inst);


/*FUNZIONE CHE RESETTA TUTTI I LOWER BOUND DI TUTTE LE VARIABILI*/
void reset_lower_bound(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	int ncols = CPXgetnumcols(env, lp);
	int *index0 = (int*)malloc(ncols * sizeof(int));				//ARRAY DI INDICI A CUI CAMBIARE IL BOUND
	double *bounds0 = (double*)calloc(ncols, sizeof(double));				//ARRAY CHE CONTIENE IL NUOVO VALORE DEL BOUND				
	char *lb0 = (char*)malloc(ncols * sizeof(char));				//ARRAY CHE SPECIFICA QUALE BOUND CAMBIARE PER OGNI VARIABILE

	for (int i = 0; i < ncols; i++)
	{
		index0[i] = i;
	}
	for (int i = 0; i < ncols; i++)
	{
		lb0[i] = "L";
	}

	CPXchgbds(env, lp, inst->nnodes, index0, lb0, bounds0);			//FUNZIONE PER MODIFICARE IL BOUND ALLE VARIABILI
	free(index0);
	free(bounds0);
	free(lb0);

}


/*-------------------FUNCTION TO SET THE LOWER BOUND OF THE SOLUTIONS'S VARIABLES--------------------*/
void hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp)
{

	int ncols = CPXgetnumcols(env, lp);

	int *index_set = (int *)malloc(ncols * sizeof(int));				//ARRAY DI INDICI DELLE VARIABILI A CUI CAMBIARE IL BOUND
	double *bounds_set = (double *)calloc(ncols, sizeof(double));				//ARRAY CHE CONTIENE IL NUOVO VALORE DEL BOUND PER OGNI VARIABILE				
	char *lb_set = (char *)malloc(ncols * sizeof(char));				//ARRAY CHE SPECIFICA QUALE BOUND CAMBIARE PER OGNI VARIABILE

	int n = 0;
	for (int i = 0; i < ncols; i++)//SET AN ARRAY OF INDEX REFERRED TO THE VARIABLES THAT I WANT TO CHANGE
	{
		if (inst->best_sol[i] == 1)//SETTO A UNO SOLO SE E' UNA VARIABILE DELLA SOLUZIONE DEL PROBLEMA
		{
			index_set[n] = i;
		}
		n++;
	}
	for (int i = 0; i < ncols; i++)
	{
		if (inst->best_sol[i] == 1)//SETTO A UNO SOLO SE E' UNA VARIABILE DELLA SOLUZIONE DEL PROBLEMA
		{
			bounds_set[i] = rand() % 1;									//SETTO IL LOWER BOUND DI OGNI VARIABILE (0/1)
		}
	}

	for (int i = 0; i < ncols; i++)
	{
		lb_set[i] = "L";
	}

	CPXchgbds(env, lp, inst->nnodes - 1, index_set, lb_set, bounds_set);		//FUNZIONE PER MODIFICARE IL BOUND ALLE VARIABILI
	

	free(index_set);
	free(bounds_set);
	free(lb_set);
}

/*--------------------------------FUNCTION FRO SETTING THE INITIAL SOLUTION------------------------*/