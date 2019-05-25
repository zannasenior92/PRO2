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
double nearest_neighborhood(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);
int index_worst_cost_tsp(instance *inst, double *opt_cost, int num_sel_tsp);
double cost_tsp(instance *inst, int* tsp);
int* cross_over(instance *inst, int *tspFather, int *tspMother);
double worst_cost_tsp(instance *inst, double *opt_cost, int num_sel_tsp);
int index_best_cost_tsp(instance *inst, double *tsp_fitness, int num_sel_tsp);
void update_bestsol(instance *inst, int *tsp_opt);


void genetic_alg(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	int max_num_pop = 100;								//END AFTER ANALYZE THIS NUMBER OF POPULATIONS
	int num_sel_tsp = inst->nnodes/2;					//NUMBER OF TSPs OF THE POPULATION
	int rows = num_sel_tsp;								//NUMBER OF POPULATION MEMBERS
	int cols = inst->nnodes;							//NUMBER OF SELECTED NODES (N BECAUSE IS A TSP PROBLEM)
	
	//MATRIX OF SOLUTIONS OF NEIGHBORHOODS TO CREATE SONS (EVERY ROW RAPPRESENT THE SOLUTION'S SET OF NODES)
	int **TSP_solutions = (int **)malloc(rows * cols * sizeof(int));
	
	double *tsp_fitness = (double *)malloc(inst->nnodes * sizeof(double));		//NEIGHBORHOOD'S FITNESSES(COSTS) ARRAY
	int index_best_fitness;														//FITNESS BEST SOLUTION INDEX
	double worst_fitness = 0;													//FITNESS(COST) WORST FATHER
	int index_worst_tsp_parent = 0;												//WORST FATHER INDEX
	int num_of_populations = 0;													//NUMBER OF POPULATIONS THAT I WANT TO ANALYZE
	int done = 1;

	for (int i = 0; i < num_sel_tsp; i++)/*COMPUTE INITIAL POPULATION WITH NEAREST NEIGHBORHOOD*/
	{
		for (int h = 0; h < inst->ncols; h++)//RESET BESTSOL
		{
			inst->best_sol[h] = 0.0;
		}
		tsp_fitness[i] = nearest_neighborhood(inst, env, lp, i);	//TSP COST
		TSP_solutions[i] = inst->choosen_nodes;						//OPTIMAL TSP SOLUTION
		if (GENETIC_ALG >400)
		{
			printf("Tsp%d Solution: ",i);

			for (int k = 0; k < inst->nnodes; k++)
			{
				printf("%d ", TSP_solutions[i][k] + 1);
			}
			printf("\n");
		}
		if (tsp_fitness[i] > worst_fitness)							//WORST PARENTS COST
		{
			index_worst_tsp_parent = i;
			worst_fitness = tsp_fitness[i];
		}
	}
	/*------------------------------NEXT POPULATIONS-----------------------------------*/
	while (num_of_populations < max_num_pop)
	{
		if (GENETIC_ALG > 400)
		{
			printf("\n");
			printf("Worst cost: %lf \n", worst_fitness);
		}
		int **newTSP_solutions = TSP_solutions;	//NEW POPULATION
		double *new_opt_cost = tsp_fitness;			//NEW SOLUTION COST
		for (int i = 0; i < num_sel_tsp; i++)	//COUPLE PARENTS AND VERIFY IF THE SON IS A GOOD BOY
		{
			for (int j = i + 1; j < num_sel_tsp; j++)
			{
				int* current_son = cross_over(inst, TSP_solutions[i], TSP_solutions[j]);	//EDGES CURRENT SON
				double current_cost = cost_tsp(inst, current_son);							//COST CURRENT SON
				//SUBSTITUTE THE NEW BETTER TSP SON WITH A PARENT
				if (GENETIC_ALG > 400)
				{
					printf("Current Cost: %lf \n", current_cost);
				}
				if (current_cost < worst_fitness)
				{
					newTSP_solutions[index_worst_tsp_parent] = current_son;
					new_opt_cost[index_worst_tsp_parent] = current_cost;
				}
			}
		}
		if (GENETIC_ALG >400)
		{
			printf("\n");
			printf("--New generation--\n\n");
			for (int u = 0; u < num_sel_tsp; u++)
			{
				printf("NewTsp%d Solution: ", u);
				for (int t = 0; t < inst->nnodes; t++)
				{
					printf("%d ", newTSP_solutions[u][t] + 1);
				}
				printf("\n");
			}
		}
		index_worst_tsp_parent = index_worst_cost_tsp(inst, new_opt_cost, num_sel_tsp);	//WORST TSPS COST
		TSP_solutions = newTSP_solutions;	//THE NEW UPDATED POPULATION
		tsp_fitness = new_opt_cost;
		worst_fitness = worst_cost_tsp(inst, tsp_fitness, num_sel_tsp);//UPDATE WORST COST
		index_best_fitness = index_best_cost_tsp(inst, tsp_fitness, num_sel_tsp);
		update_bestsol(inst, TSP_solutions[index_best_fitness]);
		num_of_populations++;

	}
	printf("Worst Fitness is: %lf \n", worst_fitness);
	free(TSP_solutions);
	free(tsp_fitness);
}


