/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void build_modelFischetti(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);


/*------------------POSITION OF VARIABLE INSIDE THE MODEL----------------------------*/
int xpos(int i, int j, instance *inst) {
	if (i > j) return xpos(j, i, inst);
	return i * inst->nnodes + j-((i+1)*(i+2)/2);
}
int zpos(int i, int j, instance *inst) {
	return ((xpos(inst->nnodes,inst->nnodes,inst)+(i * inst->nnodes + j))+1);
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
}



/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	
	int error;
	CPXENVptr env = CPXopenCPLEX(&error); //create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); //create the structure for our model(lp)
	build_modelFischetti(inst, env, lp); //populate the model
	if (CPXmipopt(env, lp)) print_error("Error resolving the model\n"); //CPXmipopt to solve the model

	int ncols = CPXgetnumcols(env, lp);
	if (CPXgetobjval(env, lp, &inst->best_obj_val)) print_error("no best solution avaialable");
	printf("Miglior soluzione=%.0f\n", inst->best_obj_val);
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
			if (inst->best_sol[xpos(i, j, inst)] > 0.5){
				
				if(VERBOSE>=100){
					printf("Il nodo (%d,%d) e' selezionato\n", i+1, j+1);
				}
				//Aggiungo i nodi a due a due, cosi so che ad ogni coppia corrisponde un arco
				inst->choosen_edge[n] = i;	//Uso un vettore lungo 2*nnodes per salvare i nodi corrispondenti agli archi
				inst->choosen_edge[n+1] = j; //scelti. Cosi aggiorno il file per il plot una sola volta e lo sovrascrivo.
				n += 2;
				//add_edge_to_plot(i, j, inst);//add in a file selected edges
				count++;
			}
		}
	}
	add_edge_to_file(inst);
	if (VERBOSE >= 10) {
		printf("Selected nodes: %d \n", count);
	}
	
	
	/*-------------------------------------------------------------------------------*/

	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIROENMENT-----------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}

/*------------------------------BUILD CPLEX MODEL------------------------------------*/
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
		for (int j = i+1; j < inst->nnodes; j++)
		{
			double obj = dist(i, j, inst);
			sprintf(cname[0], "x(%d,%d)", i + 1, j + 1);//print variables on cplex 
			
			/*--------------------PRINT DISTANCE d(i,j)-------------------*/
			if (VERBOSE >= 500) {
				printf("Distance d(%d,%d): %f \n",i+1,j+1, dist(i, j,inst));
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
			double obj =0;
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
	index[0] = zpos(0,0, inst);
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
			if (CPXchgcoef(env, lp, lastrow, zpos(v,h, inst), 1.0)) print_error(" wrong CPXchgcoef [x1]");
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
				
				sprintf(cname[0], "h%d_x(%d,%d)",h+1, i + 1, j + 1);
				for (int t = 0; t < h; t++) {
					index[n] = zpos(i, t, inst);
					value[n] = 1.0;
					n++;
				}
				
				index[n] = xpos(i, j, inst);
				value[n] = 1.0;
				n++;
				for (int t = h+1; t < inst->nnodes; t++) {
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
