
/*------------------------GENETIC ALGORITHM-----------------------*/

#include "TSP.h"
#include <Windows.h>
#include <time.h>

#define CHOICE_OF_ADD_BAD 0.5		//REPRESENT THE PERCENTAGE FOR WHICH I CAN INSERT A BAD TSP IN A POPULATION
#define CHOICE_OF_POPULATION 0.2	//REPRESENT THE PERCENTAGE FOR WHICH I SELECT MUTATION OR CROSS-OVER
#define VARIANCE_CHOICE_OF_POP 0.2	//REPRESENT THE DIMENSION OF RANGE FOR SELECT A FATHER AS SON IN NEXT POPULATION
#define POPULATION_SIZE 50			//REPRESENT THE DIMENSION OF THE POPULATION CREATED EACH TIME7
#define NUMBER_OF_POPULATIONS 200	//REPRESENT THE NUMBER OF POPULATIONS CREATED


/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
void reverse_xpos(int x, instance* inst, int* nodes);
void update_choosen_edge(instance* inst);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);
double nearest_neighborhood(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node);
double nearest_neighborhood_GRASP(instance *inst, CPXENVptr env, CPXLPptr lp, int start_node, int seed);
int update_index_worst_cost_tsp(instance *inst, double *opt_cost, int num_sel_tsp);
double cost_tsp(instance *inst, int* tsp);
int* cross_over(instance *inst, int *tsps, int father, int mother, int num_pop_tsp);
int* mutation(instance* inst, int *tsps, int tspParent, int num_pop_tsp);
double update_worst_cost_population(instance *inst, double *opt_cost, int num_sel_tsp);
int index_best_cost_tsp(instance *inst, double *tsp_fitness, int num_sel_tsp);
void update_bestsol(instance *inst, int *tsp_opt, int index_best_fitness, int num_pop_tsp);


