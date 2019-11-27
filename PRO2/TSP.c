/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>
#define CPX_PARAM_EPAGAP = 0.05;


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void select_and_build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFlow1(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFischetti(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst);
void update_choosen_edges(instance* inst);
void loop_method_with_timelimit(CPXENVptr env, CPXLPptr lp, instance *inst);
void print_error(const char *err);

/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst, int i)
{
	
	inst->compact = 0;
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	//CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);
	CPXsetintparam(env, CPX_PARAM_RANDOMSEED, 123456);
	CPXsetdblparam(env, CPX_PARAM_TILIM, 3600);
	CPXsetintparam(env, CPX_PARAM_CLOCKTYPE, 2);
	double start_time, end_time, elapsed_time;
	if (CPXgettime(env, &start_time)) print_error("error getting time\n");
	
	build_model(inst, env, lp);
	loop_method_with_timelimit(env, lp, inst);

	if (CPXgettime(env, &end_time)) print_error("error getting time\n");
	elapsed_time = end_time - start_time;

	int status = CPXgetstat(env, lp);
	int ncols = CPXgetnumcols(env, lp);
	inst->best_sol = (double *)calloc(ncols, sizeof(double));				//best objective solution
	if (CPXgetx(env, lp, inst->best_sol, 0, ncols - 1)) {
		inst->input_file_name[strlen(inst->input_file_name) - 1] = '\0';
		char out_file[100] = "";
		strcat(out_file, "file");
		char iters[5] = "";
		sprintf(iters, "%d", i);
		strcat(out_file, iters);
		strcat(out_file, ".txt");
		FILE* output = fopen(out_file, "w");
		fprintf(output, "Sec_Loop_TIMELIMIT,%s,%f,0,123456", inst->input_file_name, elapsed_time);
		fclose(output);
		CPXfreeprob(env, &lp);
		CPXcloseCPLEX(&env);
		return 0;
	}


	if (CPXgetobjval(env, lp, &inst->best_obj_val)) print_error("no best objective function");	//OPTIMAL SOLUTION FOUND
	printf("Object function optimal value is: %f\n", inst->best_obj_val);

	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT-----------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	inst->input_file_name[strlen(inst->input_file_name) - 1] = '\0';
	char out_file[100] = "";
	strcat(out_file, "file");
	char iter[5] = "";
	sprintf(iter, "%d", i);
	strcat(out_file, iter);
	strcat(out_file, ".txt");
	FILE* output = fopen(out_file, "w");
	if ((status == 101) || (status == 102)) {
		fprintf(output, "Sec_Loop_TIMELIMIT,%s,%f,1,123456", inst->input_file_name, elapsed_time);
	}
	else {
		fprintf(output, "Sec_Loop_TIMELIMIT,%s,%f,0,123456", inst->input_file_name, elapsed_time);

	}
	fclose(output);
	return 0;


}
