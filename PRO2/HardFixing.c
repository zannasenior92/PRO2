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
	
	for (int i = 0; i < inst->nnodes; i++)
	{
		CPXchgbds(env, lp, inst->nnodes,index, lb, bounds);//FUNZIONE PER MODIFICARE IL BOUND ALLE VARIABILI
	}
}