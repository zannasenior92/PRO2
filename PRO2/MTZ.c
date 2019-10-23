#include "TSP.h"
#include "ilcplex/cplex.h"

/*------------------------------BUILD CPLEX MODEL------------------------------------*/
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp) {

	double lb = 0.0;																	//lower bound
	char binary = 'B';																	//binary variable (0 OR 1)
	char integer = 'I';

	/*-------------------------TO WRITE WELL THE MODEL-------------------------------*/
	char **cname = (char **)calloc(1, sizeof(char *));									// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));

	/*-------------------------DEFINE VARIABLES ON THE MODEL----------------------*/
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
	if (VERBOSE >= 500)
	{
		printf("Last x index in CPLEX is: %d \n", inst->last_x_index);
	}


	/*------------------------------------ADD CONSTRAINTS-----------------------------------------*/

	/*--------ADD THE u VARIABLES- ui=i POSITION IN THE CIRCUIT-----------*/
	for (int i = 0; i < inst->nnodes; i++)
	{
		double lbu = (i == 0) ? 1.0 : 2.0;												//LOWER BOUND
		double obj = 0;
		sprintf(cname[0], "u(%d)", i + 1);												//PRINT VARIABLES IN CPLEX IN .lp FILE 
		double ub = (i == 0) ? 1.0 : inst->nnodes;

		/*--------------------INSERT VARIABLE IN CPLEX----------------*/
		if (CPXnewcols(env, lp, 1, &obj, &lbu, &ub, &integer, cname)) print_error(" wrong CPXnewcols on u var.s");
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

	/*----------------------------ADD VARIABLES FOR LAZY CONSTRAINTS--------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		/*--------------y_ij + y_ji <= 1 for all i<j-----------------*/

		int izero = 0;
		int *index = (int *)malloc(2 * sizeof(int));
		double *value = (double *)malloc(2 * sizeof(double));
		double rhs = 1.0;
		char sense = 'L';


		for (int j = i + 1; j < inst->nnodes; j++) {

			if (i == j) continue;
			index[0] = xpos_compact(i, j, inst);										//VARIABLE'S  INDEX
			value[0] = 1.0;														//VARIABLE'S VALUE  
			index[1] = xpos_compact(j, i, inst);
			value[1] = 1.0;
			sprintf(cname[0], "link(%d,%d)", i + 1, j + 1);


			if (CPXaddlazyconstraints(env, lp, 1, 2, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");
		}
		free(index);
		free(value);
	}

	int izero = 0;
	int *index = (int *)malloc(1 * sizeof(int));
	double *value = (double *)malloc(1 * sizeof(double));
	double rhs = 1.0;
	char sense = 'E';
	index[0] = upos(0, inst);													//INSERT u1=1 (indexes start from 0)
	value[0] = 1.0;
	sprintf(cname[0], "u1(1)");
	if (CPXaddlazyconstraints(env, lp, 1, 1, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");


	/*-----------------------------CONSTRAINTS ON u VARIABLES-----------------------*/

	/*------------------------ui-uj+M*yij<=M-1--------------------- */
	for (int i = 1; i < inst->nnodes; i++) {

		char sense = 'L';
		int izero = 0;
		int *index = (int *)malloc(3 * sizeof(int));
		double *value = (double *)malloc(3 * sizeof(double));

		for (int j = 1; j < inst->nnodes; j++) {
			if (i == j) continue;
			double big_M = (double)inst->nnodes - 1;
			double rhs = big_M - 1;

			sprintf(cname[0], "uij(%d,%d)", i + 1, j + 1);
			index[0] = upos(i, inst);											//INDEX OF THE COLUMN CORRESPOND TO THE VARIABLE
			value[0] = 1.0;														//SET TO 1 VARIABLE'S VALUE  
			index[1] = (upos(j, inst));
			value[1] = -1.0;
			index[2] = xpos_compact(i, j, inst);
			value[2] = big_M;
			if (CPXaddlazyconstraints(env, lp, 1, 3, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");

		}
		free(index);
		free(value);
	}



	/*-------------------write the cplex model in file model.lp------------------*/
	CPXwriteprob(env, lp, "modelMTZ.lp", NULL);
}