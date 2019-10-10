#include "TSP.h"
#include <Windows.h>
#include <time.h>

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
int xpos(int i, int j, instance *inst);
double dist(int i, int j, instance *inst);

/*OPT VAL OF A TSP*/
double cost_tsp(instance *inst, int* tsp)
{
	double cost = 0;
	int n = 0;
	while (n < inst->nnodes - 1)
	{
		int n1 = tsp[n];
		int n2 = tsp[n + 1];
		cost += dist(n1, n2, inst);
		n++;
	}

	cost += dist(tsp[inst->nnodes - 1], tsp[0], inst);
	return cost;
}

/*COMPUTE THE WORST SOLUTION INDEX OF TSPs PARENTS*/
int update_index_worst_cost_tsp(instance *inst, double *tsp_fitness, int num_sel_tsp)
{
	double worst_cost = 0;
	int index_worst = 0;
	for (int i = 0; i < num_sel_tsp; i++)
	{
		if (tsp_fitness[i] > worst_cost)
		{
			worst_cost = tsp_fitness[i];
			index_worst = i;
		}
	}
	return index_worst;
}

/*COMPUTE THE WORST COST OF THE POPULATION OF TSPs*/
double update_worst_cost_population(instance *inst, double *tsp_fitness, int num_sel_tsp)
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

/*UPDATE BEST SOLUTION*/
void update_bestsol(instance *inst, int *tsp_opt)
{
	for (int h = 0; h < inst->ncols; h++)//RESET BEST SOLUTION
	{
		inst->best_sol[h] = 0.0;
	}
	for (int i = 0; i < inst->nnodes - 1; i++)
	{
		inst->best_sol[xpos(tsp_opt[i], tsp_opt[i + 1], inst)] = 1.0;
	}
	inst->best_sol[xpos(tsp_opt[inst->nnodes - 1], tsp_opt[0], inst)] = 1.0;
}

/**********************************************************************************************/
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
