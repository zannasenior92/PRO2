/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);

/*------------------POSITION OF VARIABLE INSIDE THE MODEL----------------------------*/
int xpos(int i, int j, instance *inst) {
	return i * inst->nnodes + j;
}
int upos(int i, instance *inst) {
	return inst->nnodes*inst->nnodes + i;
}



/*-------------------------DISTANCE BETWEEN TWO POINTS-------------------------------*/
double dist(int i, int j, instance *inst){
	double dx = inst->xcoord[i] - inst->xcoord[j];
	double dy = inst->ycoord[i] - inst->ycoord[j];
	double rij = sqrt((dx*dx+dy*dy)/10.0);
	int tij = (int)(rij + 0.5);
	if (tij < rij)
		return (tij + 1);
	else
		return tij;
}

double dist_euc(int i, int j, instance *inst) {
	double dx = inst->xcoord[i] - inst->xcoord[j];
	double dy = inst->ycoord[i] - inst->ycoord[j];
	return (int)(sqrt((dx*dx + dy * dy))+0.5);
	}



/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	
	int error;
	CPXENVptr env = CPXopenCPLEX(&error); //create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); //create the structure for our model(lp)
	build_model(inst, env, lp); //populate the model
	if (CPXmipopt(env, lp)) print_error("Error resolving the model\n"); //CPXmipopt to solve the model

	int ncols = CPXgetnumcols(env, lp);
	printf("numero colonne %d\n", ncols);
	inst->best_sol= (double *)calloc(ncols, sizeof(double)); //best objective solution
	if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
	
	if(VERBOSE>=200){
		for (int i = 0; i < ncols - 1; i++){
			printf("Best %f\n", inst->best_sol[i]);
		}
	}
	int count = 0;
	int n = 0;
	/*-------------------PRINT SELECTED EDGES(remember cplex tolerance)--------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (inst->best_sol[xpos(i, j, inst)] > 0.5) {

				if (VERBOSE >= 1) {
					printf("Il nodo (%d,%d) e' selezionato\n", i + 1, j + 1);
				}
				//Aggiungo i nodi a due a due, cosi so che ad ogni coppia corrisponde un arco
				inst->choosen_edge[n] = i;	//Uso un vettore lungo 2*nnodes per salvare i nodi corrispondenti agli archi
				inst->choosen_edge[n + 1] = j; //scelti. Cosi aggiorno il file per il plot una sola volta e lo sovrascrivo.
				n += 2;
				count++;
			}
		}
	}
	add_edge_to_file(inst);

	if (VERBOSE >= 1) {
		printf("Selected nodes: %d \n", count);
	}
	/*-------------------------------------------------------------------------------*/

	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT-----------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}

