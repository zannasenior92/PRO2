#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void reverse_xpos(int x, instance* inst, int* nodes);
void update_choosen_edge(instance* inst);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void reset_old_edges2(instance *inst, int old1, int old2, int new1, int new2);
void set_new_edges2(instance *inst, int old1, int old2, int new1, int new2);


/*-----------------------------2-OPT ALGORITHM---------------------------------------*/
double two_opt(instance *inst, CPXENVptr env, CPXLPptr lp) {
	//scrivo gli archi della soluzione ottima in un array
	int* edges = (int*)calloc(inst->nnodes, sizeof(int));
	int n = 0;
	for (int i = 0; i < inst->ncols; i++) {
		if (inst->best_sol[i] > TOLERANCE) {
			edges[n] = i;
			//printf("edge[%d]=%d\n", n, i);
			n++;
		}
	}
	double min_delta = 0;
	double edge1_length, edge2_length;
	double new_dist1, new_dist2;
	int min_new_edge1, min_new_edge2;
	int old_edge1, old_edge2;
	//INIZIALIZZO 2 vettori dove mettere i nodi relativi agli archi
	int* nodes_edge1 = (int*)calloc(2, sizeof(int));
	int* nodes_edge2 = (int*)calloc(2, sizeof(int));
	int done = 0;

	while (done != 1) {
		for (int i = 0; i < inst->nnodes; i++) {
			reverse_xpos(edges[i], inst, nodes_edge1);
			for (int j = i + 1; j < inst->nnodes; j++) {
				//prendo i nodi dagli archi
				reverse_xpos(edges[j], inst, nodes_edge2);
				//controllo che non abbiano nodi in comune altrimenti non si puo fare lo scambio
				if ((nodes_edge1[0] == nodes_edge2[0]) || (nodes_edge1[0] == nodes_edge2[1]) ||
					(nodes_edge1[1] == nodes_edge2[0]) || (nodes_edge1[1] == nodes_edge2[1]))
					continue;
				//prendo la lunghezza degli archi vecchi
				edge1_length = dist(nodes_edge1[0], nodes_edge1[1], inst);
				edge2_length = dist(nodes_edge2[0], nodes_edge2[1], inst);
				//prendo la lunghezza dei nuovi presunti archi
				new_dist1 = dist(nodes_edge1[0], nodes_edge2[1], inst);
				new_dist2 = dist(nodes_edge1[1], nodes_edge2[0], inst);
				//guardo la differenza
				double delta = new_dist1 + new_dist2 - edge1_length - edge2_length;
				//se delta minore distanza più corta
				if (delta < min_delta) {

					min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[1], inst);
					min_new_edge2 = xpos(nodes_edge1[1], nodes_edge2[0], inst);
					if ((inst->best_sol[min_new_edge1] == 1) || (inst->best_sol[min_new_edge2] == 1)) {
						continue;
					}
					old_edge1 = edges[i];
					old_edge2 = edges[j];

					inst->best_sol[old_edge1] = 0.0;
					inst->best_sol[old_edge2] = 0.0;
					inst->best_sol[min_new_edge1] = 1.0;
					inst->best_sol[min_new_edge2] = 1.0;

					if (kruskal_sst(env, lp, inst) == 1) {

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
						inst->best_sol[min_new_edge1] = 0.0;
						inst->best_sol[min_new_edge2] = 0.0;
						continue;
					}
				}
			}
		}
		done = 1;

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