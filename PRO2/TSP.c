/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void build_modelFlow1(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);
int xpos_compact(int i, int j, instance *inst);
int ypos(int i, int j, instance *inst);
double dist(int i, int j, instance *inst);


/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	build_modelFlow1(inst, env, lp);												//populate the model
	if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model

	int ncols = CPXgetnumcols(env, lp);
	printf("numero colonne %d\n", ncols);
	inst->best_sol= (double *)calloc(ncols, sizeof(double));				//best objective solution
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
		for (int j = 0; j < inst->nnodes; j++) {
			if (inst->best_sol[xpos_compact(i, j, inst)] > 0.5) {

				if (VERBOSE >= 1) {
					printf("Il nodo (%d,%d) e' selezionato\n", i + 1, j + 1);
				}
				/*--ADD EDGES(VECTOR LENGTH = 2*nnodes TO SAVE NODES OF EVERY EDGE)--*/
				inst->choosen_edge[n] = i;
				inst->choosen_edge[n + 1] = j;
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

	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	if (CPXgetobjval(env, lp, &inst->best_obj_val)) print_error("no best objective function");	//OPTIMAL SOLUTION FOUND
	printf("Object function optimal value is: %f\n", inst->best_obj_val);
	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT-----------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}

/*------------------------------BUILD CPLEX MODEL------------------------------------*/
void build_modelFlow1(instance *inst, CPXENVptr env, CPXLPptr lp) {

	double lb = 0.0;																	//lower bound
	char binary = 'B';																	//binary variable (0 OR 1)
	char integer = 'I';																	//integer variable

	/*-------------------------TO WRITE WELL THE MODEL-------------------------------*/
	char **cname = (char **)calloc(1, sizeof(char *));									// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));

	/*-------------------------DEFINE x -VARIABLES ON THE MODEL----------------------*/
	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = 0; j < inst->nnodes; j++)
		{

			double obj = dist(i, j, inst);
			sprintf(cname[0], "x(%d,%d)", i + 1, j + 1);								//PRINT VARIABLES IN CPLEX IN .lp FILE  
			double ub = (i == j) ? 0.0 : 1.0;

			/*--------------------PRINT DISTANCE d(i,j)-------------------*/
			if (VERBOSE >= 500) {
				printf("Distance d(%d,%d): %f \n", i + 1, j + 1, dist(i, j, inst));
			}

			/*----------------------INSERT VARIABLE IN CPLEX--------------*/
			if (CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) print_error(" wrong CPXnewcols on x var.s");
			/*--------------------CHECK VARIABLE POSITION-----------------*/
			if (CPXgetnumcols(env, lp) - 1 != xpos_compact(i, j, inst)) print_error(" wrong position for x var.s");

			if (VERBOSE >= 500)
			{
				printf("The column with i=%d e j=%d is in position %d and xpos is %d\n", i, j, CPXgetnumcols(env, lp), xpos_compact(i, j, inst));
			}
		}
	}
	inst->last_x_index = CPXgetnumcols(env, lp) - 1;									//LAST x INDEX(INDEXES START FROM 0)
	if (VERBOSE >= 200)
	{
		printf("Last x index in CPLEX is: %d (Remember indexes start from 0)\n", inst->last_x_index);
	}

	/*--------DEFINE y VARIABLES  yij= FLOW IN ARC (i,j) i!=j------------------------*/
	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = 0; j < inst->nnodes; j++) {
			double lbu = 0.0;									//LOWER BOUND
			double obj = 0;
			sprintf(cname[0], "y(%d,%d)", i + 1, j + 1);		//PRINT VARIABLES IN CPLEX IN .lp FILE 
			double ub = (i == j) ? 0.0 : inst->nnodes-1;
			
			/*--------------------INSERT VARIABLE IN CPLEX----------------*/
			if (CPXnewcols(env, lp, 1, &obj, &lbu, &ub, &integer, cname)) print_error(" wrong CPXnewcols on y(%d,%d) var.s",i,j);
			/*--------------------CHECK VARIABLE POSITION-----------------*/
			if (CPXgetnumcols(env, lp) - 1 != ypos(i, j, inst))	print_error(" wrong position for y var.s");
				
			if (VERBOSE >= 200)
			{
				printf("The column with i=%d e j=%d is in position %d and ypos is %d\n", i, j, CPXgetnumcols(env, lp), ypos(i, j, inst));
			}
		}
	}


	/*--------------------------------ADD CONSTRAINTS----------------------------*/

	/*--------------------------------IN DEGREE----------------------------------*/
	for (int h = 0; h < inst->nnodes; h++)

	{
		int lastrow = CPXgetnumrows(env, lp);
		if (VERBOSE >= 300)																//print every indeg
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
			if (CPXchgcoef(env, lp, lastrow, xpos_compact(i, h, inst), 1.0)) print_error(" wrong CPXchgcoef [x2]");
		}
	}


	/*--------------------------------OUT DEGREE----------------------------------*/
	for (int h = 0; h < inst->nnodes; h++)
	{
		int lastrow = CPXgetnumrows(env, lp);
		if (VERBOSE >= 300)																//print every outdeg
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
			if (CPXchgcoef(env, lp, lastrow, xpos_compact(h, i, inst), 1.0)) print_error(" wrong CPXchgcoef [x1]");
		}
	}
	
	/*----------------------------ADD LAZY CONSTRAINTS--------------*/
	/*--------------------------yij<=(n-1)*xij --------------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		int izero = 0;
		int *index = (int *)malloc(2* sizeof(int));
		double *value = (double *)malloc(2* sizeof(double));
		char sense = 'L';
		double rhs = 0;
		for (int j = 0; j < inst->nnodes; j++) {

			if (i == j) continue;
			index[0] = ypos(i, j, inst);										//VARIABLE'S  INDEX
			value[0] = 1.0;														//VARIABLE'S VALUE  
			index[1] = xpos_compact(i, j, inst);										//VARIABLE'S  INDEX
			value[1] = -(inst->nnodes-1);
			sprintf(cname[0], "y(%d,%d)", i + 1, j + 1);

			if (CPXaddlazyconstraints(env, lp, 1, 2, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");

		}
		free(index);
		free(value);
	}
	

	/*-----------------------------CONSTRAINTS ON y VARIABLES-----------------------*/

	/*------------------------------SUM_j y1j = n-1    j!=1-------------------------*/
	char sense = 'E';
	int izero = 0;
	int *index = (int *)malloc((inst->nnodes-1)*sizeof(int));
	double *value = (double *)malloc((inst->nnodes - 1) *sizeof(double));
	double rhs = inst->nnodes - 1;												//(n-1)
	sprintf(cname[0], "y1j(1,j)");

	for (int j = 1; j < inst->nnodes; j++) {

		index[j-1] = ypos(0,j, inst);											//INDEX OF THE COLUMN CORRESPOND TO THE VARIABLE
		value[j-1] = 1.0;	
	}
	
	if (CPXaddlazyconstraints(env, lp, 1, inst->nnodes-1, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");
	free(index);
	free(value);

	/*--------------------------SUM_j(yij)-SUM_k(yjk) = 1    j!=1-------------------*/
	for (int j = 1; j < inst->nnodes; j++) {
		char sense = 'E';
		int izero = 0;
		int *index = (int *)malloc((2*inst->nnodes - 2) * sizeof(int));
		double *value = (double *)malloc((2*inst->nnodes - 2) * sizeof(double));
		double rhs = 1;
		sprintf(cname[0], "yij(i,%d)_yjk(%d,k)",j+1,j+1);

		/*----LAZY INSERTION FUNCTION NEEDS ARRAY OF INDEXES AND ARRAY OF RELATED VALUES------*/
		int lazy_index = 0;
		for (int i = 0; i < inst->nnodes; i++) {											//FIRST SUM
			if (i == j) continue;
			index[lazy_index] = ypos(i, j, inst);											//INDEX OF THE COLUMN CORRESPOND TO THE VARIABLE
			value[lazy_index] = 1.0;
			lazy_index++;
		}
		for (int k = 0; k < inst->nnodes; k++) {											//SECOND SUM
			if (k == j) continue;
			index[lazy_index] = ypos( j, k, inst);											//INDEX OF THE COLUMN CORRESPOND TO THE VARIABLE
			value[lazy_index] = -1.0;
			lazy_index++;
		}
		if (VERBOSE >= 1 & (j == inst->nnodes - 1)) 
		{
			printf("Last number of lazy_index: %d \n", lazy_index);
		}
		if (CPXaddlazyconstraints(env, lp, 1, (2 * inst->nnodes - 2), &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");
		free(index);
		free(value);

	}
	/*-------------------write the cplex model in file model.lp------------------*/
	CPXwriteprob(env, lp, "modelFlow1.lp", NULL);
}