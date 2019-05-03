#include "TSP.h"
#include <ilcplex/cplex.h>

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
int xpos(int i, int j, instance *inst);


/*FUNZIONE CHE RESETTA TUTTI I LOWER BOUND DI TUTTE LE VARIABILI*/
void reset_lower_bound(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	int ncols = CPXgetnumcols(env, lp);
	int *index = (int*)malloc(ncols * sizeof(int));				//ARRAY DI INDICI A CUI CAMBIARE IL BOUND
	int *bounds = (int*)calloc(ncols, sizeof(int));				//ARRAY CHE CONTIENE IL NUOVO VALORE DEL BOUND				
	char *lb = (char*)malloc(ncols * sizeof(char));				//ARRAY CHE SPECIFICA QUALE BOUND CAMBIARE PER OGNI VARIABILE

	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = 0; j < inst->nnodes; j++)
		{
			index[i+j] = xpos(i, j, inst);
		}
	}
	for (int i = 0; i < ncols; i++)
	{
		lb[i] = "L";
	}
	
	CPXchgbds(env, lp, inst->nnodes,index, lb, bounds);			//FUNZIONE PER MODIFICARE IL BOUND ALLE VARIABILI
	free(index);
	free(bounds);
	free(lb);

}


/*-------------------FUNCTION TO SET THE LOWER BOUND OF THE SOLUTIONS'S VARIABLES--------------------*/
void hard_fixing(instance *inst, CPXENVptr env, CPXLPptr lp, int *ind)
{

	int ncols = CPXgetnumcols(env, lp);

	int *index = (int*)malloc(ncols * sizeof(int));				//ARRAY DI INDICI DELLE VARIABILI A CUI CAMBIARE IL BOUND
	int *bounds = (int*)calloc(ncols, sizeof(int));				//ARRAY CHE CONTIENE IL NUOVO VALORE DEL BOUND PER OGNI VARIABILE				
	char *lb = (char*)malloc(ncols * sizeof(char));				//ARRAY CHE SPECIFICA QUALE BOUND CAMBIARE PER OGNI VARIABILE

	int n = 0;
	for (int i = 0; i < ncols; i++)//SET AN ARRAY OF INDEX REFERRED TO THE VARIABLES THAT I WANT TO CHANGE
	{
		if (inst->best_sol[i] == 1)//SETTO A UNO SOLO SE E' UNA VARIABILE DELLA SOLUZIONE DEL PROBLEMA
		{
			index[n] = i;
		}
		n++;
	}
	for (int i = 0; i < ncols; i++)
	{
		if (inst->best_sol[i] == 1)//SETTO A UNO SOLO SE E' UNA VARIABILE DELLA SOLUZIONE DEL PROBLEMA
		{
			bounds[i] = rand() % 1;									//SETTO IL LOWER BOUND DI OGNI VARIABILE (0/1)
		}
	}

	for (int i = 0; i < ncols; i++)
	{
		lb[i] = "L";
	}

	CPXchgbds(env, lp, inst->nnodes-1, index, lb, bounds);		//FUNZIONE PER MODIFICARE IL BOUND ALLE VARIABILI
	free(index);
	free(bounds);
	free(lb);

	/*WRITE IN A FILE THE MODIFIED MODEL*/
	CPXwriteprob(env, lp, "modelchanged.lp", NULL);
}

/*--------------------------------FUNCTION FRO SETTING THE INITIAL SOLUTION------------------------*/