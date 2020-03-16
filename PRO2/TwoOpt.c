#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void reverse_xpos(int x, instance* inst, int* nodes);
void update_choosen_edge(instance* inst);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
double cost_alg(instance* inst);
void reset_old_edges2(instance *inst, int old1, int old2, int new1, int new2);
void set_new_edges2(instance *inst, int old1, int old2, int new1, int new2);


/*-----------------------------2-OPT ALGORITHM---------------------------------------*/
double two_opt(instance *inst, CPXENVptr env, CPXLPptr lp){
	double initial_cost = cost_alg(inst);

	/*--------------WRITE EDGES OF OPTIMAL SOLUTION IN AN ARRAY--------------*/
	int* edges = (int*)calloc(inst->nnodes, sizeof(int));
	int n = 0;
	for (int i = 0; i < inst->ncols; i++) {
		if (inst->best_sol[i] > TOLERANCE) {
			edges[n] = i;
			if (TWO_OPT > 400)
			{
				printf("edge[%d]=%d\n", n, i);
			}
			n++;
		}
	}

	double min_delta = 0;
	double edge1_length, edge2_length;
	double new_dist1, new_dist2;
	int min_new_edge1, min_new_edge2;
	int old_edge1, old_edge2;

	/*------INIZIALIZE TWO VECTOR WHERE PUT NODES RELATED TO EDGES------*/
	int* nodes_edge1 = (int*)calloc(2, sizeof(int));
	int* nodes_edge2 = (int*)calloc(2, sizeof(int));
	int done = 0;

	for (int i = 0; i < inst->nnodes; i++) {
		reverse_xpos(edges[i], inst, nodes_edge1);
		for (int j = i + 1; j < inst->nnodes; j++) {
			
			reverse_xpos(edges[j], inst, nodes_edge2);				//TAKE NODES FROM EDGE
				
			/*---------------------------CHECK INCOMPATIBILITIES(NO ADJACENT EDGES)------------------------*/
			if ((nodes_edge1[0] == nodes_edge2[0]) || (nodes_edge1[0] == nodes_edge2[1]) ||
				(nodes_edge1[1] == nodes_edge2[0]) || (nodes_edge1[1] == nodes_edge2[1]))
				continue;
			//---------TAKE THE LENGTH OF OLD EDGES
			edge1_length = dist(nodes_edge1[0], nodes_edge1[1], inst);
			edge2_length = dist(nodes_edge2[0], nodes_edge2[1], inst);
			//---------TAKE LENGTH OF NEW PRESUMED EGDES
			new_dist1 = dist(nodes_edge1[0], nodes_edge2[1], inst);
			new_dist2 = dist(nodes_edge1[1], nodes_edge2[0],  inst);
			//---------SEE THE DIFFERENCE
			double delta = new_dist1 + new_dist2 - edge1_length - edge2_length;
			//---------UPDATE IF DELTA IS MINOR
			if (delta < min_delta) {
				/*----------------CHOOSE THE NEW TWO EDGES---------------*/
				min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[1], inst);
				min_new_edge2 = xpos(nodes_edge1[1], nodes_edge2[0], inst);
				if ((inst->best_sol[min_new_edge1] == 1) || (inst->best_sol[min_new_edge2]==1)) {
					continue;
				}
				old_edge1 = edges[i];
				old_edge2 = edges[j];
				if (TWO_OPT > 400)
				{
					printf("best_sol[%d]=%f\n", old_edge1, inst->best_sol[old_edge1]);
					printf("best_sol[%d]=%f\n", old_edge2, inst->best_sol[old_edge2]);
					printf("best_sol[%d]=%f\n", min_new_edge1, inst->best_sol[min_new_edge1]);
					printf("best_sol[%d]=%f\n", min_new_edge2, inst->best_sol[min_new_edge2]);
				}
				/*-----------------UPDATE SELECTION----------------------*/
				set_new_edges2(inst, old_edge1, old_edge2, min_new_edge1, min_new_edge2);
				
				if (TWO_OPT > 400)
				{
					printf("AFTER SELECTION \n");
					printf("best_sol[%d]=%f\n", old_edge1, inst->best_sol[old_edge1]);
					printf("best_sol[%d]=%f\n", old_edge2, inst->best_sol[old_edge2]);
					printf("best_sol[%d]=%f\n", min_new_edge1, inst->best_sol[min_new_edge1]);
					printf("best_sol[%d]=%f\n", min_new_edge2, inst->best_sol[min_new_edge2]);

				}
				/*-----------------CHECK IF CONNECTED COMPONENTS ARE PRESENT-------------*/
				if (kruskal_sst(env, lp, inst) == 1) {
					if (TWO_OPT > 400)
					{
						printf("RESET %d, %d e %d, %d, AND ADD %d, %d e %d %d \n",
							nodes_edge1[0] + 1, nodes_edge1[1] + 1, nodes_edge2[0] + 1, nodes_edge2[1] + 1,
							nodes_edge1[0] + 1, nodes_edge2[1] + 1, nodes_edge1[1] + 1, nodes_edge2[0] + 1);
						printf("STEP BY %d, %d TO %d, %d\n",
							old_edge1, old_edge2, min_new_edge1, min_new_edge2);
					}
					
					free(nodes_edge1);
					free(nodes_edge2);
					free(edges);
					double final_cost = cost_alg(inst);
					return delta;
					
				}
				else {
					/*-----------------------RESET EDGES IN BEST SOL-----------*/
					reset_old_edges2(inst, old_edge1, old_edge2, min_new_edge1, min_new_edge2);
					
					//---------TAKE LENGTH OF NEW PRESUMED EGDES
					new_dist1 = dist(nodes_edge1[0], nodes_edge2[0], inst);
					new_dist2 = dist(nodes_edge1[1], nodes_edge2[1], inst);
					//---------SEE THE DIFFERENCE
					double delta = new_dist1 + new_dist2 - edge1_length - edge2_length;
					if (delta < min_delta) {
						/*----------------CHOOSE THE NEW TWO EDGES---------------*/
						min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[0], inst);
						min_new_edge2 = xpos(nodes_edge1[1], nodes_edge2[1], inst);
						if ((inst->best_sol[min_new_edge1] == 1) || (inst->best_sol[min_new_edge2] == 1)) {
							continue;
						}
						old_edge1 = edges[i];
						old_edge2 = edges[j];
						if (TWO_OPT > 400)
						{
							printf("best_sol[%d]=%f\n", old_edge1, inst->best_sol[old_edge1]);
							printf("best_sol[%d]=%f\n", old_edge2, inst->best_sol[old_edge2]);
							printf("best_sol[%d]=%f\n", min_new_edge1, inst->best_sol[min_new_edge1]);
							printf("best_sol[%d]=%f\n", min_new_edge2, inst->best_sol[min_new_edge2]);
						}
						/*-----------------UPDATE SELECTION----------------------*/
						set_new_edges2(inst, old_edge1, old_edge2, min_new_edge1, min_new_edge2);

						if (TWO_OPT > 400)
						{
							printf("AFTER SELECTION \n");
							printf("best_sol[%d]=%f\n", old_edge1, inst->best_sol[old_edge1]);
							printf("best_sol[%d]=%f\n", old_edge2, inst->best_sol[old_edge2]);
							printf("best_sol[%d]=%f\n", min_new_edge1, inst->best_sol[min_new_edge1]);
							printf("best_sol[%d]=%f\n", min_new_edge2, inst->best_sol[min_new_edge2]);

						}
						/*-----------------CHECK IF CONNECTED COMPONENTS ARE PRESENT-------------*/
						if (kruskal_sst(env, lp, inst) == 1) {
							if (TWO_OPT > 400)
							{
								printf("RESET %d, %d e %d, %d, AND ADD %d, %d e %d %d \n",
									nodes_edge1[0] + 1, nodes_edge1[1] + 1, nodes_edge2[0] + 1, nodes_edge2[1] + 1,
									nodes_edge1[0] + 1, nodes_edge2[1] + 1, nodes_edge1[1] + 1, nodes_edge2[0] + 1);
								printf("STEP BY %d, %d TO %d, %d\n",
									old_edge1, old_edge2, min_new_edge1, min_new_edge2);
							}

							free(nodes_edge1);
							free(nodes_edge2);
							free(edges);
							double final_cost = cost_alg(inst);
							return delta;

						}
					}

					if (TWO_OPT > 400)
					{
						printf("CHANGE NOTHING \n");
					}
					
				}
			}
		}
	}
	
	free(nodes_edge1);
	free(nodes_edge2);
	free(edges);
	return 0;
	
}


void reverse_xpos(int x, instance* inst, int* nodes) {
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (xpos(i, j, inst) == x) {
				nodes[0] = i;
				nodes[1] = j;
				return;
			}
		}
	}
}

void reset_old_edges2(instance *inst, int old1, int old2, int new1, int new2)
{
	inst->best_sol[old1] = 1.0;
	inst->best_sol[old2] = 1.0;

	inst->best_sol[new1] = 0.0;
	inst->best_sol[new2] = 0.0;
}

void set_new_edges2(instance *inst, int old1, int old2, int new1, int new2)
{
	inst->best_sol[old1] = 0.0;
	inst->best_sol[old2] = 0.0;

	inst->best_sol[new1] = 1.0;
	inst->best_sol[new2] = 1.0;
}