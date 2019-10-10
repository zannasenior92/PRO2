#include "TSP.h"
#include <Windows.h>
#include <time.h>

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
		for (int i = 0; i < inst->nnodes; i++)
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