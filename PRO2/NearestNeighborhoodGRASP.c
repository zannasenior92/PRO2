#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);


/*--------------GREEDY ALGORITHM TO FIND A INITIAL SOLUTION FOR THE TSP PROBLEM---------------*/
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node)
{
	int starting_node = start_node; //INITIAL NODE
	printf("Initial Node: %d \n", starting_node + 1);

	double distance, nearest_distance;//DISTANCE i-TH
	int n = 0;//SELECTED EDGES COUNTER
	double cost = 0;
	int *selected_nodes = (int*)calloc(inst->nnodes, sizeof(int));	//ARRAY OF SELECTED NODES
	selected_nodes[start_node] = 1;
	int j = 0;
	srand(time(NULL));
	double random;
	while (n < inst->nnodes - 1)
	{
		int *nearest_three_nodes = (int*)calloc(3, sizeof(int)); //salvo i 3 nodi più vicini
		int nearest_selected = 0;
		int selected_node;
		random = (double)rand() / (double)RAND_MAX;
		printf("IL RANDOM VALE %f\n", random);
		j = 0;
		while (j < 3) {
			nearest_distance = INFINITY;
			selected_node = -1;
			//TROVO IL NODO PIù VICINO AL NODO DI PARTENZA
			for (int i = 0; i < inst->nnodes; i++)
			{
				if ((i == starting_node) || (selected_nodes[i] == 1)) {
					continue;
				}
				distance = dist(starting_node, i, inst);
				if (distance < nearest_distance)
				{
					nearest_distance = distance;
					selected_node = i;
				}
			}
			//CONTROLLARE QUESTO IF (NON CI VA UN BREAK?)
			if (selected_node == -1) {
				continue;
			}

			if (random < 0.5) {
				printf("SCELTA NORMALE\n");
				printf("Selected edge: x(%d,%d) \n", starting_node + 1, selected_node + 1);
				selected_nodes[selected_node] = 1;//NODE SELECTED AN SO VISITED
				cost += nearest_distance;
				inst->best_sol[xpos(starting_node, selected_node, inst)] = 1;
				starting_node = selected_node;
				printf("Now i'm in node: %d \n", starting_node + 1);
				nearest_selected = 1;
				n++;
				break;
			}
			/*FIND n-1 NODES (n-2 EDGES); FROM ALL DIFFERENT NODES RESPECT TO THE NODE WHERE I'M NOW ;
			FROM ALL NOT SELECTED NODES*/
			nearest_three_nodes[j] = selected_node;
			selected_nodes[selected_node] = 1;//NODE SELECTED AN SO VISITED
			//printf("Now i'm in node: %d \n", starting_node + 1);
			j++;
		}
		if (nearest_selected == 1) {
			continue;
		}
		printf("SCELTA RANDOM\n");
		int selected = rand() % 3;
		printf("IL SELECTED RANDOM VALE %d\n", selected);
		printf("scelgo %d tra: ", nearest_three_nodes[selected]);
		for (int stampa = 0; stampa < 3; stampa++) {
			printf(" %d ", nearest_three_nodes[stampa]);
		}
		printf("\n");
		selected_node = nearest_three_nodes[selected];
		selected_nodes[selected_node] = 1;//NODE SELECTED AN SO VISITED
		//DESELEZIONO GLI ALTRI 2 NODI
		for (int k = 0; k < 3; k++) {
			if (nearest_three_nodes[k] != selected_node) {
				selected_nodes[nearest_three_nodes[k]] = 0;
			}
		}
		cost += dist(selected_node, starting_node, inst);
		inst->best_sol[xpos(starting_node, selected_node, inst)] = 1;
		starting_node = selected_node;
		//printf("Now i'm in node: %d \n", starting_node + 1);
		n++;
		free(nearest_three_nodes);

	}
	printf("Sono uscito dal for\n");
	inst->best_sol[xpos(starting_node, start_node, inst)] = 1;
	cost += dist(starting_node, start_node, inst);
	//printf("Last edge selected is x(%d,%d)", starting_node + 1, start_node + 1);
	free(selected_nodes);
	return cost;
}