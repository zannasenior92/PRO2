/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>
#define CPX_PARAM_EPAGAP = 0.05;


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFlow1(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFischetti(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst);

/*METODO CHE CONTROLLA IL TIPO DI MODELLO E COSTRUISCE QUELLO RELATIVO*/
void select_and_build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
	if (inst->model_type == 0){
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

/*------------------POSITION OF VARIABLE INSIDE THE MODEL----------------------------*/
int xpos(int i, int j, instance *inst) {
	if (i > j) return xpos(j, i, inst);
	return i * inst->nnodes + j - ((i + 1)*(i + 2) / 2);
}
int xpos_compact(int i, int j, instance *inst) {
	return i * inst->nnodes + j;
}
int ypos(int i, int j, instance *inst) {
	return (inst->last_x_index + 1) + i * inst->nnodes + j;
}
int zpos(int i, int j, instance *inst) {
	return ((xpos(inst->nnodes, inst->nnodes, inst) + (i * inst->nnodes + j)) + 1);
}
int upos(int i, instance *inst) {
	return inst->nnodes*inst->nnodes + i;
}

/*-------------------------DISTANCE BETWEEN TWO POINTS-------------------------------*/
double dist(int i, int j, instance *inst){
	if(inst->dist_type==0) {
		double dx = inst->xcoord[i] - inst->xcoord[j];
		double dy = inst->ycoord[i] - inst->ycoord[j];
		return (int)(sqrt((dx*dx + dy * dy)) + 0.5);
	}
	if(inst->dist_type==1){
		double dx = inst->xcoord[i] - inst->xcoord[j];
		double dy = inst->ycoord[i] - inst->ycoord[j];
		double rij = sqrt((dx*dx+dy*dy)/10.0);
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
		else print_error("Something go wrong in distance");
}

/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	
	inst->compact = 0;
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	double time1, time2;
	if (CPXgettime(env, &time1)) print_error("time error");
	build_model(inst, env, lp);
	FILE* log = CPXfopen("log.txt", "w");
	
	/*---------------------------METODO LOOP----------------------*/
	int done = 0;
	int resolved = 0;
	double ticks1, ticks2, time3, time4;
	//SETTO timelimit a 10 secondi
	double timelimit;

	while (!done) {
		//prendo il tempo
		if (CPXgettime(env, &time3)) print_error("Error getting time\n");
		//if (CPXgetdettime(env, &ticks1)) print_error("Error getting time\n");
		timelimit = 2.0;
		if (CPXsetdblparam(env, CPX_PARAM_TILIM, timelimit)) print_error("Error on setting parameter");

		resolved = 0;
		//Finchè lo stato di uscita non è ottimo
		while (!resolved) {
			
			if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");
			//Guardo con che stato è terminata la risoluzione
			int status = CPXgetstat(env, lp);
			printf("Status=%d\n", status);
			//Se è terminata per timelimit e non ha soluzioni raddoppio il tempo (????) e continuo
			if (status == CPXMIP_TIME_LIM_INFEAS) {	
				timelimit *= 2;
				if (CPXsetdblparam(env, CPX_PARAM_TILIM, timelimit)) print_error("Error on setting parameter");
				printf("No soluzione, raddoppio il tempo\n");
				continue;
			
			}
			if (status == CPXMIP_TIME_LIM_FEAS) {
				int ncols = CPXgetnumcols(env, lp);
				inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
				if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
				printf("Ha %d comp conn\n", kruskal_sst(env, lp, inst));
				if (kruskal_sst(env, lp, inst) == inst->nnodes) {
					printf("Ha n componenti connesse\n");
					if (CPXsetdblparam(env, CPX_PARAM_TILIM, timelimit * 2)) print_error("Error on setting parameter");
					continue;
				}
				else {
					add_SEC(env, lp, inst);
					if (VERBOSE >= 10) {
						printf("Aggiunti vincoli\n");
					}
					
				}

			}
			if ((status == CPXMIP_OPTIMAL) || (status == CPXMIP_OPTIMAL_TOL)) {
				int ncols = CPXgetnumcols(env, lp);
				inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
				if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");
				resolved = 1;
				if (kruskal_sst(env, lp, inst) == 1) {
					done = 1;
					resolved = 1;
				}

				else {
					add_SEC(env, lp, inst);
					if (VERBOSE >= 10) {
						printf("Aggiunti vincoli\n");
					}
				}
			}
		}
		
		//if (CPXgetdettime(env, &ticks2)) print_error("Error getting time\n");
		if(CPXgettime(env,&time4)) print_error("Error getting time\n");
		//printf("Ticks=%f\n", ticks2-ticks1);
		printf("Tempo=%f\n", time4-time3);

		if (CPXsetlogfile(env, log)) print_error("Error in log file");
		
		
	}
	
	//Ultimissima iterazione senza timelimit TODO
	if (CPXsetdblparam(env, CPX_PARAM_TILIM, 1e+75)) print_error("Error on setting parameter");
	done = 0;
	while (!done) {
		if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");
	}
	int ncols = CPXgetnumcols(env, lp);
	inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
	if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) print_error("no solution avaialable");


	if(VERBOSE>=200){
		for (int i = 0; i < ncols - 1; i++){
			printf("Best %f\n", inst->best_sol[i]);
		}
	}
	int count = 0;
	int n = 0;
	/*-------------------PRINT SELECTED EDGES(remember cplex tolerance)--------------*/
	if (inst->compact == 1) {
		for (int i = 0; i < inst->nnodes; i++) {
			for (int j = 0; j < inst->nnodes; j++) {
					if (inst->best_sol[xpos_compact(i, j, inst)] > 0.5) {

						if (VERBOSE >= 100) {
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
	}
	else {
		for (int i = 0; i < inst->nnodes; i++) {
			for (int j = i+1; j < inst->nnodes; j++) {
				if (inst->best_sol[xpos(i, j, inst)] > 0.5) {

					if (VERBOSE >= 100) {
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
	}
	add_edge_to_file(inst);

	if (VERBOSE >= 100) {
		printf("Selected nodes: %d \n", count);
	}
	/*-------------------------------------------------------------------------------*/
	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	double *opt_val = 0;																//VALUE OPTIMAL SOL
	if (CPXgetobjval(env, lp, &opt_val)) print_error("Error getting optimal value");;													//OPTIMAL SOLUTION FOUND
	printf("Object function optimal value is: %.0f\n", opt_val);
	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT-----------*/

	if (CPXgettime(env, &time2)) print_error("time error");
	printf("Total time %f\n", time2 - time1);
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}

int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst) {
	int c1, c2 = 0;
	int n_connected_comp = 0;
	int max = -1;
	inst->mycomp = (int*)calloc(inst->nnodes, sizeof(int));

	/*INIZIALIZZAZIONE*/
	for (int i = 0; i < inst->nnodes; i++) {
		inst->comp[i] = i;
	}
	/*UNIONE COMPONENTI CONNESSE*/
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (inst->best_sol[xpos(i, j, inst)] > 0.5) {
				if (inst->comp[i] != inst->comp[j]) {
					c1 = inst->comp[i];
					c2 = inst->comp[j];
				}
				for (int v = 0; v < inst->nnodes; v++) {
					if (inst->comp[v] == c2)
						inst->comp[v] = c1;
				}
				
			}

		}
	}
	
	for (int i = 0; i < inst->nnodes; i++) {
		if (VERBOSE >= 100) {
			printf("Componente %d\n", inst->comp[i]);
		}
		inst->mycomp[inst->comp[i]] = 1;

	}

	int n = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		if (inst->mycomp[i]!=0) {
			n++;
		}

	}
	if(VERBOSE>=100){
		printf("Componenti connesse %d\n", n);
	}
	inst->n_connected_comp = n;
	return n;
}

void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst) {
	int nnz = 0;
	double rhs = -1.0;
	char sense = 'L';
	int ncols = CPXgetnumcols(env, lp);
	int *index = (int *)malloc( ncols* sizeof(int));
	double *value = (double *)malloc(ncols * sizeof(double));
	int matbeg = 0;
	char **cname = (char **)calloc(1, sizeof(char *));							// (char **) required by cplex...
	cname[0] = (char *)calloc(100, sizeof(char));

	for(int h=0; h < inst->nnodes; h++){
		if(inst->mycomp[h]!=0){
			for (int i = 0; i < inst->nnodes; i++) {
				if (inst->comp[i] != h) continue;
				rhs++;
				sprintf(cname[0], "SEC(%d)", i);

				for (int j = i + 1; j < inst->nnodes; j++) {
					if (inst->comp[j] == h) {
						index[nnz] = xpos(i, j, inst);
						value[nnz] = 1;
						nnz++;
					}
				}
			}
			if (CPXaddrows(env, lp, 0, 1, nnz, &rhs, &sense, &matbeg, index, value, NULL, cname)) print_error("wrong CPXaddrow");
			if(VERBOSE>=200){
				CPXwriteprob(env, lp, "model.lp", NULL);
			}
			}

	}
}

