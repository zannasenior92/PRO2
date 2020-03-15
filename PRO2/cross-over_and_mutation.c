#include "TSP.h"
#include <Windows.h>
#include <time.h>

/*-------------------CROSS-OVER TO MERGE TWO TSPs----------------------*/
void cross_over(instance *inst, int* tsps, int father, int mother, int num_pop_tsp)
{
	int rand_cut = rand() % inst->nnodes;						//CUT WHERE TO CREATE SON WITH SOME FATHER'S AND MOTHER'S CHROMOSOMES
	int* son_flags = (int*)calloc(inst->nnodes, sizeof(int));	//FLAG WHO REPRESENT HOW NODE IS INSERTED
	
	if (GENETIC_ALG > 400)
	{
		printf("\n");
		printf("Random cut in position: %d \n", rand_cut);
		printf("Nodes of the father are: ");
		for (int i = 0; i < inst->nnodes; i++)
		{
			printf("%d ", tsps[father*inst->nnodes + i] + 1);
		}
		printf("\n\n");
		printf("Nodes of the mother are: ");
		for (int i = 0; i < inst->nnodes; i++)
		{
			printf("%d ", tsps[mother*inst->nnodes + i] + 1);
		}
		printf("\n\n");
	}
	int count_duplicate = 0;									//DUPLICATE ELEMENT IN FATHER AND MOTHER
	int son_index = 0;											//INDEX OF SON'S ELEMENTS

	for (int i = 0; i < rand_cut; i++)							//ADD FATHER ELEMENTS
	{
		inst->new_SON[son_index++] = tsps[father*inst->nnodes + i];						//CHOOSE FATHER NODE
		son_flags[tsps[father*inst->nnodes + i]] = 1;

	}
	for (int m = rand_cut; m < inst->nnodes; m++)				//ADD MOTHER ELEMENTS
	{
		if (son_flags[tsps[mother*inst->nnodes + m]] != 1)						//DON'T TAKE ELEMENTS ALREADY SELECTED FROM FATHER
		{
			inst->new_SON[son_index++] = tsps[mother*inst->nnodes + m];					//CHOOSE MOTHER NODE
			son_flags[tsps[mother*inst->nnodes + m]] = 1;
		}
	}
	int last_son_index = son_index;

	/*----------------ADD MISSING NODES(IF THERE WERE DUPLICATES)-------------*/
	for (int l = 0; l < inst->nnodes; l++)
	{
		if (son_flags[l] == 0)
		{
			inst->new_SON[son_index++] = l;
		}
	}
	if (GENETIC_ALG > 400)
	{
		printf("Nodes of the son are:    ");
		for (int i = 0; i < inst->nnodes; i++)
		{
			printf("%d ", inst->new_SON[i] + 1);
		}
		printf("\n\n");
	}
	free(son_flags);
	//return inst->new_SON;
}

/*MUTATION TO MODIFY ONLY SOME CRHOMOSOMES OF AN INDIVIDUAL*/
void mutation(instance* inst, int *tsps, int tspParent, int num_pop_tsp)
{
	for (int l = 0; l < inst->nnodes; l++)
	{
		inst->new_SON[l] = tsps[tspParent*inst->nnodes+l];
	}

	if (GENETIC_ALG > 400)
	{
		printf("\n");
		printf("TSP BEFORE Mutation: ");
		for (int k = 0; k < inst->nnodes; k++)
		{
			printf("%d ", inst->new_SON[k] + 1);
		}
		printf("\n\n");
	}

	int index_chromosome1 = rand() % inst->nnodes;
	int index_chromosome2 = rand() % inst->nnodes;
	int index_chromosome3 = rand() % inst->nnodes;
	int index_chromosome4 = rand() % inst->nnodes;
	while (index_chromosome2 == index_chromosome1)
	{
		index_chromosome2 = rand() % inst->nnodes;
	}
	while ((index_chromosome3 == index_chromosome1) || (index_chromosome3 == index_chromosome2) )
	{
		index_chromosome3 = rand() % inst->nnodes;
	}
	while ((index_chromosome4 == index_chromosome1) || (index_chromosome4 == index_chromosome2) || (index_chromosome4 == index_chromosome3))
	{
		index_chromosome4 = rand() % inst->nnodes;
	}

	int chromosome1 = inst->new_SON[index_chromosome1];
	int chromosome2 = inst->new_SON[index_chromosome2];
	int chromosome3 = inst->new_SON[index_chromosome3];
	int chromosome4 = inst->new_SON[index_chromosome4];

	inst->new_SON[index_chromosome2] = chromosome1;
	inst->new_SON[index_chromosome1] = chromosome2;
	inst->new_SON[index_chromosome3] = chromosome3;
	inst->new_SON[index_chromosome4] = chromosome4;

	if (GENETIC_ALG > 400)
	{
		printf("\n");
		printf("TSP AFTER Mutation: ");
		for (int k = 0; k < inst->nnodes; k++)
		{
			printf("%d ", inst->new_SON[k] + 1);
		}
		printf("\n\n");
	}

	//return inst->new_SON;
}