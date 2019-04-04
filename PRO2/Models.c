#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFlow1(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFischetti(instance *inst, CPXENVptr env, CPXLPptr lp);


/*------------------------CHECK THE MODEL AND BUILD THE RELATIVE---------------------*/
void select_and_build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
	if (inst->model_type == 0) {
		printf("Modello TSP normale\n");
		build_model(inst, env, lp);
	}
	if (inst->model_type == 1) {
		printf("Modello Flow1\n");
		inst->compact = 1;
		build_modelFlow1(inst, env, lp);
	}
	if (inst->model_type == 2) {
		printf("Modello MTZ\n");
		inst->compact = 1;
		build_modelMTZ(inst, env, lp);
	}
	if (inst->model_type == 3) {
		printf("Modello Fischetti\n");
		build_modelFischetti(inst, env, lp);
	}

}

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
			if (VERBOSE >= 500) {
				printf("Distance d(%d,%d): %f \n", i + 1, j + 1, dist(i, j, inst));
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

/*********************************** MTZ MODEL *************************************/
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp) {

	double lb = 0.0;																	//lower bound
	char binary = 'B';																	//binary variable (0 OR 1)
	char integer = 'I';

	/*-------------------------TO WRITE WELL THE MODEL----------------------------*/
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
		//double ub = inst->nnodes;

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
			index[1] = upos(j, inst);
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

/************************************* FLOW1 MODEL **************************************/
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
			double ub = (i == j) ? 0.0 : inst->nnodes - 1;

			/*--------------------INSERT VARIABLE IN CPLEX----------------*/
			if (CPXnewcols(env, lp, 1, &obj, &lbu, &ub, &integer, cname)) print_error(" wrong CPXnewcols on y(%d,%d) var.s", i, j);
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
		int *index = (int *)malloc(2 * sizeof(int));
		double *value = (double *)malloc(2 * sizeof(double));
		char sense = 'L';
		double rhs = 0;
		for (int j = 0; j < inst->nnodes; j++) {

			if (i == j) continue;
			index[0] = ypos(i, j, inst);										//VARIABLE'S  INDEX
			value[0] = 1.0;														//VARIABLE'S VALUE  
			index[1] = xpos_compact(i, j, inst);										//VARIABLE'S  INDEX
			value[1] = -(inst->nnodes - 1);
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
	int *index = (int *)malloc((inst->nnodes - 1) * sizeof(int));
	double *value = (double *)malloc((inst->nnodes - 1) * sizeof(double));
	double rhs = inst->nnodes - 1;												//(n-1)
	sprintf(cname[0], "y1j(1,j)");

	for (int j = 1; j < inst->nnodes; j++) {

		index[j - 1] = ypos(0, j, inst);											//INDEX OF THE COLUMN CORRESPOND TO THE VARIABLE
		value[j - 1] = 1.0;
	}

	if (CPXaddlazyconstraints(env, lp, 1, inst->nnodes - 1, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");
	free(index);
	free(value);

	/*--------------------------SUM_j(yij)-SUM_k(yjk) = 1    j!=1-------------------*/
	for (int j = 1; j < inst->nnodes; j++) {
		char sense = 'E';
		int izero = 0;
		int *index = (int *)malloc((2 * inst->nnodes - 2) * sizeof(int));
		double *value = (double *)malloc((2 * inst->nnodes - 2) * sizeof(double));
		double rhs = 1;
		sprintf(cname[0], "yij(i,%d)_yjk(%d,k)", j + 1, j + 1);

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
			index[lazy_index] = ypos(j, k, inst);											//INDEX OF THE COLUMN CORRESPOND TO THE VARIABLE
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
	/*-------------------write the cplex model in file modelFlow1.lp------------------*/
	CPXwriteprob(env, lp, "modelFlow1.lp", NULL);
}

/*********************************** FISCH MODEL ************************************/
void build_modelFischetti(instance *inst, CPXENVptr env, CPXLPptr lp) {

	double lb = 0.0; //lower bound
	double ub = 1.0; //upper bound
	char binary = 'B'; //binary variable (0 OR 1)
	//char continuous = 'C';

	//Definisco cname per scrivere il modello in modo più chiaro
	char **cname = (char **)calloc(1, sizeof(char *));		// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));

	/*-------------------------DEFINE VARIABLES ON THE MODEL----------------------*/
	for (int i = 0; i < inst->nnodes; i++)
	{
		for (int j = i + 1; j < inst->nnodes; j++)
		{
			double obj = dist(i, j, inst);
			sprintf(cname[0], "x(%d,%d)", i + 1, j + 1);//print variables on cplex 

			/*--------------------PRINT DISTANCE d(i,j)-------------------*/
			if (VERBOSE >= 500) {
				printf("Distance d(%d,%d): %f \n", i + 1, j + 1, dist(i, j, inst));
			}
			if (CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) print_error(" wrong CPXnewcols on x var.s");
			if (CPXgetnumcols(env, lp) - 1 != xpos(i, j, inst)) print_error(" wrong position for x var.s");

		}
	}
	/*----------------INSERISCO LE Z: zvh=1 se vertice v si trova in posizione h*/
	for (int i = 0; i < inst->nnodes; i++)
	{
		double lb = 0.0; //lower bound
		double ub = 1.0; //upper bound
		char binary = 'B'; //binary variable (0 OR 1)
		for (int j = 0; j < inst->nnodes; j++)
		{
			double obj = 0;
			sprintf(cname[0], "z(%d,%d)", i + 1, j + 1);//print variables on cplex 

			if (CPXnewcols(env, lp, 1, &obj, &lb, &ub, &binary, cname)) print_error(" wrong CPXnewcols on v variables");

		}
	}


	/*--------------------------------ADD CONSTRAINTS----------------------------*/
	for (int h = 0; h < inst->nnodes; h++)  // degree ciclo esterno per ogni vincolo che voglio aggiungere per nodo h
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
				/*-------------SET TO 1 THE VARIABLES COEFFICIENT IN THE EQUATION----------------------------*/
				if (CPXchgcoef(env, lp, lastrow, xpos(i, h, inst), 1.0)) print_error(" wrong CPXchgcoef [x1]");
		}
	}
	/*---------------VINCOLI PER Z-------------*/
	/* INSERISCO z11=1*/
	int izero = 0;
	int *index = (int *)malloc(1 * sizeof(int));
	double *value = (double *)malloc(1 * sizeof(double));
	double rhs = 1.0;
	char sense = 'E';
	index[0] = zpos(0, 0, inst);
	value[0] = 1.0;
	sprintf(cname[0], "z11");
	if (CPXaddlazyconstraints(env, lp, 1, 1, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");

	/*-------------ogni vertice deve avere una posizione---*/
	for (int v = 0; v < inst->nnodes; v++)
	{
		int lastrow = CPXgetnumrows(env, lp);	//chiedo a cplex ultima riga cambiata chiedendo numero di righe
		double rhs = 1.0;
		char sense = 'E';
		sprintf(cname[0], "somme_z((%d),h)", v + 1);
		if (CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) print_error(" wrong CPXnewrows [x1]");
		for (int h = 0; h < inst->nnodes; h++)
		{
			/*-------------SET TO 1 THE VARIABLES COEFFICIENT IN THE EQUATION----------------------------*/
			if (CPXchgcoef(env, lp, lastrow, zpos(v, h, inst), 1.0)) print_error(" wrong CPXchgcoef [x1]");
		}
	}
	/*-------------ogni posizione deve avere un vertice---*/
	for (int h = 0; h < inst->nnodes; h++)
	{
		int lastrow = CPXgetnumrows(env, lp);
		double rhs = 1.0;
		char sense = 'E';
		sprintf(cname[0], "somme_z(v,(%d))", h + 1);
		if (CPXnewrows(env, lp, 1, &rhs, &sense, NULL, cname)) print_error(" wrong CPXnewrows [x1]");
		for (int v = 0; v < inst->nnodes; v++)
		{
			/*-------------SET TO 1 THE VARIABLES COEFFICIENT IN THE EQUATION----------------------------*/
			if (CPXchgcoef(env, lp, lastrow, zpos(v, h, inst), 1.0)) print_error(" wrong CPXchgcoef [x1]");
		}
	}
	/*---------------Vincolo forte somme(zit)+xij+somme(jt)<=2*/

	for (int i = 1; i < inst->nnodes; i++) {
		for (int j = 1; j < inst->nnodes; j++) {
			if (i == j) continue;
			for (int h = 2; h < inst->nnodes; h++) {
				int n = 0;
				char sense = 'L';
				int izero = 0;
				int *index = (int *)malloc((inst->nnodes) * sizeof(int));
				double *value = (double *)malloc((inst->nnodes) * sizeof(double));
				double rhs = 2;

				sprintf(cname[0], "h%d_x(%d,%d)", h + 1, i + 1, j + 1);
				for (int t = 0; t < h; t++) {
					index[n] = zpos(i, t, inst);
					value[n] = 1.0;
					n++;
				}

				index[n] = xpos(i, j, inst);
				value[n] = 1.0;
				n++;
				for (int t = h + 1; t < inst->nnodes; t++) {
					index[n] = zpos(j, t, inst);
					value[n] = 1.0;
					n++;
				}
				if (CPXaddlazyconstraints(env, lp, 1, n, &rhs, &sense, &izero, index, value, cname)) print_error("wrong CPXlazyconstraints");
				free(index);
				free(value);
			}

		}
	}

	CPXwriteprob(env, lp, "modelFischetti.lp", NULL);
}
