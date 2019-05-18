
/*-------------------------------------------3-OPT FUNCTION-------------------------------------------------*/

#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
int* reversexpos(instance* inst, int CPX_pos);
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
	int *nodes_edge1;
	int *nodes_edge2;
	int *nodes_edge3;
	int done = 0;

	for (int i = 0; i < inst->nnodes/3; i++)
	{
		nodes_edge1 = reversexpos(inst, i);
		for (int j = inst->nnodes/3 ; j < (inst->nnodes*2) / 3; j++)
		{
			nodes_edge2 = reversexpos(inst, j);
			for (int k = (inst->nnodes * 2) / 3; k < inst->nnodes; k++)
			{
				nodes_edge3 = reversexpos(inst, k);

				if ((nodes_edge1[0] == nodes_edge2[0]) || (nodes_edge1[0] == nodes_edge2[1]) || 
					(nodes_edge1[0] == nodes_edge3[0]) || (nodes_edge1[0] == nodes_edge3[1]) ||
					(nodes_edge1[1] == nodes_edge2[0]) || (nodes_edge1[1] == nodes_edge2[1]) ||
					(nodes_edge1[1] == nodes_edge3[0]) || (nodes_edge1[1] == nodes_edge3[1]) ||
					(nodes_edge2[0] == nodes_edge3[0]) || (nodes_edge2[0] == nodes_edge3[1]) ||
					(nodes_edge2[1] == nodes_edge3[0]) || (nodes_edge2[1] == nodes_edge3[1]))
					continue;

				//prendo la lunghezza degli archi vecchi
				edge1_length = dist(nodes_edge1[0], nodes_edge1[1], inst);
				edge2_length = dist(nodes_edge2[0], nodes_edge2[1], inst);
				edge3_length = dist(nodes_edge3[0], nodes_edge3[1], inst);
				//prendo la lunghezza dei nuovi presunti archi
				new_dist1 = dist(nodes_edge1[0], nodes_edge2[1], inst);
				new_dist2 = dist(nodes_edge2[0], nodes_edge3[1], inst);
				new_dist3 = dist(nodes_edge3[0], nodes_edge1[1], inst);
				//guardo la differenza
				double delta = new_dist1 + new_dist2 + new_dist3 - edge1_length - edge2_length - edge3_length;

				//se delta minore distanza più corta
				if (delta < min_delta) 
				{
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


					inst->best_sol[old_edge1] = 0.0;
					inst->best_sol[old_edge2] = 0.0;
					inst->best_sol[old_edge3] = 0.0;

					inst->best_sol[min_new_edge1] = 1.0;
					inst->best_sol[min_new_edge2] = 1.0;
					inst->best_sol[min_new_edge3] = 1.0;

					if (kruskal_sst(env, lp, inst) == 1) {
						if (TWO_OPT > 400)
						{
							printf("AZZERO %d;%d , %d;%d e %d;%d, e aggiungo %d;%d , %d;%d e %d;%d \n",
								nodes_edge1[0] + 1, nodes_edge1[1] + 1, nodes_edge2[0] + 1, nodes_edge2[1] + 1,
								nodes_edge3[0] + 1, nodes_edge3[1], 
								nodes_edge1[0] + 1, nodes_edge2[1] + 1, nodes_edge2[0] + 1, nodes_edge3[1] + 1,
								nodes_edge3[0] + 1, nodes_edge1[1] + 1);
							printf("PASSO DA %d,%d e %d A %d,%d e %d\n",
								old_edge1, old_edge2, old_edge3, min_new_edge1, min_new_edge2, min_new_edge3);
						}


						/*update_choosen_edge(inst);
						add_edge_to_file(inst);
						plot_gnuplot(inst);
						*/
						free(nodes_edge1);
						free(nodes_edge2);
						free(edges);
						return delta;

					}
					else {
						if (TWO_OPT > 400)
						{
							printf("NON CAMBIO NULLA\n");
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


}

/*-------------FUNCTION THAT RETURN (i,j) INTO AN ARRAY----------*/
int* reversexpos(instance* inst, int CPX_pos) {
	/*ARRAY THAT CONTAIN THE TWO NODES OF AN EDGE*/
	int* nodes = (int*)calloc(2, sizeof(int));

	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (xpos(i, j, inst) == CPX_pos) {
				if (DIST_AND_POS > 200)
				{
					printf("edge %d is (%d,%d)\n", CPX_pos, i + 1, j + 1);
				}
				nodes[0] = i;
				nodes[1] = j;
			}
		}
	}
	return nodes;
}