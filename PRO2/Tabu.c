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
double tabu_search(instance *inst, CPXENVptr env, CPXLPptr lp) {
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
	double min_delta = INFINITY;
	double edge1_length, edge2_length;
	double new_dist1, new_dist2;
	int min_new_edge1, min_new_edge2;
	int old_edge1, old_edge2;
	//INIZIALIZZO 2 vettori dove mettere i nodi relativi agli archi
	int* nodes_edge1 = (int*)calloc(2, sizeof(int));
	int* nodes_edge2 = (int*)calloc(2, sizeof(int));
	int done = 0;
	int in_tabu = 0;
	int candidate_edge1, candidate_edge2, candidate_old_edge1, candidate_old_edge2;
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
					//devo salvare il più corto e salvarmi gli archi
					
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
						//CONTROLLO SE SONO NELLA TABU LIST
						if (inst->tabu_index != 0) {
							for (int k = 0; k < inst->tabu_index; k += 4) {
								//printf("controllo %d - %d / %d - %d / %d - %d / %d - %d\n", min_new_edge1, inst->tabu_list[k], old_edge1,
									//inst->tabu_list[k + 1], min_new_edge2, inst->tabu_list[k + 2], old_edge2, inst->tabu_list[k + 3]);
								if (((min_new_edge1 == inst->tabu_list[k]) && (old_edge1 == inst->tabu_list[k + 1])) &&
									((min_new_edge2 == inst->tabu_list[k + 2]) && (old_edge2 == inst->tabu_list[k + 3]))) {
									//printf("Gia presenti nella tabu list %d e %d \n", min_new_edge1, min_new_edge2);
									//Ritorno alla soluzione originale
									inst->best_sol[old_edge1] = 1.0;
									inst->best_sol[old_edge2] = 1.0;
									inst->best_sol[min_new_edge1] = 0.0;
									inst->best_sol[min_new_edge2] = 0.0;
									in_tabu = 1;
									break;
								}

							}
						}
						//mi salvo il minore delta trovato e gli archi associati
						//la prima volta che faccio tabu search salto lo scambio che mi da delta=0
						if (inst->tabu_flag == 0) {
							if (delta == 0) continue;
							else {
								if (in_tabu != 0) {
									in_tabu = 0;
									continue;
								}
								else{
									min_delta = delta;
									candidate_edge1 = min_new_edge1;
									candidate_edge2 = min_new_edge2;
									candidate_old_edge1 = old_edge1;
									candidate_old_edge2 = old_edge2;
									//printf("salvo il delta minimo %f dato da %d con %d e %d con %d\n", min_delta, candidate_edge1,
										//candidate_old_edge1, candidate_edge2, candidate_old_edge2);
								}
							}
						}
						else {
							if (in_tabu != 0) {
								in_tabu = 0;
								continue;
							}
							else {
								min_delta = delta;
								candidate_edge1 = min_new_edge1;
								candidate_edge2 = min_new_edge2;
								candidate_old_edge1 = old_edge1;
								candidate_old_edge2 = old_edge2;
								//printf("salvo il delta minimo %f dato da %d con %d e %d con %d\n", min_delta, candidate_edge1,
									//candidate_old_edge1, candidate_edge2, candidate_old_edge2);
							}
						}
						
						//Ritorno alla soluzione originale
						inst->best_sol[old_edge1] = 1.0;
						inst->best_sol[old_edge2] = 1.0;
						inst->best_sol[min_new_edge1] = 0.0;
						inst->best_sol[min_new_edge2] = 0.0;
						continue;
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
	
	inst->best_sol[candidate_old_edge1] = 0.0;
	inst->best_sol[candidate_old_edge2] = 0.0;
	inst->best_sol[candidate_edge1] = 1.0;
	inst->best_sol[candidate_edge2] = 1.0;
	//aggiungo i lati nella tabu list
	if (inst->tabu_index == 200) {
		inst->tabu_index = 0;
		printf("azzero l'indice tabu index\n");
	}
	//se delta maggiore di zero aggiungo archi a tabu list
	if (min_delta > 0) {
		inst->tabu_list[inst->tabu_index] = candidate_old_edge1;
		inst->tabu_list[inst->tabu_index + 1] = candidate_edge1;
		inst->tabu_list[inst->tabu_index + 2] = candidate_old_edge2;
		inst->tabu_list[inst->tabu_index + 3] = candidate_edge2;
		inst->tabu_index += 4;

		//printf("Aggiungo a tabu list %d - %d - %d - %d \n", candidate_old_edge1, candidate_edge1, candidate_old_edge2, candidate_edge2);
	}
	printf("DELTA MIN  %f \n", min_delta);
	printf("scambio %d con %d e %d con %d\n", candidate_old_edge1, candidate_edge1, candidate_old_edge2, candidate_edge2);
	if(inst->tabu_flag ==0) inst->tabu_flag = 1;
	printf("-----------tabu list---------------\n");
	for (int k = 1; k < inst->tabu_index+1; k ++) {
		printf("%d = %d\n",k, inst->tabu_list[k]);
		if(k%4==0)
			printf("----------------\n");
	}

	free(nodes_edge1);
	free(nodes_edge2);
	free(edges);

	return min_delta;

}


