/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_plot(int i, int j, instance *inst);
void add_edge_to_file(instance *inst);


/*------------------POSITION OF VARIABLE INSIDE THE MODEL----------------------------*/
int xpos(int i, int j, instance *inst) {
	if (i > j) return xpos(j, i, inst);
	return i * inst->nnodes + j-((i+1)*(i+2)/2);
}


/*-------------------------DISTANCE BETWEEN TWO POINTS-------------------------------*/
double dist(int i, int j, instance *inst) {
	if (inst->dist_type == 0) {
		double dx = inst->xcoord[i] - inst->xcoord[j];
		double dy = inst->ycoord[i] - inst->ycoord[j];
		return (int)(sqrt((dx*dx + dy * dy)) + 0.5);
	}
	if (inst->dist_type == 1) {
		double dx = inst->xcoord[i] - inst->xcoord[j];
		double dy = inst->ycoord[i] - inst->ycoord[j];
		double rij = sqrt((dx*dx + dy * dy) / 10.0);
		int tij = (int)(rij + 0.5);
		if (tij < rij)
			return (tij + 1);
		else
			return tij;
	}
	if (inst->dist_type == 2) {
		double PI = 3.141592;
		double deg = (int)(inst->xcoord[i]);
		double min = inst->xcoord[i] - deg;
		double lati = PI * (deg + 5.0*min / 3.0) / 180.0;
		deg = (int)(inst->ycoord[i] + 0.5);
		min = inst->ycoord[i] - deg;
		double longi = PI * (deg + 5.0*min / 3.0) / 180.0;

		deg = (int)(inst->xcoord[j]);
		min = inst->xcoord[j] - deg;
		double latj = PI * (deg + 5.0*min / 3.0) / 180.0;
		deg = (int)(inst->ycoord[j] + 0.5);
		min = inst->ycoord[j] - deg;
		double longj = PI * (deg + 5.0*min / 3.0) / 180.0;

		double RRR = 6378.388;
		double q1 = cos(longi - longj);
		double q2 = cos(lati - latj);
		double q3 = cos(lati + latj);
		int dij = (int)(RRR*acos(0.5*((1.0 + q1)*q2 - (1.0 - q1)*q3)) + 1.0);
		return dij;
	}
	else print_error("Something go wrong in dist function");

}



/*------------------------------------SOLVE THE MODEL--------------------------------------------------------------*/
int TSPopt(instance *inst)
{
	
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);												//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");									//create the structure for our model(lp)
	build_model(inst, env, lp);															//populate the model
	if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");					//CPXmipopt to solve the model

	int ncols = CPXgetnumcols(env, lp);
	
	inst->best_sol= (double *)calloc(ncols, sizeof(double));							//best objective solution
	if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
	
	if(VERBOSE>=200){
		for (int i = 0; i < ncols - 1; i++){
			printf("Best %f\n", inst->best_sol[i]);
		}
	}
	int count = 0;																		//CUNTER SELECTED NODES
	int n = 0;
	/*-------------------PRINT SELECTED EDGES(remember cplex tolerance)--------------*/
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (inst->best_sol[xpos(i, j, inst)] > 0.5){
				
				if(VERBOSE>=100){
					printf("Il nodo (%d,%d) e' selezionato\n", i+1, j+1);
				}
				/*--ADD EDGES(VECTOR LENGTH = 2*nnodes TO SAVE NODES OF EVERY EDGE)--*/
				inst->choosen_edge[n] = i;
				inst->choosen_edge[n+1] = j;
				n += 2;

				count++;
			}
		}
	}
	add_edge_to_file(inst);
	if (VERBOSE >= 10) {
		printf("Selected nodes: %d \n", count);
	}
	/*-------------------------------------------------------------------------------*/

	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	if (CPXgetobjval(env, lp, &inst->best_obj_val)) print_error("no best objective function");	//OPTIMAL SOLUTION FOUND
	printf("Object function optimal value is: %f\n", inst->best_obj_val);
	/*----------------------CLEAN AND CLOSE THE CPLEX ENVIROENMENT-------------------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}
/*-------------------------------------------------------------------------------------------------------------*/



/*------------------------------BUILD CPLEX MODEL------------------------------------*/
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
		for (int j = i+1; j < inst->nnodes; j++)
		{
			double obj = dist(i, j, inst);
			sprintf(cname[0], "x(%d,%d)", i + 1, j + 1);								//PRINT VARIABLES IN CPLEX IN .lp FILE 
			
			/*--------------------PRINT DISTANCE d(i,j)-------------------*/
			if (VERBOSE >= 500) {
				printf("Distance d(%d,%d): %f \n",i+1,j+1, dist(i, j,inst));
			}

			 
			/*----------------------INSERT VARIABLE IN CPLEX--------------*/
			if (CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) print_error(" wrong CPXnewcols on x var.s");
			
			/*--------------------CHECK VARIABLE POSITION-----------------*/ 
			if (CPXgetnumcols(env, lp) - 1 != xpos(i, j, inst)) print_error(" wrong position for x var.s");
			if (VERBOSE >= 500)
			{
				printf("The column with i=%d e j=%d is in position %d and xpos is %d\n", i, j, CPXgetnumcols(env, lp), xpos(i, j, inst));
			}
		}
	}


	/*--------------------------------ADD CONSTRAINTS----------------------------*/
	for (int h = 0; h < inst->nnodes; h++)
	{
		int lastrow = CPXgetnumrows(env, lp);										//LAST ROW NUMBER(to insert the new constraint)
		if (VERBOSE >= 200) {
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
	/*-------------------write the cplex model in file model.lp------------------*/
	CPXwriteprob(env, lp, "model.lp", NULL);
}