/*------------------------------BUILD CPLEX MODEL------------------------------------*/
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {

	double lb = 0.0; //lower bound
	char binary = 'B'; //binary variable (0 OR 1)
	char integer = 'I';

	//Definisco cname per scrivere il modello in modo più chiaro
	char **cname = (char **)calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));

	/*-------------------------DEFINE VARIABLES ON THE MODEL----------------------*/
	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = 0; j < inst->nnodes; j++)
		{
			
			double obj = dist(i, j, inst);
			sprintf(cname[0], "x(%d,%d)", i + 1, j + 1);//print variables on cplex 
			double ub = (i == j) ? 0.0 : 1.0;
			/*--------------------PRINT DISTANCE d(i,j)-------------------*/
			if (VERBOSE >= 500) {
				printf("Distance d(%d,%d): %f \n",i+1,j+1, dist(i, j,inst));
			}

			//Metodo per inserire colonna: env=environment, lp=problema, obj=funzione obiettivo, 
			// lb=lower bound, ub=upper bound, binary=tipo della variabile, cname=nome della colonna
			if (CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) print_error(" wrong CPXnewcols on x var.s");
			//confronto se la posizione della colonna aggiunta sia uguale a quella della xpos
			//printf("La colonna con i=%d e j=%d e' in posizione %d e xpos e' %d\n", i, j, CPXgetnumcols(env, lp), xpos(i,j,inst));
			if (CPXgetnumcols(env, lp) - 1 != xpos(i, j, inst)) print_error(" wrong position for x var.s");
		}
	}
	/*--------ADD THE u VARIABLES dove ui=posizione di i nel circuito-----------*/
	for (int i = 0; i < inst->nnodes; i++)
	{
		double lbu = (i == 0) ? 1.0 : 2.0;
		//double lbu = 2.0; //lower bound u
		double obj = 0;
		sprintf(cname[0], "u(%d)", i + 1);//print variables on cplex 
		double ub = (i == 0) ? 1.0 : inst->nnodes;
		//double ub = inst->nnodes;
		
		//Metodo per inserire colonna: env=environment, lp=problema, obj=funzione obiettivo, 
		// lb=lower bound, ub=upper bound, binary=tipo della variabile, cname=nome della colonna
		if (CPXnewcols(env, lp, 1, &obj, &lbu, &ub, &integer, cname)) print_error(" wrong CPXnewcols on u var.s");
	}


	/*--------------------------------ADD CONSTRAINTS----------------------------*/
	
	for (int h = 0; h < inst->nnodes; h++) //-----------------------------in-degree
		/*(For every node h, the sum of all the incoming arcs (i,h) must be 1)*/
	{
		int lastrow = CPXgetnumrows(env, lp);
		if(VERBOSE>=300)//print every indeg
		{
			printf("indeg(%d) \n", h + 1);
		}
		
		double rhs = 1.0;
		char sense = 'E';
		sprintf(cname[0], "indeg(%d)", h + 1);
		if (CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) print_error(" wrong CPXnewrows [x2]");
		for (int i = 0; i < inst->nnodes; i++)
		{
			if (i == h) continue;
			if (CPXchgcoef(env, lp, lastrow, xpos(i, h, inst), 1.0)) print_error(" wrong CPXchgcoef [x2]");
		}
	}
	for (int h = 0; h < inst->nnodes; h++) //----------------------------out-degree 
		/*(For every node h, the sum of all the outgoings arcs (h,j) must be 1)*/
	{
		int lastrow = CPXgetnumrows(env, lp);
		if (VERBOSE >= 300)//print every outdeg
		{
			printf("outdeg(%d) \n", h + 1);
		}

		double rhs = 1.0;
		char sense = 'E';
		sprintf(cname[0], "outdeg(%d)", h + 1);
		if (CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) print_error(" wrong CPXnewrows [x1]");
		for (int i = 0; i < inst->nnodes; i++)
		{
			if (i == h) continue;
			if (CPXchgcoef(env, lp, lastrow, xpos(h, i, inst), 1.0)) print_error(" wrong CPXchgcoef [x1]");
		}
	}

	/*---------------------------y_ij + y_ji <= 1 for all i<j--------------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		//variabili per poter inserire i lazy
		int izero = 0;
		int *index = (int *)malloc(2 * sizeof(int));
		double *value = (double *)malloc(2 * sizeof(double));
		double rhs = 1.0;
		char sense = 'L';

		for (int j = i+1; j < inst->nnodes; j++) {
			//Each entry, index[i], specifies the column index of the corresponding coefficient, value[i]
			if (i == j) continue;
			// ho cercato di inserire i lazy ma non ci capisco na mazza di come funziona la funzione CPXaddlazyconstraint
			index[0] = xpos(i, j, inst); //devo inserirci l'indice della colonna ovvero della variabile
			value[0] = 1.0; //setto a 1 il valore della variabile  
			index[1] = xpos(j, i, inst);
			value[1] = 1.0;
			sprintf(cname[0], "link(%d,%d)", i + 1, j + 1);
			/*environment, lp problem, nuber of lazy constraints to insert, */

			if (CPXaddlazyconstraints(env, lp, 1, 2, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");
			
			/*INSERIMENTO STATICO
			if (CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) print_error(" wrong CPXnewrows [l3]");
			if (CPXchgcoef(env, lp, lastrow, xpos(i, j, inst), 1.0)) print_error(" wrong CPXchgcoef [l3]");
			if (CPXchgcoef(env, lp, lastrow, xpos(j, i, inst), 1.0)) print_error(" wrong CPXchgcoef [l3]");
			*/
		}
		free(index);
		free(value);
	}
	/* INSERISCO u1=1*/
	int izero = 0;
	int *index = (int *)malloc(1 * sizeof(int));
	double *value = (double *)malloc(1* sizeof(double));
	double rhs = 1.0;
	char sense = 'E';
	index[0] = upos(0, inst);
	value[0] = 1.0;
	sprintf(cname[0], "u1(1)");
	if (CPXaddlazyconstraints(env, lp, 1, 1, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");

	/*-------------CONSTRAINTS ON u VARIABLES-----------------------*/
	/*------------  ui-uj+M*yij<=M-1 -------------------- */
	
	for (int i = 1; i < inst->nnodes; i++) {
		
		char sense = 'L';
		int izero = 0;
		int *index = (int *)malloc(3 * sizeof(int));
		double *value = (double *)malloc(3 * sizeof(double));

		for (int j = 1; j < inst->nnodes; j++) {
			if (i == j) continue;
			double big_M = (double)inst->nnodes - 1;
			double rhs = big_M - 1;

			sprintf(cname[0], "uij(%d,%d)", i + 1, j+1);
			index[0] = upos(i,inst); //devo inserirci l'indice della colonna ovvero della variabile
			printf("upos=%d\n", index[0]);
			value[0] = 1.0; //setto a 1 il valore della variabile  
			index[1] = (upos(j,inst));
			value[1] = -1.0;
			index[2] = xpos(i,j,inst);
			value[2] = big_M;
			//inst->u[i] - inst->u[i] + big_M * xpos(i, j, inst);
			if (CPXaddlazyconstraints(env, lp, 1, 3, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");

		}

	}
	
	

	CPXwriteprob(env, lp, "model.lp", NULL); //write the cplex model in file model.lp
}
