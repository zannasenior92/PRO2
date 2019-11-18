/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void build_modelFlow2(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);
void print_error(const char *err);
int xpos(int i, int j, instance *inst);
int xpos_compact(int i, int j, instance *inst);
int ypos(int i, int j, instance *inst);
int zpos_flow2(int i, int j, instance *inst);
double dist(int i, int j, instance *inst);


/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst, int i, int j, int seed)
{

	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	CPXsetintparam(env, CPX_PARAM_RANDOMSEED, seed);
	CPXsetdblparam(env, CPX_PARAM_TILIM, 3600);
	CPXsetintparam(env, CPX_PARAM_CLOCKTYPE, 2);
	double start_time, end_time, elapsed_time;
	if (CPXgettime(env, &start_time)) print_error("error getting time\n");

	build_modelFlow2(inst, env, lp);												//populate the model
	//CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);							//to visualize in video
	if (CPXmipopt(env, lp)) print_error("Error resolving the model\n");		//CPXmipopt to solve the model

	if (CPXgettime(env, &end_time)) print_error("error getting time\n");
	elapsed_time = end_time - start_time;

	int status = CPXgetstat(env, lp);
	int ncols = CPXgetnumcols(env, lp);
	//printf("numero colonne %d\n", ncols);
	inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
	if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) {
		//print_error("no solution avaialable");
		if (j == 0) {
			inst->input_file_name[strlen(inst->input_file_name) - 1] = '\0';
		}
		char out_file[100] = "";
		strcat(out_file, "file");
		char iters[5] = "";
		char iterj[5] = "";
		sprintf(iters, "%d", i);
		sprintf(iterj, "%d", j);
		strcat(out_file, iters);
		strcat(out_file, "_");
		strcat(out_file, iterj);
		strcat(out_file, ".txt");
		FILE* output = fopen(out_file, "w");
		fprintf(output, "Flow2,%s,%f,0,%d", inst->input_file_name, elapsed_time, seed);
		fclose(output);
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env);
		return 0;
	}

	/*-----------------------FIND AND PRINT THE OPTIMAL SOLUTION---------------------*/
	if (CPXgetobjval(env, lp, &inst->best_obj_val)) print_error("no best objective function");	//OPTIMAL SOLUTION FOUND
	printf("Object function optimal value is: %f\n", inst->best_obj_val);

	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT-----------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	if (j == 0) {
		inst->input_file_name[strlen(inst->input_file_name) - 1] = '\0';
	}
	char out_file[100] = "";
	strcat(out_file, "file");
	char iter[5] = "";
	char itj[5] = "";
	sprintf(iter, "%d", i);
	sprintf(itj, "%d", j);
	strcat(out_file, iter);
	strcat(out_file, "_");
	strcat(out_file, itj);
	strcat(out_file, ".txt");
	FILE* output = fopen(out_file, "w");

	if ((status == 101) || (status == 102)) {
		fprintf(output, "Flow2,%s,%f,1,%d", inst->input_file_name, elapsed_time, seed);
	}
	else {
		fprintf(output, "Flow2,%s,%f,0,%d", inst->input_file_name, elapsed_time, seed);

	}
	fclose(output);
	return 0;
}