void genetic_alg(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	srand((unsigned int)time(NULL));							//NEW RANDOM NUMBERS

	int max_num_pop = NUMBER_OF_POPULATIONS;	//LIMIT THE NUMBER OF CREATED POPULATION, END AFTER ANALYZE THIS NUMBER OF POPULATIONS
	int num_pop_tsp = POPULATION_SIZE;			//NUMBER OF TSPs OF THE POPULATION(CANNOT BE LESS THAN nnodes!)

	//MATRIX OF SOLUTIONS OF NEIGHBORHOODS TO CREATE SONS (EVERY ROW RAPPRESENT THE SOLUTION'S SET OF NODES)
	int *TSP_solutions = (int*) malloc( sizeof(int)*num_pop_tsp*inst->nnodes);
	

	double *TSP_fitness = (double *)malloc(num_pop_tsp * sizeof(double));		//NEIGHBORHOOD'S FITNESSES(COSTS) ARRAY (CONTAIN TSP COSTS OF TSP FINDED WITH HEURISTIC NEAREST NEIGHBORHOOD)
	
	int		index_best_fitness;									//FITNESS BEST SOLUTION INDEX
	double	best_fitness = 0;									//BEST SOLUTION COST
	double	worst_fitness = 0;									//FITNESS(COST) WORST FATHER
	int		index_worst_tsp_parent = 0;							//WORST FATHER INDEX
	int		num_of_populations = 0;								//COUNTER REGARDING NUMBER OF POPULATION THAT I'M ANALIZING
	int		done = 1;

	for (int i = 0; i < num_pop_tsp; i++)//COMPUTE INITIAL POPULATION WITH NEAREST NEIGHBORHOOD
	{

		for (int h = 0; h < inst->ncols; h++)//RESET BEST SOLUTION
		{inst->best_sol[h] = 0.0;}

		TSP_fitness[i] = nearest_neighborhood_GRASP(inst, env, lp, 1, i);	//TSP COST
		for (int n = 0; n < inst->nnodes; n++)//COPY SOLUTION OF GRASP INTO A MATRIX THAT CONTAIN ALL TSP'S SOLUTIONS
		{
			TSP_solutions[i*inst->nnodes+n] = inst->choosen_nodes[n];
		}

		if (GENETIC_ALG > 400) {
			printf("Initial fitness Tsp%d : %lf \n ", i, TSP_fitness[i]);
		}

		if (TSP_fitness[i] > worst_fitness)						//FIND WORST PARENT'S COST
		{
			index_worst_tsp_parent = i;
			worst_fitness = TSP_fitness[i];
		}
	}

	if (GENETIC_ALG > 500)
	{
		for (int k = 0; k < num_pop_tsp; k++)
		{
			printf("TSP%d : ", k);
			for (int m = 0; m < inst->nnodes; m++)
			{
				printf("%d ", TSP_solutions[k*inst->nnodes+m]);
			}
			printf("\n\n");
		}
	}//PRINT INITIAL TSPs
	

	index_best_fitness = index_best_cost_tsp(inst, TSP_fitness, num_pop_tsp);
	best_fitness = TSP_fitness[index_best_fitness];
	update_bestsol(inst, TSP_solutions,index_best_fitness, num_pop_tsp);
	printf("Start Best Fitness is TSP %d: %lf \n",index_best_fitness ,best_fitness);
	printf("\n");
	/*---------------------------------------------------------------------------------*/

	/*------------------------------NEXT POPULATIONS-----------------------------------*/
	int *newTSP_solutions = (int*)calloc(num_pop_tsp*inst->nnodes, sizeof(int));
	
	//int **newTSP_solutions = (int **)malloc(num_pop_tsp * inst->nnodes * sizeof(int));	//NEW POPULATION (NEW POPULATION FORMED BY SUBSTITUTING BETTER TSPs CREATED WITH CROSSOVER WITH OLD TSPs OF PREVIOUS POPULATION)
	double *newTSP_fitness = (double *)malloc(num_pop_tsp * sizeof(double));	//NEW SOLUTIONS COST
	inst->new_SON = (int*)calloc(inst->nnodes, sizeof(int));					//NEW SON GENERATED

	double	choice_of_new_gen;									//RANDOM CHOICE FOR CREATE NEW POPULATION
	double	choice_add_bad;										//RANDOM CHOICE FOR ADD IN A NEW POPULATION A BAD MEMBER
	int		subs_index_choice;									//RANDOM CHOICE FOR SELECT A POPULATION ELEMENT TO SUBSITUTE WITH A BAD TSP

	for (int i = 0; i < num_pop_tsp; i++)
	{
		for (int j = 0; j < inst->nnodes; j++)
		{
			newTSP_solutions[i*inst->nnodes+j] = TSP_solutions[i*inst->nnodes+j];
		}
		newTSP_fitness[i] = TSP_fitness[i];
	}
	

	if (GENETIC_ALG > 500)
	{
		for (int k = 0; k < num_pop_tsp; k++)
		{
			printf("newTSP%d : ", k);
			for (int m = 0; m < inst->nnodes; m++)
			{
				printf("%d ", TSP_solutions[k*inst->nnodes+m]);
			}
			printf("\n\n");
		}
	}//PRINT NEW TSPs

	/*-----------------------------GENERO LE POPOLAZIONI-------------------------------*/

	while (num_of_populations < max_num_pop)
	{
		int* current_son1 = (int*)malloc(inst->nnodes * sizeof(int));	//SON GENERATED FROM PARENTS WITH CROSS-OVER OR MUTATION
		int* current_son2 = (int*)malloc(inst->nnodes * sizeof(int));

		int *cross_over_solutions = (int*)malloc(sizeof(int)*num_pop_tsp*inst->nnodes);
		//int	**cross_over_solutions = (int **)malloc(num_pop_tsp * inst->nnodes * sizeof(int));
		double *cross_over_fitness = (double *)malloc(num_pop_tsp * sizeof(double));

		if (GENETIC_ALG > 400) { printf("\n Worst cost: %lf \n", worst_fitness); }

		choice_add_bad = (double)rand() / (double)RAND_MAX;				//GENERATE A RANDOM CHOICE FOR ADD A BAD MEMBER TO A NEW POPULATION

		
		double	current_cost1;											//COST OF SON
		double	current_cost2;
		int		new_tsp_index = 0;
		int		flag_DUPLICATE1;										//FLAG FOR CHECK IF THERE IS A DUPLICATE IN THE POPULATION
		int		flag_DUPLICATE2;
		

		for (int i = 0; i < num_pop_tsp; i++)
		{
			choice_of_new_gen = (double)rand() / (double)RAND_MAX;	//GENERATE A RANDOM CHOICE
			subs_index_choice = rand() % inst->nnodes;				//RANDOM INDEX FOR SUBSTITUTE A TSP WITH A BAD TSP

			if (choice_of_new_gen < CHOICE_OF_POPULATION)//---------------CHOOSE MUTATION
			{
				mutation(inst, TSP_solutions, i, num_pop_tsp);
				for (int ind = 0; ind < inst->nnodes; ind++) {
					current_son1[ind] = inst->new_SON[ind];
				}
				current_cost1 = cost_tsp(inst, current_son1);
				
					//-------------------------------SUBSTITUTE THE NEW BETTER TSP SON WITH A PARENT
					if (current_cost1 < worst_fitness)
					{
						for (int k = 0; k < inst->nnodes; k++)
						{
							newTSP_solutions[i*inst->nnodes + k] = current_son1[k];
						}
						newTSP_fitness[i] = current_cost1;
						index_worst_tsp_parent = update_index_worst_cost_tsp(inst, newTSP_fitness, num_pop_tsp);	//UPDATE WORST TSPs INDEX
						worst_fitness = update_worst_cost_population(inst, newTSP_fitness, num_pop_tsp);			//UPDATE WORST COST
					}
					else
					{
						if (choice_add_bad < CHOICE_OF_ADD_BAD)
						{
							for (int g = 0; g < inst->nnodes; g++)
							{
								newTSP_solutions[i*inst->nnodes + g] = current_son1[g];
							}
							newTSP_fitness[i] = current_cost1;
							index_worst_tsp_parent = update_index_worst_cost_tsp(inst, newTSP_fitness, num_pop_tsp);	//UPDATE WORST TSPs INDEX
							worst_fitness = update_worst_cost_population(inst, newTSP_fitness, num_pop_tsp);			//UPDATE WORST COST
						}
						else
						{
							for (int g = 0; g < inst->nnodes; g++)
							{
								newTSP_solutions[i*inst->nnodes + g] = TSP_solutions[i*inst->nnodes+g];
							}
							newTSP_fitness[i] = TSP_fitness[i];
							index_worst_tsp_parent = update_index_worst_cost_tsp(inst, newTSP_fitness, num_pop_tsp);	//UPDATE WORST TSPs INDEX
							worst_fitness = update_worst_cost_population(inst, newTSP_fitness, num_pop_tsp);			//UPDATE WORST COST 
						}
					}
				
			}
			else if (choice_of_new_gen > CHOICE_OF_POPULATION && choice_of_new_gen < (CHOICE_OF_POPULATION + VARIANCE_CHOICE_OF_POP))
			{
				for (int g = 0; g < inst->nnodes; g++)
				{
					newTSP_solutions[i*inst->nnodes + g] = TSP_solutions[i*inst->nnodes + g];
				}
				newTSP_fitness[i] = TSP_fitness[i];
				index_worst_tsp_parent = update_index_worst_cost_tsp(inst, newTSP_fitness, num_pop_tsp);	//UPDATE WORST TSPs INDEX
				worst_fitness = update_worst_cost_population(inst, newTSP_fitness, num_pop_tsp);			//UPDATE WORST COST 
			}
			else//-----------------------------------------------------CHOOSE CROSS-OVER
			{
				int counter_DUPLICATES = 0; //COUNTER FOR VERIFY IF ALL ELEMENT ARE DUPLICATE
				double worst_cross_over;
				int index_worst_co;
				int index_best_co = 0;
				int cross_over_index = 0;
				
				for (int j = i + 1; j < num_pop_tsp; j++)
				{
					flag_DUPLICATE1 = 0;
					flag_DUPLICATE2 = 0;
					subs_index_choice = rand() % inst->nnodes;//RANDOM INDEX FOR SUBSTITUTE A TSP WITH A BAD TSP


					cross_over(inst, TSP_solutions, i, j, num_pop_tsp);	//NODES CURRENT SON
					for (int ind = 0; ind < inst->nnodes; ind++) {
						current_son1[ind] = inst->new_SON[ind];
					}
					cross_over(inst, TSP_solutions, j, i, num_pop_tsp);
					for (int ind = 0; ind < inst->nnodes; ind++) {
						current_son2[ind] = inst->new_SON[ind];
					}
					
					current_cost1 = cost_tsp(inst, current_son1);									//COST CURRENT SON
					current_cost2 = cost_tsp(inst, current_son2);

					if (GENETIC_ALG > 400)
					{
						printf("Current1 son's Cost %d: %lf \n", i + j, current_cost1);
						printf("Current2 son's Cost %d: %lf \n", i + j, current_cost2);
					}

					if (cross_over_index < num_pop_tsp)
					{
						for (int g = 0; g < inst->nnodes; g++)
						{
							cross_over_solutions[cross_over_index*inst->nnodes + g] = current_son1[g];
						}
						cross_over_fitness[cross_over_index] = current_cost1;
						cross_over_index++;
						index_worst_co = update_index_worst_cost_tsp(inst, cross_over_fitness, num_pop_tsp);
						worst_cross_over = update_worst_cost_population(inst, cross_over_fitness, num_pop_tsp);
					}
					if (cross_over_index < num_pop_tsp)
					{
						for (int g = 0; g < inst->nnodes; g++)
						{
							cross_over_solutions[cross_over_index*inst->nnodes + g] = current_son2[g];
						}
						cross_over_fitness[cross_over_index] = current_cost2;
						cross_over_index++;
						index_worst_co = update_index_worst_cost_tsp(inst, cross_over_fitness, num_pop_tsp);
						worst_cross_over = update_worst_cost_population(inst, cross_over_fitness, num_pop_tsp);
					}
					else
					{
						if (current_cost1 < worst_fitness)
						{
							for (int g = 0; g < inst->nnodes; g++)
							{
								cross_over_solutions[index_worst_co*inst->nnodes + g] = current_son1[g];
							}
							cross_over_fitness[index_worst_co] = current_cost1;
							index_worst_co = update_index_worst_cost_tsp(inst, cross_over_fitness, num_pop_tsp);
							worst_cross_over = update_worst_cost_population(inst, cross_over_fitness, num_pop_tsp);
						}
						if (current_cost2 < worst_fitness)
						{
							for (int g = 0; g < inst->nnodes; g++)
							{
								cross_over_solutions[index_worst_co*inst->nnodes + g] = current_son2[g];
							}
							cross_over_fitness[index_worst_co] = current_cost2;
							index_worst_co = update_index_worst_cost_tsp(inst, cross_over_fitness, num_pop_tsp);
							worst_cross_over = update_worst_cost_population(inst, cross_over_fitness, num_pop_tsp);
						}
					}
					index_best_co = index_best_cost_tsp(inst, cross_over_fitness, num_pop_tsp);
				}
				if (i== num_pop_tsp -1)
				{
					for (int g = 0; g < inst->nnodes; g++)
					{
						newTSP_solutions[i*inst->nnodes + g] = TSP_solutions[i*inst->nnodes + g];
					}
					newTSP_fitness[i] = TSP_fitness[i];
					index_worst_tsp_parent = update_index_worst_cost_tsp(inst, newTSP_fitness, num_pop_tsp);	//UPDATE WORST TSPs INDEX
					worst_fitness = update_worst_cost_population(inst, newTSP_fitness, num_pop_tsp);			//UPDATE WORST COST 
				}
				else
				{
					for (int g = 0; g < inst->nnodes; g++)
					{
						newTSP_solutions[i*inst->nnodes + g] = cross_over_solutions[index_best_co*inst->nnodes + g];
					}
					newTSP_fitness[i] = cross_over_fitness[index_best_co];
					index_worst_tsp_parent = update_index_worst_cost_tsp(inst, newTSP_fitness, num_pop_tsp);	//UPDATE WORST TSPs INDEX
					worst_fitness = update_worst_cost_population(inst, newTSP_fitness, num_pop_tsp);			//UPDATE WORST COST 
				}

				
			}
			
		}
		


		if (GENETIC_ALG > 400)
		{
			printf("\n");
			printf("--New generation--\n\n");
			for (int u = 0; u < num_pop_tsp; u++)
			{
				printf("NewTsp%d Solution: ", u);
				for (int t = 0; t < inst->nnodes; t++)
				{
					printf("%d ", newTSP_solutions[u*inst->nnodes + t] + 1);
				}
				printf("\n");
			}
		}

		for (int i = 0; i < num_pop_tsp; i++)//UPDATE POPULATION AND POPULATION COST
		{
			for (int g = 0; g < inst->nnodes; g++)
			{
				TSP_solutions[i*inst->nnodes + g] = newTSP_solutions[i*inst->nnodes + g];
			}
			TSP_fitness[i] = newTSP_fitness[i];
		}

		if (GENETIC_ALG > 200)
		{
			printf("New TSPs fitness: \n");

			for (int i = 0; i < num_pop_tsp; i++)
			{
				printf("Tsp%d fitness: %lf \n", i, TSP_fitness[i]);
			}
		}
		printf("\n");
		index_best_fitness = index_best_cost_tsp(inst, TSP_fitness, num_pop_tsp);
		best_fitness = TSP_fitness[index_best_fitness];
		update_bestsol(inst, TSP_solutions, index_best_fitness, num_pop_tsp);

		printf("Worst fitness index: %d \n", index_worst_tsp_parent);
		printf("Worst fitness:       %lf \n", worst_fitness);
		printf("Best fitness index:  %d \n", index_best_fitness);
		printf("Best Fitness :       %lf \n", best_fitness);
		printf("\n");
		num_of_populations++;

		//Sleep(5000);
		
		free(cross_over_solutions);
		free(cross_over_fitness);
		free(current_son1);

		free(current_son2);
	}

	free(newTSP_fitness);
	free(TSP_solutions);
	free(TSP_fitness);

	free(newTSP_solutions);

	

	
}
/*******************************************************************************************************************/
/*******************************************************************************************************************/
/*******************************************************************************************************************/