/* File che contiene le funzioni che usano cplex */
#include "TSP.h"
#include <ilcplex/cplex.h>


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void build_modelPers(instance *inst, CPXENVptr env, CPXLPptr lp);
void print_error(const char *err);
void add_edge_to_file(instance *inst);
int xpos(int i, int j, instance *inst);






/*------------------------------SOLVE THE MODEL--------------------------------------*/
int TSPopt(instance *inst)
{
	
	int error;
	CPXENVptr env = CPXopenCPLEX(&error); //create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); //create the structure for our model(lp)
	build_modelPers(inst, env, lp); //populate the model
	CPXsetintparam(env, CPX_PARAM_SCRIND, CPX_ON);							//to visualize in video
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
				/*--ADD EDGES(VECTOR LENGTH = 2*nnodes TO SAVE NODES OF EVERY EDGE)--*/
				inst->choosen_edge[n] = i;	
				inst->choosen_edge[n+1] = j;
				n += 2;
				count++;
			}
		}
	}
	add_edge_to_file(inst);
	if (VERBOSE >= 100) {
		printf("Selected nodes: %d \n", count);
	}
	
	
	/*-------------------------------------------------------------------------------*/

	/*------------------------------CLEAN AND CLOSE THE CPLEX ENVIROENMENT-----------*/
	CPXfreeprob(env, &lp);
	CPXcloseCPLEX(&env);
	return 0;
}

