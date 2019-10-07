/*------------------------GENETIC ALGORITHM-----------------------*/

#include "TSP.h"
#include <Windows.h>
#include <time.h>
/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void reverse_xpos(int x, instance* inst, int* nodes);
void update_choosen_edge(instance* inst);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
double nearest_neighborhood(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);
int update_index_worst_cost_tsp(instance *inst, double *opt_cost, int num_sel_tsp);
double cost_tsp(instance *inst, int* tsp);
int* cross_over(instance *inst, int *tspFather, int *tspMother, int* son);
double update_worst_cost_population(instance *inst, double *opt_cost, int num_sel_tsp);
int index_best_cost_tsp(instance *inst, double *tsp_fitness, int num_sel_tsp);
void update_bestsol(instance *inst, int *tsp_opt);


void genetic_alg(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	int max_num_pop = 1000;						//LIMIT THE NUMBER OF CREATED POPULATION, END AFTER ANALYZE THIS NUMBER OF POPULATIONS
	int num_sel_tsp = inst->nnodes;				//NUMBER OF TSPs OF THE POPULATION(CANNOT BE LESS THAN nnodes!)
	int rows = num_sel_tsp;						//NUMBER OF POPULATION MEMBERS
	int cols = inst->nnodes;					//NUMBER OF SELECTED NODES (N BECAUSE IS A TSP PROBLEM)
	
	//MATRIX OF SOLUTIONS OF NEIGHBORHOODS TO CREATE SONS (EVERY ROW RAPPRESENT THE SOLUTION'S SET OF NODES)
	int **TSP_solutions = (int **)malloc(rows * cols * sizeof(int));
	
	double *tsp_fitness = (double *)malloc(num_sel_tsp * sizeof(double));		//NEIGHBORHOOD'S FITNESSES(COSTS) ARRAY (CONTAIN TSP COSTS OF TSP FINDED WITH HEURISTIC NEAREST NEIGHBORHOOD)
	int index_best_fitness;														//FITNESS BEST SOLUTION INDEX
	double best_fitness = 0;													//BEST SOLUTION COST
	double worst_fitness = 0;													//FITNESS(COST) WORST FATHER
	int index_worst_tsp_parent = 0;												//WORST FATHER INDEX
	int num_of_populations = 0;													//COUNTER REGARDING NUMBER OF POPULATION THAT I'M ANALIZING
	int done = 1;

	for (int i = 0; i < num_sel_tsp; i++)/*COMPUTE INITIAL POPULATION WITH NEAREST NEIGHBORHOOD*/
	{
		for (int h = 0; h < inst->ncols; h++)//RESET BEST SOLUTION
		{
			inst->best_sol[h] = 0.0;
		}

		tsp_fitness[i] = nearest_neighborhood_GRASP(inst, env, lp, i);	//TSP COST
		TSP_solutions[i] = inst->choosen_nodes;						//OPTIMAL TSP SOLUTION
		
		if (GENETIC_ALG >300)
		{			
			printf("Tsp%d fitness: %lf \n ",i, tsp_fitness[i]);
		}

		if (tsp_fitness[i] > worst_fitness)							//FIND WORST PARENT'S COST
		{
			index_worst_tsp_parent = i;
			worst_fitness = tsp_fitness[i];
		}
	}
	printf("\n");
	/*------------------------------NEXT POPULATIONS-----------------------------------*/
	int **newTSP_solutions = (int **)malloc(rows * cols * sizeof(int));;//NEW POPULATION (NEW POPULATION FORMED BY SUBSTITUTING BETTER TSPs CREATED WITH CROSSOVER WITH OLD TSPs OF PREVIOUS POPULATION)
	double *new_tsp_cost = (double *)malloc(num_sel_tsp * sizeof(double));//NEW SOLUTIONS COST
	int* son = (int*)calloc(inst->nnodes, sizeof(int));

	while (num_of_populations < max_num_pop)
	{
		if (GENETIC_ALG > 400){printf("\n Worst cost: %lf \n", worst_fitness);}


		for (int i = 0; i < num_sel_tsp; i++)//UPDATE NEW TSP SOLUTIONS
		{
			newTSP_solutions[i] = TSP_solutions[i];
		}
		for (int m = 0; m < num_sel_tsp; m++)//UPDATE NEW TSP COSTS
		{
			new_tsp_cost[m] = tsp_fitness[m];
		}


		for (int i = 0; i < num_sel_tsp; i++)	//COUPLE PARENTS AND VERIFY IF THE SON IS A GOOD BOY
		{
			for (int j = i + 1; j < num_sel_tsp; j++)
			{
				int* current_son = cross_over(inst, TSP_solutions[i], TSP_solutions[j],son);	//EDGES CURRENT SON
				double current_cost = cost_tsp(inst, current_son);								//COST CURRENT SON

				if (GENETIC_ALG > 400){printf("Current Cost %d: %lf \n", i + j,current_cost);}

				//SUBSTITUTE THE NEW BETTER TSP SON WITH A PARENT
				if (current_cost < worst_fitness)
				{
					newTSP_solutions[index_worst_tsp_parent] = current_son;
					new_tsp_cost[index_worst_tsp_parent] = current_cost;
					index_worst_tsp_parent = update_index_worst_cost_tsp(inst, new_tsp_cost, num_sel_tsp);	//UPDATE WORST TSPs INDEX
					worst_fitness = update_worst_cost_population(inst, new_tsp_cost, num_sel_tsp);			//UPDATE WORST COST
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
		
		for (int i = 0; i < num_sel_tsp; i++)//UPDATE POPULATION AND POPULATION COST
		{
			TSP_solutions[i] = newTSP_solutions[i];
			tsp_fitness[i] = new_tsp_cost[i];
		}
		
		if (GENETIC_ALG > 400)
		{
			printf("New TSPs COSTS: \n");

			for (int i = 0; i < num_sel_tsp; i++)
			{
				printf("Tsp%d fitness: %lf \n", i, tsp_fitness[i]);
			}
		}

		index_best_fitness = index_best_cost_tsp(inst, tsp_fitness, num_sel_tsp);
		printf("index_best_fitness: %d \n", index_best_fitness);
		best_fitness = tsp_fitness[index_best_fitness];
		update_bestsol(inst, TSP_solutions[index_best_fitness]);
		num_of_populations++;
		printf("Best Fitness is: %lf \n", best_fitness);

		//Sleep(1000);
	}
	free(TSP_solutions);
	free(tsp_fitness);
	free(son);
}


/*-------------------CROSS-OVER TO MERGE TWO TSPs----------------------*/
int* cross_over(instance *inst, int *tspFather, int *tspMother, int* son)
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
	int son_index = 0;			//INDEX OF SON'S ELEMENTS

	for (int i = 0; i < inst->nnodes / 2; i++)//ADD FATHER ELEMENTS
	{
		son[son_index++] = tspFather[i];		//CHOOSE FATHER NODE
		for (int j = 0; j < inst->nnodes; j++)	//SCAN MOTHER ELEMENTS
		{
			if (tspFather[i] == tspMother[j])
			{
				mother_flags[j] = 1;			//NODE ALREADY INSERTED (WILL NOT BE CHOOSE ANOTHER TIME)
			}
		}
	}
	for (int m = inst->nnodes / 2; m < inst->nnodes; m++)//ADD MOTHER ELEMENTS
	{
		if (mother_flags[m] != 1)//DON'T TAKE ELEMENTS ALREADY SELECTED FROM FATHER
		{
			son[son_index++] = tspMother[m];						//CHOOSE MOTHER NODE
			for (int f = inst->nnodes / 2; f < inst->nnodes; f++)	//SCAN FATHER ELEMENTS AND ACTIVE FLAG IF ARE PRESENT IN MOTHER 
			{
				if (tspMother[m] == tspFather[f])
				{
					father_flags[f] = 1;		//NODE ALREADY INSERTED (WILL NOT BE CHOOSE ANOTHER TIME)
				}
			}
		}
	}
	int last_son_index = son_index;

	/*----------------ADD MISSING NODES(IF THERE WERE DUPLICATES)-------------*/
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
			if (control == 0)//IF THE NODE ISN'T ALREADY IN THE SON THEN PUT IT IN AND RESTART FOR ADDING A NEW NODE
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
	//RICORDARSI DI LIBERARE son CON free(son)
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
	for (int i = 0; i < inst->nnodes-1; i++)
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

