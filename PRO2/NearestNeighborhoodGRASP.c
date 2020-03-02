#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
double cost_alg(instance* inst);

/*--------------GREEDY ALGORITHM TO FIND A INITIAL SOLUTION FOR THE TSP PROBLEM---------------*/
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node, int seed)
{
	inst->choosen_nodes = (int *)malloc(inst->nnodes * sizeof(int));
	int starting_node = start_node; //INITIAL NODE
	if (NEAREST_NEIGH_GRASP > 400)
	{
		printf("Initial Node: %d \n", starting_node + 1);
	}

	double distance, nearest_distance;									//DISTANCE i-TH
	int n = 0;															//SELECTED EDGES COUNTER
	double cost = 0;													//COST OF THE SOLUTION
	int *selected_nodes = (int*)calloc(inst->nnodes, sizeof(int));		//ARRAY OF SELECTED NODES
	selected_nodes[start_node] = 1;										//START NODE SELECTED
	inst->choosen_nodes[n] = start_node;								//INITIAL NODE
	int j = 0;
	srand(seed);
	double random;														//START NODE

	/*------------------------FIND THE CIRCUIT------------------------*/
	while (n < inst->nnodes - 1)
	{
		int *nearest_three_nodes = (int*)calloc(3, sizeof(int));		//SAVE THREE NEAREST NODES
		int nearest_selected = 0;										//SELECT THE NEAREST NODE CHOICE
		int selected_node;												//THE CURRENT SELECTED NODE
		random = (double)rand() / (double)RAND_MAX;

		j = 0;															//INDEX OF THREE NEAREST NODES
		selected_node = -1;

		/*THREE NEAREST NODE SELECTION*/
		while (j < 3) {
			nearest_distance = INFINITY;
			//FIND NEAREST NODE RESPECT TO THE START NODE
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

			if (selected_node == -1) {
				continue;
			}

			/*---------NORMAL CHOICE THAT CHOOSE ONE NODE(THE NEAREST)---------*/
			if (random < 0.5) {
				if (NEAREST_NEIGH_GRASP > 400)
				{
					printf("NORMAL CHOICE\n");
					printf("Selected edge: x(%d,%d) \n", starting_node + 1, selected_node + 1);
				}
				selected_nodes[selected_node] = 1;							//NODE SELECTED AN SO VISITED
				cost += nearest_distance;
				inst->best_sol[xpos(starting_node, selected_node, inst)] = 1;
				starting_node = selected_node;								//UPDATE THE STARTING NODE
				if (NEAREST_NEIGH_GRASP > 400)
				{
					printf("Now i'm in node: %d \n", starting_node + 1);
				}
				nearest_selected = 1;
				n++;
				inst->choosen_nodes[n] = selected_node;						//SAVE THE NODE IN THE INSTANCE
				break;
			}
			/*FIND n-1 NODES (n-2 EDGES); FROM ALL DIFFERENT NODES RESPECT TO THE NODE WHERE I'M NOW ;
			FROM ALL NOT SELECTED NODES*/
			nearest_three_nodes[j] = selected_node;
			selected_nodes[selected_node] = 1;								//NODE SELECTED AN SO VISITED
			if (NEAREST_NEIGH_GRASP > 400)
			{
				printf("Now i'm in node: %d \n", starting_node + 1);
			}
			j++;
		}
		if (nearest_selected == 1) {
			continue;
		}
		int selected = rand() % 3;
		if (NEAREST_NEIGH_GRASP > 400)
		{
			printf("SCELTA RANDOM\n");
			printf("scelgo %d tra: ", nearest_three_nodes[selected]);
			for (int stampa = 0; stampa < 3; stampa++) {
				printf(" %d ", nearest_three_nodes[stampa]);
			}
			printf("\n");
		}

		selected_node = nearest_three_nodes[selected];
		selected_nodes[selected_node] = 1;								//ONE NODE (OF THE THREES) SELECTED AN SO VISITED


		/*---------------DESELECT THE OTHER TWO NODES---------------------*/
		for (int k = 0; k < 3; k++) {
			if (nearest_three_nodes[k] != selected_node) {
				selected_nodes[nearest_three_nodes[k]] = 0;
			}
		}
		cost += dist(selected_node, starting_node, inst);
		inst->best_sol[xpos(starting_node, selected_node, inst)] = 1;
		starting_node = selected_node;										//UPDATE THE STARTING NODE
		if (NEAREST_NEIGH_GRASP > 400)
		{
			printf("Now i'm in node: %d \n", starting_node + 1);
		}
		n++;
		inst->choosen_nodes[n] = selected_node;							//SAVE THE NODE IN THE INSTANCE
		free(nearest_three_nodes);
	}
	inst->best_sol[xpos(starting_node, start_node, inst)] = 1;
	cost += dist(starting_node, start_node, inst);
	if (NEAREST_NEIGH_GRASP > 400)
	{
		printf("Last edge selected is x(%d,%d)", starting_node + 1, start_node + 1);
	}
	free(selected_nodes);

	return cost;
}