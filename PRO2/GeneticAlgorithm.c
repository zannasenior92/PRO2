/*------------------------GENETIC ALGORITHM-----------------------*/

#include "TSP.h"
/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void reverse_xpos(int x, instance* inst, int* nodes);
void update_choosen_edge(instance* inst);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
void reset_old_edges3(instance *inst, int old1, int old2, int old3, int new1, int new2, int new3);
void set_new_edges3(instance *inst, int old1, int old2, int old3, int new1, int new2, int new3);
double nearest_neighborhood(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);


void genetic_alg(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	int rows = inst->nnodes;
	int cols = inst->nnodes;
	
	//MATRIX OF SOLUTIONS OF NEIGHBORHOODS TO CREATE SONS (EVERY ROW RAPPRESENT A SET OF NODES OF THE SOLUTION)
	double **TSP_solutions = (double *)malloc(rows * cols * sizeof(int));	
	double *opt_cost = (double *)malloc(inst->nnodes * sizeof(double));		//ARRAY NEIGHBORHOOD'S COSTS
	double opt_VAL = INFINITY;												//VALORE DELLA SOLUZIONE OTTIMA
	int done = 1;
	double worst_cost = INFINITY;											//COSTO DEL PEGGIOR TSP PADRE
	int index_worst_tsp_parent = 0;											//INDICE PEGGIOR TSP PADRE
	int num_sel_tsp = inst->nnodes;											//NUMERO DI TSP PRESI IN CONSIDERAZIONE OGNI VOLTA
											  
	/*COMPUTE INITIAL POPULATION WITH NEIGHBORHOOD*/
	for (int i = 0; i < num_sel_tsp; i++)
	{
		
		opt_cost[i] = nearest_neighborhood(inst, env, lp, i);	//TSP COST
		TSP_solutions[i] = inst->choosen_nodes;						//OPTIMAL TSP SOLUTION

		
		if (opt_cost[i] < worst_cost)							//WORST PARENTS COST
		{
			index_worst_tsp_parent = i;
			worst_cost = opt_cost[i];
		}
	}

	while (done == 1)
	{
		for (int i = 0; i < num_sel_tsp; i++)//COUPLE PARENTS AND VERIFY IF THE SON IS A GOOD BOY
		{
			for (int j = 0; j < num_sel_tsp; j++)
			{
				double* current_son = cross_over(inst, TSP_solutions[i], TSP_solutions[j]);	//EDGES CURRENT SON
				double current_cost = cost_tsp(inst, current_son);							//COST CURRENT SON
				
				//SUBSTITUTE THE NEW BETTER TSP SON WITH A PARENT
				if (current_cost < worst_cost)
				{
					TSP_solutions[index_worst_tsp_parent] = current_son;
					opt_cost[i] = current_cost;
				}
			}
		}
		index_worst_tsp_parent = worst_cost_tsp(inst, opt_cost, num_sel_tsp);	//WORST TSPS COST
	}
}


/*CROSS-OVER TO MERGE TWO TSPs*/
double* cross_over(instance *inst, double* tspFather, double* tspMother)
{
	if (GENETIC_ALG > 500)
	{
		printf("Nodes of the father are:\n");
		for(int i = 0; i < inst->nnodes; i++)
		{
			printf("%d \n", tspFather[i]);
		}

		printf("Nodes of the mother are:\n");
		for (int i = 0; i < inst->nnodes; i++)
		{
			printf("%d \n", tspMother[i]);
		}

	}
	int count_duplicate;	//DUPLICATE ELEMENT IN FATHER AND MOTHER
	double* son = (double*)calloc(inst->nnodes, sizeof(double));
	int son_index = 0;			//INDEX OF SON'S ELEMENTS

	for (int i = 0; i < inst->nnodes / 2; i++)//ADD FATHER ELEMENTS
	{
		son[son_index++] = tspFather[i];	//CHOOSE FATHER NODE

		for (int j = 0; j < inst->nnodes; j++)//SCAN MOTHER ELEMENTS
		{
			if (tspFather[i] == tspMother[j])
			{
				tspMother[j] = -1;			//NODE ALREADY INSERTED (WILL NOT BE CHOOSE ANOTHER TIME)
			}
		}
	}
	for (int m = inst->nnodes / 2; m < inst->nnodes; m++)//ADD MOTHER ELEMENTS
	{
		if (tspMother[m] != -1)//DON'T TAKE ELEMENTS ALREADY PRESENT IN THE SON
		{
			son[son_index++] = tspMother[m];	//CHOOSE MOTHER NODE
			for (int f = inst->nnodes / 2; f < inst->nnodes; f++)//SCAN FATHER ELEMENTS
			{
				if (tspMother[m] == tspFather[f])
				{
					tspFather[f] = -1;		//NODE ALREADY INSERTED (WILL NOT BE CHOOSE ANOTHER TIME)
				}
			}

		}
	}
	/*----------------ADD MISSING NODES-------------*/
	for (int l = 0; l < inst->nnodes - son_index; l++)
	{
		int rand_node; //CHOOSE A RANDOM NODE
		int done = 0;
		while (done==0)
		{
			rand_node = rand() % inst->nnodes;
			int control = 0;
			for (int i = 0; i < son_index; i++)//CONTROL IF THE NEW RANDOM NODE IS ALREADY IN THE SON
			{
				if (rand_node == son[i])
				{
					control = 1;
				}
			}
			if (control == 0)
			{
				son[son_index++] = rand_node;
				done = 1;
			}
		}
	}
	if (GENETIC_ALG > 500)
	{
		printf("Nodes of the son are:\n");
		for (int i = 0; i < inst->nnodes; i++)
		{
			printf("%d \n", son[i]);
		}
	}

	return son;
}

/*OPT VAL OF A TSP*/
double cost_tsp(instance *inst, double* tsp)
{
	double cost = 0;
	for (int i = 0; i < inst->nnodes-1; i+2) 
	{
		int n1 = tsp[i];
		int n2 = tsp[i + 1];
		cost += dist(n1, n2, inst);
	}
	cost += dist(tsp[inst->nnodes - 1], tsp[0], inst);
	return cost;
}

/*WORST SOLUTION OF TSPS PARENTS*/
int worst_cost_tsp(instance *inst, double *opt_cost, int num_sel_tsp)
{
	double worst_cost = 0;
	int index_worst = 0;
	for (int i = 0; i < num_sel_tsp; i++)
	{
		if (opt_cost[i] > worst_cost)
		{
			worst_cost = opt_cost[i];
			index_worst = i;
		}
	}
	return index_worst;
}
