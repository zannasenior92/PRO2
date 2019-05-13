/*-----------------------NEAREST NEIGHBORHOOD---------------------------*/

#include "TSP.h"


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);


/*--------------GREEDY ALGORITHM TO FIND A INITIAL SOLUTION FOR THE TSP PROBLEM---------------*/
void nearest_neighborhood(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node)
{
	int rand_initial_v = start_node; //INITIAL NODE
	printf("Initial Node: %d \n", rand_initial_v + 1);

	double distance;//DISTANCE i-TH
	int n = 0;//SELECTED EDGES COUNTER

	int *index0 = (int*)malloc(1 * sizeof(int));					//ARRAY OF INDEXES TO CHANGE BOUND
	double *bounds0 = (double*)calloc(1, sizeof(double));			//ARRAY THAT CONTAIN THE NEW VALUE OF THE BOUND				
	char *lb0 = (char*)malloc(1 * sizeof(char));					//ARRAY THAT SPECIFIES WHAT BOUND CHANGE FOR EACH VARIABLE
	int *selected_nodes = (int*)calloc(inst->nnodes, sizeof(int));	//ARRAY OF SELECTED NODES

	while (n < inst->nnodes-1)
	{
		double near_distance = INFINITY;
		int selected_node;
		
		/*FIND n-1 NODES (n-2 EDGES); FROM ALL DIFFERENT NODES RESPECT TO THE NODE WHERE I'M NOW ;
		FROM ALL NOT SELECTED NODES*/
		for (int i = 0; i < inst->nnodes && (i != rand_initial_v) && (selected_nodes[i] != 1); i++)
		{
			distance = dist(rand_initial_v, i, inst);
			if (distance < near_distance)
			{
				near_distance = distance;
				selected_node = i;
			}
		}
		printf("Selected edge: x(%d,%d) \n", rand_initial_v + 1, selected_node + 1);
		selected_nodes[selected_node] = 1;//NODE SELECTED AN SO VISITED
		inst->choosen_edge[n] = xpos(rand_initial_v, selected_node, inst);//SAVE EDGE

		index0[0] = xpos(selected_node,rand_initial_v, inst);
		bounds0[0] = 1;
		lb0[0] = 'L';

		/*SELECT THE EDGE IN CPLEX MODEL*/
		CPXchgbds(env, lp, n, index0, lb0, bounds0);//FUNCTION TO MODIFY BOUNDS TO THE VARIABLES

		rand_initial_v = selected_node;
		printf("Now i'm in node: %d \n", rand_initial_v + 1);

		n++;
	}
	inst->choosen_edge[n] = xpos(rand_initial_v, start_node, inst);//SAVE LAST EDGE
	/*SELECT THE LAST EDGE*/
	index0[0] = xpos(rand_initial_v, start_node, inst);
	bounds0[0] = 1;
	lb0[0] = 'L';
	/*SELECT THE LAST EDGE IN CPLEX MODEL*/
	CPXchgbds(env, lp, n, index0, lb0, bounds0);//FUNCTION TO MODIFY BOUNDS TO THE VARIABLES

	printf("Last edge selected is x(%d,%d)", rand_initial_v + 1, start_node + 1);

	free(index0);
	free(bounds0);
	free(lb0);
	free(selected_nodes);
}