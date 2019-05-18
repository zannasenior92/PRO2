#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void reverse_xpos(int x, instance* inst, int* nodes);
void update_choosen_edge(instance* inst);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);

/*-----------------------------2-OPT ALGORITHM---------------------------------------*/
double two_opt(instance *inst, CPXENVptr env, CPXLPptr lp){
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

	while (done!=1){
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
				new_dist2 = dist(nodes_edge1[1], nodes_edge2[0],  inst);
				//guardo la differenza
				double delta = new_dist1 + new_dist2 - edge1_length - edge2_length;
				//se delta minore distanza più corta
				if (delta < min_delta) {
					//devo salvare il più corto e salvarmi gli archi
					/*for (int k = 0; k < inst->ncols; k++){
						if(inst->best_sol[k]>0.5)
							printf("%d\n ", k);
					}*/
					min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[1], inst);
					min_new_edge2 = xpos(nodes_edge1[1], nodes_edge2[0], inst);
					if ((inst->best_sol[min_new_edge1] == 1)|| (inst->best_sol[min_new_edge2]==1)) {
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
					
					inst->best_sol[old_edge1] = 0.0;
					inst->best_sol[old_edge2] = 0.0;
					inst->best_sol[min_new_edge1] = 1.0;
					inst->best_sol[min_new_edge2] = 1.0;
					if (TWO_OPT > 400)
					{
						printf("DOPO\n");
						printf("best_sol[%d]=%f\n", old_edge1, inst->best_sol[old_edge1]);
						printf("best_sol[%d]=%f\n", old_edge2, inst->best_sol[old_edge2]);
						printf("best_sol[%d]=%f\n", min_new_edge1, inst->best_sol[min_new_edge1]);
						printf("best_sol[%d]=%f\n", min_new_edge2, inst->best_sol[min_new_edge2]);

					}
					
					if (kruskal_sst(env, lp, inst) == 1) {
						if (TWO_OPT > 400)
						{
							printf("AZZERO %d, %d e %d, %d, e aggiungo %d, %d e %d %d \n",
								nodes_edge1[0] + 1, nodes_edge1[1] + 1, nodes_edge2[0] + 1, nodes_edge2[1] + 1,
								nodes_edge1[0] + 1, nodes_edge2[1] + 1, nodes_edge1[1] + 1, nodes_edge2[0] + 1);
							printf("PASSO DA %d e %d A %d, %d\n",
								old_edge1, old_edge2, min_new_edge1, min_new_edge2);
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
						inst->best_sol[min_new_edge1] = 0.0;
						inst->best_sol[min_new_edge2] = 0.0;
						continue;
					}
				}
			}
		}
		done = 1;

	} 
	//scorro tutti gli archi della soluzione data all'interno di edges a coppie
	
	/*
	inst->best_sol[old_edge1] = 0;
	inst->best_sol[old_edge2] = 0;
	inst->best_sol[min_new_edge1] = 1;
	inst->best_sol[min_new_edge2] = 1;
	if (kruskal_sst(env, lp, inst) != 1) {
		inst->best_sol[old_edge1] = 1;
		inst->best_sol[old_edge2] = 1;
		inst->best_sol[min_new_edge1] = 0;
		inst->best_sol[min_new_edge2] = 0;
		free(nodes_edge1);
		free(nodes_edge2);
		free(edges);
		return 0;
	}
	
	free(nodes_edge1);
	free(nodes_edge2);
	free(edges);*/
	free(nodes_edge1);
	free(nodes_edge2);
	free(edges);
	return 0;
	
}


void reverse_xpos(int x, instance* inst, int* nodes) {
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (xpos(i, j, inst) == x) {
				//printf("arco %d fatto da (%d,%d)\n", x, i + 1, j + 1);
				nodes[0] = i;
				nodes[1] = j;
				return;
			}
		}
	}
}