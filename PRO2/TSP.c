/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);

/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst, int i)
{
	
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 123456);
	CPXsetdblparam(env, CPX_PARAM_TILIM, 3600);
	CPXsetintparam(env, CPX_PARAM_CLOCKTYPE, 2);
	double start_time, end_time, elapsed_time;
	if (CPXgettime(env, &start_time)) print_error("error getting time\n");

	build_modelMTZ(inst, env, lp);												//populate the model
	//CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);							//to visualize in video
	if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model

	if (CPXgettime(env, &end_time)) print_error("error getting time\n");
	elapsed_time = end_time - start_time;

	int status = CPXgetstat(env, lp);
	int ncols = CPXgetnumcols(env, lp);
	//printf("numero colonne %d\n", ncols);
	inst->best_sol= (double *)calloc(ncols, sizeof(double));				//best objective solution
	if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) {
		//print_error("no solution avaialable");
		inst->input_file_name[strlen(inst->input_file_name) - 1] = '\0';
		char out_file[100] = "";
		strcat(out_file, "file");
		char iter[2];
		sprintf(iter, "%d", i);
		strcat(out_file, iter);
		strcat(out_file, ".txt");
		FILE* output = fopen(out_file, "w");
		fprintf(output, "MTZ,%s,%f,0,123456", inst->input_file_name, elapsed_time);	
		fclose(output);
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env);
		return 0;
	}
	
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
	//add_edge_to_file(inst);

	if (VERBOSE >= 100) {
		printf("Selected nodes: %d \n", count);
	}
	/*-------------------------------------------------------------------------------*/
	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	if (CPXgetobjval(env, lp, &inst->best_obj_val)) print_error("no best objective function");	//OPTIMAL SOLUTION FOUND
	printf("Object function optimal value is: %f\n", inst->best_obj_val);
	
	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT-----------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	inst->input_file_name[strlen(inst->input_file_name) - 1] = '\0';
	char out_file[100]="";
	strcat(out_file, "file");
	char iter[2];
	sprintf(iter, "%d", i);
	strcat(out_file, iter);
	strcat(out_file, ".txt");
	FILE* output = fopen(out_file, "w");
	if ((status == 101) || (status == 102)) {
		fprintf(output, "MTZ,%s,%f,1,123456", inst->input_file_name, elapsed_time);
	}
	else {
		fprintf(output, "MTZ,%s,%f,0,123456", inst->input_file_name, elapsed_time);

	}
	fclose(output);
	return 0;
}