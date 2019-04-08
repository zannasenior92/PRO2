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
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void add_SEC(CPXENVptr env, CPXLPptr lp, instance *inst);
void update_choosen_edge(instance* inst);
void loop_method_with_timelimit(CPXENVptr env, CPXLPptr lp, instance *inst, FILE* log, double time1, double time2);

/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	
	inst->compact = 0;
	int error;
	CPXENVptr env = CPXopenCPLEX(&error);									//create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP");						//create the structure for our model(lp)
	double time1, time2;
	if (CPXgettime(env, &time1)) print_error("time error");									//START TIME
	build_model(inst, env, lp);
	FILE* log = CPXfopen("log.txt", "w");
	
	/*---------------------------------LOOP METHOD WITH TIMELIMIT-------------------*/
	loop_method_with_timelimit(env,lp,inst,log,time1,time2);

	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIRONMENT-----------*/
	if (CPXgettime(env, &time2)) print_error("time error");
	printf("Total time %f\n", time2 - time1);												//END TIME
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;

}
