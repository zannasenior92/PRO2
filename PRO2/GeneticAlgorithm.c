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
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node, int seed);
int update_index_worst_cost_tsp(instance *inst, double *opt_cost, int num_sel_tsp);
double cost_tsp(instance *inst, int* tsp);
int* cross_over(instance *inst, int *tspFather, int *tspMother);
int* mutation(instance* inst, int *tspParent, int* son);
double update_worst_cost_population(instance *inst, double *opt_cost, int num_sel_tsp);
int index_best_cost_tsp(instance *inst, double *tsp_fitness, int num_sel_tsp);
void update_bestsol(instance *inst, int *tsp_opt);


void genetic_alg(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	srand((unsigned int)time(NULL));							//NEW RANDOM NUMBERS

	int max_num_pop = 10000;					//LIMIT THE NUMBER OF CREATED POPULATION, END AFTER ANALYZE THIS NUMBER OF POPULATIONS
	int num_sel_tsp = 100;						//NUMBER OF TSPs OF THE POPULATION(CANNOT BE LESS THAN nnodes!)
	int rows = num_sel_tsp;						//NUMBER OF POPULATION MEMBERS
	int cols = inst->nnodes;					//NUMBER OF SELECTED NODES (N BECAUSE IS A TSP PROBLEM)
	
	//MATRIX OF SOLUTIONS OF NEIGHBORHOODS TO CREATE SONS (EVERY ROW RAPPRESENT THE SOLUTION'S SET OF NODES)
	int **TSP_solutions = (int **)malloc(rows * cols * sizeof(int));
	
	double *TSP_fitness = (double *)malloc(num_sel_tsp * sizeof(double));		//NEIGHBORHOOD'S FITNESSES(COSTS) ARRAY (CONTAIN TSP COSTS OF TSP FINDED WITH HEURISTIC NEAREST NEIGHBORHOOD)
	int index_best_fitness;														//FITNESS BEST SOLUTION INDEX
	double best_fitness = 0;													//BEST SOLUTION COST
	double worst_fitness = 0;													//FITNESS(COST) WORST FATHER
	int index_worst_tsp_parent = 0;												//WORST FATHER INDEX
	int num_of_populations = 0;													//COUNTER REGARDING NUMBER OF POPULATION THAT I'M ANALIZING
	int done = 1;

	for (int i = 0; i < num_sel_tsp; i++)//COMPUTE INITIAL POPULATION WITH NEAREST NEIGHBORHOOD
	{
		for (int h = 0; h < inst->ncols; h++)//RESET BEST SOLUTION
		{
			inst->best_sol[h] = 0.0;
		}

		TSP_fitness[i] = nearest_neighborhood_GRASP(inst, env, lp, 1,i);	//TSP COST
		TSP_solutions[i] = inst->choosen_nodes;						//OPTIMAL TSP SOLUTION
		
	
		if (GENETIC_ALG >400){
			printf("Initial fitness Tsp%d : %lf \n ",i, TSP_fitness[i]);}

		if (TSP_fitness[i] > worst_fitness)							//FIND WORST PARENT'S COST
		{
			index_worst_tsp_parent = i;
			worst_fitness = TSP_fitness[i];
		}
	}

	index_best_fitness = index_best_cost_tsp(inst, TSP_fitness, num_sel_tsp);
	best_fitness = TSP_fitness[index_best_fitness];
	update_bestsol(inst, TSP_solutions[index_best_fitness]);
	printf("Start Best Fitness is: %lf \n", best_fitness);
	printf("\n");

	Sleep(4000);

	/*------------------------------NEXT POPULATIONS-----------------------------------*/
	int **newTSP_solutions = (int **)malloc(rows * cols * sizeof(int)); //NEW POPULATION (NEW POPULATION FORMED BY SUBSTITUTING BETTER TSPs CREATED WITH CROSSOVER WITH OLD TSPs OF PREVIOUS POPULATION)
	double *newTSP_fitness = (double *)malloc(num_sel_tsp * sizeof(double)); //NEW SOLUTIONS COST
	double choice_of_new_generation;							//RANDOM CHOICE FOR CREATE NEW POPULATION
	double choice_add_bad;					//RANDOM CHOICE FOR ADD IN A NEW POPULATION A BAD MEMBER
	int c_index_sub;						//RANDOM INDEX FOR TSP SUBSTITUTION

	for (int j = 0; j < num_sel_tsp; j++)
	{
		newTSP_solutions[j] = TSP_solutions[j];
		newTSP_fitness[j] = TSP_fitness[j];
	}
	/*-----------------------------GENERO LE POPOLAZIONI-------------------------------*/
	while (num_of_populations < max_num_pop)
	{
		if (GENETIC_ALG > 400){printf("\n Worst cost: %lf \n", worst_fitness);}

		choice_of_new_generation = (double)rand() / (double)RAND_MAX;	//GENERATE A RANDOM CHOICE
		choice_add_bad = (double)rand() / (double)RAND_MAX;				//GENERATE A RANDOM CHOICE FOR ADD A BAD MEMBER TO A NEW POPULATION
		
		int* current_son1;												//SON GENERATED FROM PARENTS WITH CROSS-OVER OR MUTATION
		int* current_son2;
		double current_cost1;											//COST OF SON
		double current_cost2;
		int new_tsp_index = 0;

			for (int i = 0; i < num_sel_tsp; i++)
			{
				for (int j = i + 1; j < num_sel_tsp; j++)
				{
					current_son1 = cross_over(inst, TSP_solutions[i], TSP_solutions[j]);	//NODES CURRENT SON
					current_son2 = cross_over(inst, TSP_solutions[j], TSP_solutions[i]);

					current_cost1 = cost_tsp(inst, current_son1);									//COST CURRENT SON
					current_cost2 = cost_tsp(inst, current_son2);
					if (GENETIC_ALG > 400) 
					{ 
						printf("Current1 son's Cost %d: %lf \n", i + j, current_cost1);
						printf("Current2 son's Cost %d: %lf \n", i + j, current_cost2);
					}

					/*STO SBAGLIANDO PERCHE' STO INSERENDO I PRIMI CROSS-OVER IN CUI CAMBIA SOLO  CHE FACCIO I QUALI SONO PIU' O 
					MENO TUTTI UGUALI E QUINDI SE PER CASO UN CROSS-OVER E' BUONO DI SICURO TUTTI GLI ALTRI NON RIENTRANO NELL'ARRAY*/
					if (new_tsp_index < num_sel_tsp)
					{
						newTSP_solutions[new_tsp_index] = current_son1;
						newTSP_fitness[new_tsp_index] = current_cost1;
						new_tsp_index++;
						index_worst_tsp_parent = update_index_worst_cost_tsp(inst, newTSP_fitness, num_sel_tsp);	//UPDATE WORST TSPs INDEX
						worst_fitness = update_worst_cost_population(inst, newTSP_fitness, num_sel_tsp);			//UPDATE WORST COST
					}
					else
					{
						//SUBSTITUTE THE NEW BETTER TSP SON WITH A PARENT
						if (current_cost1 < worst_fitness)
						{
							newTSP_solutions[index_worst_tsp_parent] = current_son1;
							newTSP_fitness[index_worst_tsp_parent] = current_cost1;
							index_worst_tsp_parent = update_index_worst_cost_tsp(inst, newTSP_fitness, num_sel_tsp);	//UPDATE WORST TSPs INDEX
							worst_fitness = update_worst_cost_population(inst, newTSP_fitness, num_sel_tsp);			//UPDATE WORST COST
						}
						
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
			TSP_fitness[i] = newTSP_fitness[i];
		}
		
		if (GENETIC_ALG > 1)
		{
			printf("New TSPs fitness: \n");

			for (int i = 0; i < num_sel_tsp; i++)
			{
				printf("Tsp%d fitness: %lf \n", i, TSP_fitness[i]);
			}
		}
		printf("\n");
		index_best_fitness = index_best_cost_tsp(inst, TSP_fitness, num_sel_tsp);
		best_fitness = TSP_fitness[index_best_fitness];
		update_bestsol(inst, TSP_solutions[index_best_fitness]);

		printf("Worst fitness index: %d \n", index_worst_tsp_parent);
		printf("Worst fitness:       %lf \n", worst_fitness);
		printf("Best fitness index:  %d \n", index_best_fitness);
		printf("Best Fitness :       %lf \n", best_fitness);
		printf("\n");
		num_of_populations++;

		//Sleep(5000);
		
	}
	free(newTSP_fitness);
	free(newTSP_solutions);
	free(TSP_solutions);
	free(TSP_fitness);
}
/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/

/*-------------------CROSS-OVER TO MERGE TWO TSPs----------------------*/
int* cross_over(instance *inst, int *tspFather, int *tspMother)
{
	int* new_SON = (int*)calloc(inst->nnodes, sizeof(int));
	int rand_cut = rand() % inst->nnodes;						//CUT WHERE TO CREATE SON WITH SOME FATHER'S AND MOTHER'S CHROMOSOMES
	int* son_flags = (int*)calloc(inst->nnodes, sizeof(int));	//FLAG WHO REPRESENT HOW NODE IS INSERTED
	
	if (GENETIC_ALG > 400)
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
	int count_duplicate = 0;									//DUPLICATE ELEMENT IN FATHER AND MOTHER
	int son_index = 0;											//INDEX OF SON'S ELEMENTS

	for (int i = 0; i < rand_cut; i++)							//ADD FATHER ELEMENTS
	{
		new_SON[son_index++] = tspFather[i];						//CHOOSE FATHER NODE
		son_flags[tspFather[i]] = 1;
		
	}
	for (int m = rand_cut; m < inst->nnodes; m++)				//ADD MOTHER ELEMENTS
	{
		if (son_flags[tspMother[m]] != 1)						//DON'T TAKE ELEMENTS ALREADY SELECTED FROM FATHER
		{
			new_SON[son_index++] = tspMother[m];					//CHOOSE MOTHER NODE
			son_flags[tspMother[m]] = 1;
		}
	}
	int last_son_index = son_index;

	/*----------------ADD MISSING NODES(IF THERE WERE DUPLICATES)-------------*/
	for (int l = 0; l < inst->nnodes; l++)
	{
		if (son_flags[l] == 0)
		{
			new_SON[son_index++] = l;
		}
	}
	if (GENETIC_ALG > 400)
	{
		printf("Nodes of the son are:\n");
		for (int i = 0; i < inst->nnodes; i++)
		{
			printf("%d ", new_SON[i] + 1);
		}
		printf("\n");
	}
	free(son_flags);
	return new_SON;
	free(new_SON);
}

/*MUTATION TO MODIFY ONLY SOME CRHOMOSOMES OF AN INDIVIDUAL*/
int* mutation(instance* inst, int *tspParent, int* son)
{
	int* tsp_mutation_son = (int*)calloc(inst->nnodes, sizeof(int));
	for (int l = 0; l < inst->nnodes; l++)
	{
		tsp_mutation_son[l] = tspParent[l];
	}
	int index_chromosome1 = rand() % inst->nnodes;
	int index_chromosome2 = rand() % inst->nnodes;

	while (index_chromosome2 == index_chromosome1)
	{
		index_chromosome2 = rand() % inst->nnodes;
	}

	int chromosome1 = tsp_mutation_son[index_chromosome1];
	int chromosome2 = tsp_mutation_son[index_chromosome2];
	
	tsp_mutation_son[index_chromosome2] = chromosome1;
	tsp_mutation_son[index_chromosome1] = chromosome2;

	return tsp_mutation_son;
	free(tsp_mutation_son);
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

