/*--------------------------TSP RESOLVE & CREATE THE MODEL---------------------------*/
#include "TSP.h"
#include <ilcplex/cplex.h>
#include <time.h>

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void plot_gnuplot(instance *inst);
void select_and_build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_model(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFlow1(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelMTZ(instance *inst, CPXENVptr env, CPXLPptr lp);
void build_modelFischetti(instance *inst, CPXENVptr env, CPXLPptr lp);
void add_edge_to_file(instance *inst);
int myseparation(instance *inst, double *xstar, CPXCENVptr env, void *cbdata, int wherefrom);
int xpos(int i, int j, instance *inst);
int xpos_compact(int i, int j, instance *inst);
void print_error(const char *err);
void selected_edges(instance *inst);
void update_choosen_edge(instance* inst);
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);
double two_opt(instance *inst, CPXENVptr env, CPXLPptr lp);
double vns(instance *inst, CPXENVptr env, CPXLPptr lp);

/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst, int i)
{
	inst->compact = 0;
	int error;
	double opt_heu, opt_current;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	build_model(inst, env, lp);



	inst->ncols = CPXgetnumcols(env, lp);

	inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
	double cost;
	opt_heu = INFINITY;
	double *minimum_solution = (double*)calloc(inst->ncols, sizeof(double));
	int start_node = 0;
	for (int i = 0; i < 10; i++) {
		for (int j = 0; j < inst->nnodes; j++) {
			inst->best_sol = (double*)calloc(inst->ncols, sizeof(double));
			cost = nearest_neighborhood_GRASP(inst, env, lp, j);
			if (cost < opt_heu) {
				opt_heu = cost;
				minimum_solution = inst->best_sol;
			}
		}
	}

	printf("\nBest Initial Cost After Nearest Neighborhood GRASP %f\n", opt_heu);
	inst->best_sol = minimum_solution;

	opt_current = opt_heu;

	double min_local = CPX_INFBOUND;
	time_t timelimit = time(NULL) + 3600;
	//printf("--------------------2-OPT-------------------\n");
	double delta_vns = -1;
	//ESEGUO 2-OPT per trovare minimo locale
	while (time(NULL) < timelimit) {

		double delta = two_opt(inst, env, lp);
		
		opt_current += delta;

		if (delta == 0.0) {
			//printf("Minimo locale: %f\n", opt_current);
			if (opt_current < min_local) {
				min_local = opt_current;
			}
			while (delta_vns== -1) {
				delta_vns = vns(inst, env, lp);
			}
			opt_current += delta_vns;
			//printf("--------------------VNS-------------------\n");
			//printf("OTTIMO INIZIALE: %f\n", opt_current);
			delta_vns = -1;
			//break;
		}


	}
	if (min_local < opt_current) {
		printf("MINIMUM SOLUTION: %f\n", min_local);
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

		fprintf(output, "VNS,%s,%f,123456", inst->input_file_name, min_local);

		fclose(output);
	}
	else {
		printf("MINIMUM SOLUTION: %f\n", opt_current);
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

		fprintf(output, "VNS,%s,%f,123456", inst->input_file_name, opt_current);

		fclose(output);
	}
	
	return 0;
}