#include "TSP.h"

/*FUNCTION TO WRITE IN A FILE SELECTED EDGES THAT MUST BE PRINTED*/

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
int xpos(int i, int j, instance *inst);
int xpos_compact(int i, int j, instance *inst);
void add_edge_to_file(instance *inst);

void update_choosen_edges(instance *inst)
{
	int count = 0;
	int n = 0;
	/*-------------------PRINT SELECTED EDGES(remember cplex tolerance)--------------*/
	if (inst->compact == 1) {
		for (int i = 0; i < inst->nnodes; i++) {
			for (int j = 0; j < inst->nnodes; j++) {
				if (inst->best_sol[xpos_compact(i, j, inst)] > TOLERANCE) {

					if (VERBOSE >= 100) {
						printf("Node (%d,%d) selected\n", i + 1, j + 1);
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
			for (int j = i + 1; j < inst->nnodes; j++) {
				if (inst->best_sol[xpos(i, j, inst)] > TOLERANCE) {

					if (VERBOSE >= 100) {
						printf("Node (%d,%d) selected\n", i + 1, j + 1);
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
}