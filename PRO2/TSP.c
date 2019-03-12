/*
File che contiene le funzioni che usano cplex
*/
#include "TSP.h"
#include <ilcplex/cplex.h>


//-----------------------------FUNCTIONS & METHODS-----------------------------------
/*void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
//Funzione che mi restituisce la posizione della variabile all'interno del modello
int xpos(int i, int j, instance *inst) {
	if (i > j) return xpos(j, i, inst);
	return i * inst->nnodes + j-((i+1)*(i+2)/2);
}
*/

//Funzione per trovare la distanza tra due punti; ritorna un double
double dist(int i, int j, instance *inst){
	double dx = inst->xcoord[i] - inst->xcoord[j];
	double dy = inst->ycoord[i] - inst->ycoord[j];
	return (int)(sqrt(dx*dx + dy * dy)+0.5);
}



//--------------------------------------------------------------------------------
/*/int TSPopt(instance *inst)
{
	// open cplex model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error); //create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); //create the structure for our model(lp)
	build_model(inst, env, lp);
	if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");

	int ncols = CPXgetnumcols(env, lp);
	inst->best_sol= (double *)calloc(ncols, sizeof(double));
	if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
	if(VERBOSE>=200){
		for (int i = 0; i < ncols - 1; i++) {
			printf("Best %f\n", inst->best_sol[i]);
		}
	}
	//CPXgetbestobjval(env, lp, &inst->best_lb);
	
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}

//--------------------------------------------------------------------------------
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
	//Definisco alcune variabili
	double zero = 0.0; // one = 1.0;
	double ub = 1.0;
	char binary = 'B';
	//char continuous = 'C';
	//Definisco cname per scrivere il modello in modo più chiaro
	char **cname = (char **)calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));

	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = i+1; j < inst->nnodes; j++)
		{
			sprintf(cname[0], "x(%d,%d)", i + 1, j + 1);
			double obj = dist(i, j, inst);
			//Metodo per inserire colonna: env=environment, lp=problema, obj=funzione obiettivo, 
			// zero=lower bound, ub=upper bound, binary=tipo della variabile, cname=nome della colonna
			if (CPXnewcols(env, lp, 1, &obj, &zero, &ub, &binary, cname)) print_error(" wrong CPXnewcols on x var.s");
			//confronto se la posizione della colonna aggiunta sia uguale a quella della xpos
			//printf("La colonna con i=%d e j=%d e' in posizione %d e xpos e' %d\n", i, j, CPXgetnumcols(env, lp), xpos(i,j,inst));
			//-------------PROBLEMI CON XPOS
			if (CPXgetnumcols(env, lp) - 1 != xpos(i, j, inst)) print_error(" wrong position for x var.s");

		}
	}
	//AGGIUNGO I VINCOLI
	for (int h = 0; h < inst->nnodes; h++)  // out-degree ciclo esterno per ogni vincolo che voglio aggiungere per nodo h
	{
		int lastrow = CPXgetnumrows(env, lp);	//chiedo a cplex ultima riga cambiata chiedendo numero di righe
		if (VERBOSE >= 200) {
			printf("lastrow %d\n", lastrow);
		}
		double maxdeg = 2.0; 	 	//NOI vogliamo 2 uno entrante e uno uscente
		char sense = 'E'; 			//// E equazione
		sprintf(cname[0], "degree(%d)", h + 1);   // DO un nome NOI degree 
		if (CPXnewrows(env, lp, 1, &maxdeg, &sense, NULL, cname)) print_error(" wrong CPXnewrows [x1]");  //Nuova riga vuota con coeff diversi da 0 e con informazioni nella posizione last row 																posizione last row
		for (int i = 0; i < inst->nnodes; i++)		//cambio coefficienti non 0 mettendoli a 1 NOI se i=h salto istruzione, se i!=h faccio chgcoef change coeff a 1
									// non importa se i>h perché xpos fa inversione
		{
			if (i == h)
				continue;
			else
				if (CPXchgcoef(env, lp, lastrow, xpos(i, h, inst), 1.0)) print_error(" wrong CPXchgcoef [x1]");
		}
	}
	CPXwriteprob(env, lp, "model.lp", NULL);
}
*/