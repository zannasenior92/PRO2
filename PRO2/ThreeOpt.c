
/*-------------------------------------------3-OPT FUNCTION-------------------------------------------------*/

#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void reverse_xpos(int x, instance* inst, int* nodes);
void update_choosen_edge(instance* inst);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);

double three_opt(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	//EDGES OF THE OPTIMAL SOLUTION STORED INTO AN ARRAY
	int* edges = (int*)calloc(inst->nnodes, sizeof(int));
	int n = 0;
	for (int i = 0; i < inst->ncols; i++) {
		if (inst->best_sol[i] > TOLERANCE) {
			edges[n] = i;
			if (THREE_OPT > 400)
			{
				printf("edge[%d]=%d\n", n, i);
			}
			n++;
		}
	}

	double min_delta = 0;
	double edge1_length, edge2_length, edge3_length;
	double new_dist1, new_dist2, new_dist3;
	int min_new_edge1, min_new_edge2, min_new_edge3;
	int old_edge1, old_edge2, old_edge3;
	int *nodes_edge1 = (int*)calloc(2, sizeof(int));
	int *nodes_edge2 = (int*)calloc(2, sizeof(int));
	int *nodes_edge3 = (int*)calloc(2, sizeof(int));
	int done = 0;

	for (int i = 0; i < inst->nnodes/3; i++)
	{
		reverse_xpos(edges[i], inst, nodes_edge1);						//TAKE FIRST EDGE
		
		for (int j = inst->nnodes/3 ; j < (inst->nnodes*2) / 3; j++)
		{
			reverse_xpos(edges[j],inst, nodes_edge2);					//TAKE SECOND EDGE
		
			for (int k = (inst->nnodes * 2) / 3; k < inst->nnodes; k++)
			{
				reverse_xpos(edges[k], inst, nodes_edge3);				//TAKE THIRD EDGE

				/*---------------------------CHECK INCOMPATIBILITIES(NO ADJACENT EDGES)------------------------*/
				if ((nodes_edge1[0] == nodes_edge2[0]) || (nodes_edge1[0] == nodes_edge2[1]) || 
					(nodes_edge1[0] == nodes_edge3[0]) || (nodes_edge1[0] == nodes_edge3[1]) ||
					(nodes_edge1[1] == nodes_edge2[0]) || (nodes_edge1[1] == nodes_edge2[1]) ||
					(nodes_edge1[1] == nodes_edge3[0]) || (nodes_edge1[1] == nodes_edge3[1]) ||
					(nodes_edge2[0] == nodes_edge3[0]) || (nodes_edge2[0] == nodes_edge3[1]) ||
					(nodes_edge2[1] == nodes_edge3[0]) || (nodes_edge2[1] == nodes_edge3[1]))
					continue;

				//---------TAKE THE LENGTH OF OLD EDGES
				edge1_length = dist(nodes_edge1[0], nodes_edge1[1], inst);
				edge2_length = dist(nodes_edge2[0], nodes_edge2[1], inst);
				edge3_length = dist(nodes_edge3[0], nodes_edge3[1], inst);
				//---------TAKE LENGTH OF NEW PRESUMED EGDES
				new_dist1 = dist(nodes_edge1[0], nodes_edge2[1], inst);
				new_dist2 = dist(nodes_edge2[0], nodes_edge3[1], inst);
				new_dist3 = dist(nodes_edge3[0], nodes_edge1[1], inst);
				//---------SEE THE DIFFERENCE
				double delta = new_dist1 + new_dist2 + new_dist3 - edge1_length - edge2_length - edge3_length;

				//---------UPDATE IF DELTA IS MINOR
				if (delta < min_delta)
				{
					/*----------------CHOOSE THE NEW TWO EDGES---------------*/
					min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[1], inst);
					min_new_edge2 = xpos(nodes_edge2[0], nodes_edge3[1], inst);
					min_new_edge3 = xpos(nodes_edge3[0], nodes_edge1[1], inst);

					if ((inst->best_sol[min_new_edge1] == 1) || (inst->best_sol[min_new_edge2] == 1) 
						|| (inst->best_sol[min_new_edge3] == 1)) {
						continue;
					}
					old_edge1 = edges[i];
					old_edge2 = edges[j];
					old_edge3 = edges[k];
					if (THREE_OPT > 400)
					{
						printf("best_sol[%d]=%f\n", old_edge1, inst->best_sol[old_edge1]);
						printf("best_sol[%d]=%f\n", old_edge2, inst->best_sol[old_edge2]);
						printf("best_sol[%d]=%f\n", old_edge3, inst->best_sol[old_edge3]);

						printf("best_sol[%d]=%f\n", min_new_edge1, inst->best_sol[min_new_edge1]);
						printf("best_sol[%d]=%f\n", min_new_edge2, inst->best_sol[min_new_edge2]);
						printf("best_sol[%d]=%f\n", min_new_edge3, inst->best_sol[min_new_edge3]);

					}
					/*-----------------UPDATE SELECTION----------------------*/
					inst->best_sol[old_edge1] = 0.0;
					inst->best_sol[old_edge2] = 0.0;
					inst->best_sol[old_edge3] = 0.0;

					inst->best_sol[min_new_edge1] = 1.0;
					inst->best_sol[min_new_edge2] = 1.0;
					inst->best_sol[min_new_edge3] = 1.0;

					/*-----------------CHECK IF CONNECTED COMPONENTS ARE PRESENT-------------*/
					if (kruskal_sst(env, lp, inst) == 1) {
						if (THREE_OPT > 400)
						{
							printf("RESET %d;%d , %d;%d e %d;%d, AND ADD %d;%d , %d;%d e %d;%d \n",
								nodes_edge1[0] + 1, nodes_edge1[1] + 1, nodes_edge2[0] + 1, nodes_edge2[1] + 1,
								nodes_edge3[0] + 1, nodes_edge3[1], 
								nodes_edge1[0] + 1, nodes_edge2[1] + 1, nodes_edge2[0] + 1, nodes_edge3[1] + 1,
								nodes_edge3[0] + 1, nodes_edge1[1] + 1);
							printf("STEP BY %d,%d AND %d TO %d,%d e %d\n",
								old_edge1, old_edge2, old_edge3, min_new_edge1, min_new_edge2, min_new_edge3);
						}

						free(nodes_edge1);
						free(nodes_edge2);
						free(nodes_edge3);
						free(edges);
						return delta;

					}
					else {
						if (THREE_OPT > 400)
						{
							printf("CHANGE NOTHING \n");
						}
						inst->best_sol[old_edge1] = 1.0;
						inst->best_sol[old_edge2] = 1.0;
						inst->best_sol[old_edge3] = 1.0;

						inst->best_sol[min_new_edge1] = 0.0;
						inst->best_sol[min_new_edge2] = 0.0;
						inst->best_sol[min_new_edge3] = 0.0;
						continue;
					}
				}

			}

		}
	}
	free(nodes_edge1);
	free(nodes_edge2);
	free(nodes_edge3);
	free(edges);
	return 0;

}

