#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);

/*-----------------------------2-OPT ALGORITHM---------------------------------------*/
double two_opt(instance *inst, CPXENVptr env, CPXLPptr lp){
	//scrivo gli archi della soluzione ottima in un array
	int* edges = (int*)calloc(inst->nnodes, sizeof(int));
	int n = 0;
	for (int i = 0; i < inst->ncols; i++) {
		if (inst->best_sol[i] > TOLERANCE) {
			edges[n] = i;
			n++;
		}
	}
	int min_delta = INFINITY;
	double edge1_length, edge2_length;
	double new_dist1, new_dist2;
	//INIZIALIZZO 2 vettori dove mettere i nodi relativi agli archi
	int* nodes_edge1 = (int*)calloc(2, sizeof(int));
	int* nodes_edge2 = (int*)calloc(2, sizeof(int));
	//scorro tutti gli archi della soluzione data all'interno di edges a coppie
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i+1; j < inst->nnodes; j++) {
			//prendo i nodi dagli archi
			reverse_xpos(edges[i], inst, nodes_edge1);
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
			new_dist1 = dist(nodes_edge1[1], nodes_edge2[0], inst);
			//guardo la differenza
			double delta = new_dist1 + new_dist2 - edge1_length - edge2_length;
			//se delta minore distanza più corta
			if (delta < min_delta) {
				//devo salvare il più corto e salvarmi tutti i nodi
				min_delta = delta;

			}
		}
	}
	free(nodes_edge1);
	free(nodes_edge2);
	free(edges);





}


void reverse_xpos(int x, instance* inst, int* nodes) {
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (xpos(i, j, inst) == x) {
				nodes[0] = i;
				nodes[1] = j;
				break;
			}
		}
	}
}