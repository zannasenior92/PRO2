/*-----------------------NEAREST NEIGHBORHOOD---------------------------*/

#include "TSP.h"


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);


/*--------------GREEDY ALGORITHM TO FIND A INITIAL SOLUTION FOR THE TSP PROBLEM---------------*/
double nearest_neighborhood(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node)
{
	inst->choosen_nodes = (int *)malloc(inst->nnodes * sizeof(int));
	int starting_node = start_node; //INITIAL NODE
	if (NEAREST_NEIGH > 400)
	{
		printf("Initial Node: %d \n", starting_node + 1);
	}

	double distance, nearest_distance;//DISTANCE i-TH
	int n = 0;//SELECTED EDGES COUNTER
	double cost = 0;
	int *selected_nodes = (int*)calloc(inst->nnodes, sizeof(int));	//ARRAY OF SELECTED NODES
	selected_nodes[start_node] = 1;
	inst->choosen_nodes[n] = start_node;//INITIAL NODE

	while (n < inst->nnodes-1)
	{
		nearest_distance = INFINITY;
		int selected_node= -1;
		
		/*FIND n-1 NODES (n-2 EDGES); FROM ALL DIFFERENT NODES RESPECT TO THE NODE WHERE I'M NOW ;
		FROM ALL NOT SELECTED NODES*/
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
		if (NEAREST_NEIGH >400)
		{
			printf("Selected edge: x(%d,%d) \n", starting_node + 1, selected_node + 1);
		}
		selected_nodes[selected_node] = 1;	//NODE SELECTED AN SO VISITED
		cost += nearest_distance;
		inst->best_sol[xpos(starting_node, selected_node, inst)] = 1;	//SAVE SELECTED NODE IN BEST SOLUTION
		starting_node = selected_node;									//NOW UPDATE THE NEXT STARTING NODE
		if (NEAREST_NEIGH >400)
		{
			printf("Now i'm in node: %d \n", starting_node + 1);
		}
		n++;
		inst->choosen_nodes[n] = selected_node;//SAVE THE NODE IN THE INSTANCE
	}
	inst->best_sol[xpos(starting_node, start_node, inst)] = 1;			//SELECT THE LAST EDGE TO CLOSE THE CIRCUIT
	cost += dist(starting_node, start_node, inst); 
	if (NEAREST_NEIGH >400)
	{
		printf("Last edge selected is x(%d,%d)", starting_node + 1, start_node + 1);
	}
	free(selected_nodes);
	return cost;
}