/*-------------------CROSS-OVER TO MERGE TWO TSPs----------------------*/
int* cross_over(instance *inst, int *tspFather, int *tspMother)
{
	int* father_flags = (int*)calloc(inst->nnodes, sizeof(int));
	int* mother_flags = (int*)calloc(inst->nnodes, sizeof(int));

	if (GENETIC_ALG > 600)
	{
		printf("\n");
		printf("Nodes of the father are:\n");
		for(int i = 0; i < inst->nnodes; i++)
		{
			printf("%d ", tspFather[i] + 1);
		}
		printf("\n");
		printf("Nodes of the mother are:\n");
		for (int i = 0; i < inst->nnodes; i++)
		{
			printf("%d ", tspMother[i] + 1);
		}
		printf("\n");
	}
	int count_duplicate = 0;	//DUPLICATE ELEMENT IN FATHER AND MOTHER
	int* son = (int*)calloc(inst->nnodes, sizeof(int));
	int son_index = 0;			//INDEX OF SON'S ELEMENTS

	for (int i = 0; i < inst->nnodes / 2; i++)//ADD FATHER ELEMENTS
	{
		son[son_index++] = tspFather[i];	//CHOOSE FATHER NODE

		for (int j = 0; j < inst->nnodes; j++)//SCAN MOTHER ELEMENTS
		{
			if (tspFather[i] == tspMother[j])
			{
				mother_flags[j] = 1;			//NODE ALREADY INSERTED (WILL NOT BE CHOOSE ANOTHER TIME)
			}
		}
	}
	for (int m = inst->nnodes / 2; m < inst->nnodes; m++)//ADD MOTHER ELEMENTS
	{
		if (mother_flags[m] != 1)//DON'T TAKE ELEMENTS ALREADY PRESENT IN THE SON
		{
			son[son_index++] = tspMother[m];	//CHOOSE MOTHER NODE
			for (int f = inst->nnodes / 2; f < inst->nnodes; f++)//SCAN FATHER ELEMENTS
			{
				if (tspMother[m] == tspFather[f])
				{
					father_flags[f] = 1;		//NODE ALREADY INSERTED (WILL NOT BE CHOOSE ANOTHER TIME)
				}
			}

		}
	}
	int last_son_index = son_index;
	/*----------------ADD MISSING NODES-------------*/
	for (int l = 0; l < inst->nnodes - last_son_index; l++)
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
	if (GENETIC_ALG > 600)
	{
		printf("Nodes of the son are:\n");
		for (int i = 0; i < inst->nnodes; i++)
		{
			printf("%d ", son[i] + 1);
		}
		printf("\n");
	}
	free(mother_flags);
	free(father_flags);
	return son;
}

/*OPT VAL OF A TSP*/
double cost_tsp(instance *inst, int* tsp)
{

	double cost = 0;
	int n = 0;
	while (n < inst->nnodes -1)
	{
		int n1 = tsp[n];
		int n2 = tsp[n + 1];
		cost += dist(n1, n2, inst);
		n++;
	}

	cost += dist(tsp[inst->nnodes - 1], tsp[0], inst);
	return cost;
}

/*WORST SOLUTION OF TSPS PARENTS*/
int index_worst_cost_tsp(instance *inst, double *opt_cost, int num_sel_tsp)
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

double worst_cost_tsp(instance *inst, double *tsp_fitness, int num_sel_tsp)
{
	double worst_cost = 0;
	for (int i = 0; i < num_sel_tsp; i++)
	{
		if (tsp_fitness[i] > worst_cost)
		{
			worst_cost = tsp_fitness[i];
		}
	}
	return worst_cost;
}

int index_best_cost_tsp(instance *inst, double *tsp_fitness, int num_sel_tsp)
{
	double best_cost = INFINITY;
	int index_best = 0;
	for (int i = 0; i < num_sel_tsp; i++)
	{
		if (tsp_fitness[i] < best_cost)
		{
			best_cost = tsp_fitness[i];
			index_best = i;
		}
	}
	return index_best;
}

void update_bestsol(instance *inst, int *tsp_opt)
{
	for (int h = 0; h < inst->ncols; h++)//RESET BESTSOL
	{
		inst->best_sol[h] = 0.0;
	}
	for (int i = 0; i < inst->nnodes-1; i++)
	{
		inst->best_sol[xpos(tsp_opt[i], tsp_opt[i + 1], inst)] = 1.0;
	}
	inst->best_sol[xpos(tsp_opt[inst->nnodes - 1], tsp_opt[0], inst)] = 1.0;
}

