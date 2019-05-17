#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void print_error(const char *err);

/*********************************** DEFAULT MODEL **********************************/
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {

	double lb = 0.0;																	//lower bound
	double ub = 1.0;																	//upper bound
	char binary = 'B';																	//binary variable (0 OR 1)
																						//char continuous = 'C';

	char **cname = (char **)calloc(1, sizeof(char *));									// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));

	/*-------------------------DEFINE VARIABLES ON THE MODEL----------------------*/
	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = i + 1; j < inst->nnodes; j++)
		{
			double obj = dist(i, j, inst);
			sprintf(cname[0], "x(%d,%d)", i + 1, j + 1);								//PRINT VARIABLES IN CPLEX IN .lp FILE 

			/*--------------------PRINT DISTANCE d(i,j)-------------------*/
			if (DEFAULT_MODEL >= 500) {
				printf("Distance d(%d,%d): %f \n", i + 1, j + 1, dist(i, j, inst));
			}


			/*----------------------INSERT VARIABLE IN CPLEX--------------*/
			if (CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) print_error(" wrong CPXnewcols on x var.s");

			/*--------------------CHECK VARIABLE POSITION-----------------*/
			if (CPXgetnumcols(env, lp) - 1 != xpos(i, j, inst)) print_error(" wrong position for x var.s");
			if (DEFAULT_MODEL >= 500)
			{
				printf("The column with i=%d e j=%d is in position %d and xpos is %d\n", i, j, CPXgetnumcols(env, lp), xpos(i, j, inst));
			}
		}
	}


	/*--------------------------------ADD CONSTRAINTS----------------------------*/
	for (int h = 0; h < inst->nnodes; h++)
	{
		int lastrow = CPXgetnumrows(env, lp);										//LAST ROW NUMBER(to insert the new constraint)
		if (DEFAULT_MODEL >= 200) {
			printf("lastrow %d\n", lastrow);
		}
		double maxdeg = 2.0; 	 													//DEGREE OF EVERY NODE
		char sense = 'E'; 															// E = EQUATION
		sprintf(cname[0], "degree(%d)", h + 1);										// NAME DEGREE CONSTRAINT INSIDE THE MODEL 
		/*-----------------------------NEW CONSTRAINT ROW------------------------*/
		if (CPXnewrows(env, lp, 1, &maxdeg, &sense, NULL, cname)) print_error(" wrong CPXnewrows [x1]");
		for (int i = 0; i < inst->nnodes; i++)
		{
			if (i == h)																// TO SKIP x(i,i) COEFFICIENTS
				continue;
			else
				/*-------SET TO 1 THE VARIABLES COEFFICIENT IN THE EQUATION------*/
				if (CPXchgcoef(env, lp, lastrow, xpos(i, h, inst), 1.0)) print_error(" wrong CPXchgcoef [x1]");
		}
	}

	/*---------SUBTOUR ELIMINATION-----------
	
	int nnz = 0;
	double rhs;
	char sense = 'L';
	int ncols = CPXgetnumcols(env, lp);
	
	int matbeg = 0;
	cname[0] = (char *)calloc(100, sizeof(char));
	for (int s = 2; s < inst->nnodes; s++) {
		rhs = s - 1;
		sprintf(cname[0], "SEC_Size(%d)", s);
		int *index = (int *)malloc(s * sizeof(int));
		double *value = (double *)malloc(s * sizeof(double));
		for (int i = 0; i < inst->nnodes; i++) {
			for (int j = i+1; j < inst->nnodes; j++)
			{
				index[nnz] = xpos(i, j, inst);
				value[nnz] = 1.0;
				nnz++;
				if (nnz == s) {
					if (CPXaddrows(env, lp, 0, 1, s, &rhs, &sense, &matbeg, index, value, NULL, cname)) print_error("wrong CPXaddrow");
					nnz = 0;
					CPXwriteprob(env, lp, "model.lp", NULL);

				}

			}

		}
		free(index);
		free(value);
	}
	*/

	/*-------------------write the cplex model in file model.lp------------------*/
	CPXwriteprob(env, lp, "model.lp", NULL);
}