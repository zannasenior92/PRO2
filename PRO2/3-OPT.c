
/*-------------------------------------------3-OPT FUNCTION-------------------------------------------------*/

#include "TSP.h"
/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void reverse_xpos(int x, instance* inst, int* nodes);
void update_choosen_edge(instance* inst);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(instance *inst);
void reset_old_edges3(instance *inst, int old1, int old2, int old3, int new1, int new2, int new3);
void set_new_edges3(instance *inst, int old1, int old2, int old3, int new1, int new2, int new3);


double three_opt(instance *inst)
{
	//EDGES OF THE OPTIMAL SOLUTION STORED INTO AN ARRAY
	int* edges = (int*)calloc(inst->nnodes, sizeof(int));
	int n = 0;
	for (int i = 0; i < inst->ncols; i++) {
		if (inst->best_sol[i] > TOLERANCE) {
			edges[n] = i;
			if (THREE_OPT > 500)
			{
				printf("edge[%d]=%d\n", n, i);
			}
			n++;
		}
	}

	double min_delta = 0;
	double edge1_length, edge2_length, edge3_length;

	double new_distA1, new_distA2, new_distA3;
	double new_distB1, new_distB2, new_distB3;
	double new_distC1, new_distC2, new_distC3;
	double new_distD1, new_distD2, new_distD3;

	int curr_new_edge1, curr_new_edge2, curr_new_edge3;
	int min_new_edge1, min_new_edge2, min_new_edge3;
	int old_edge1, old_edge2, old_edge3;
	int *nodes_edge1 = (int*)calloc(2, sizeof(int));
	int *nodes_edge2 = (int*)calloc(2, sizeof(int));
	int *nodes_edge3 = (int*)calloc(2, sizeof(int));
	int done = 0;

	for (int i = 0; i < inst->nnodes; i++)
	{
		reverse_xpos(edges[i], inst, nodes_edge1);						//TAKE FIRST EDGE

		for (int j = i + 1; j < inst->nnodes - 1; j++)
		{
			reverse_xpos(edges[j], inst, nodes_edge2);					//TAKE SECOND EDGE

			for (int k = j + 1; k < inst->nnodes - 2; k++)
			{
				reverse_xpos(edges[k], inst, nodes_edge3);				//TAKE THIRD EDGE

				/*---------------------------CHECK INCOMPATIBILITIES(NO ADJACENT EDGES)------------------------*/
				if ((nodes_edge1[0] == nodes_edge2[0]) || (nodes_edge1[0] == nodes_edge2[1]) ||
					(nodes_edge1[0] == nodes_edge3[0]) || (nodes_edge1[0] == nodes_edge3[1]) ||
					(nodes_edge1[1] == nodes_edge2[0]) || (nodes_edge1[1] == nodes_edge2[1]) ||
					(nodes_edge1[1] == nodes_edge3[0]) || (nodes_edge1[1] == nodes_edge3[1]) ||
					(nodes_edge2[0] == nodes_edge3[0]) || (nodes_edge2[0] == nodes_edge3[1]) ||
					(nodes_edge2[1] == nodes_edge3[0]) || (nodes_edge2[1] == nodes_edge3[1]))
					continue;

				/*----THE OLD EDGES---*/
				old_edge1 = edges[i];
				old_edge2 = edges[j];
				old_edge3 = edges[k];
				/*-------------------------TAKE THE LENGTH OF OLD EDGES-----------------------*/
				edge1_length = dist(nodes_edge1[0], nodes_edge1[1], inst);
				edge2_length = dist(nodes_edge2[0], nodes_edge2[1], inst);
				edge3_length = dist(nodes_edge3[0], nodes_edge3[01], inst);
				/*---------------------------MIN NEW EDGES = OLD EDGES------------------------*/
				min_new_edge1 = xpos(nodes_edge1[0], nodes_edge1[1], inst);
				min_new_edge2 = xpos(nodes_edge2[0], nodes_edge2[1], inst);
				min_new_edge3 = xpos(nodes_edge3[0], nodes_edge3[1], inst);
				/******************************************************************************/

				/*---------1) TAKE LENGTH OF FIRST NEW PRESUMED EGDES-------------------------*/
				new_distA1 = dist(nodes_edge1[0], nodes_edge2[1], inst);
				new_distA2 = dist(nodes_edge2[0], nodes_edge3[1], inst);
				new_distA3 = dist(nodes_edge3[0], nodes_edge1[1], inst);
				//---------SEE THE DIFFERENCE
				double deltaA = new_distA1 + new_distA2 + new_distA3 - edge1_length - edge2_length - edge3_length;
				//---------TRY TO SELECT THESE EDGES	
				curr_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[1], inst);
				curr_new_edge2 = xpos(nodes_edge2[0], nodes_edge3[1], inst);
				curr_new_edge3 = xpos(nodes_edge3[0], nodes_edge1[1], inst);
				/*-----------------UPDATE SELECTION----------------------*/
				set_new_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);
				/*-----CHECK IF AREN'T PRESENT CONNECTED COMPONENTS------*/
				if (kruskal_sst(inst) == 1)
				{
					if (deltaA < min_delta)
					{
						min_delta = deltaA;
						/*----------------CHOOSE THE THREE TWO EDGES---------------*/
						min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[1], inst);
						min_new_edge2 = xpos(nodes_edge2[0], nodes_edge3[1], inst);
						min_new_edge3 = xpos(nodes_edge3[0], nodes_edge1[1], inst);
					}
				}
				/*-----------------------RESET EDGES IN BEST SOL-----------*/
				reset_old_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);

				/******************************************************************************/

				/*---------2) TAKE LENGTH OF SECOND NEW PRESUMED EGDES------------------------*/
				new_distB1 = dist(nodes_edge1[0], nodes_edge2[0], inst);
				new_distB2 = dist(nodes_edge1[1], nodes_edge3[0], inst);
				new_distB3 = dist(nodes_edge3[1], nodes_edge2[1], inst);
				//---------SEE THE DIFFERENCE
				double deltaB = new_distB1 + new_distB2 + new_distB3 - edge1_length - edge2_length - edge3_length;
				//---------TRY TO SELECT THESE EDGES	
				curr_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[0], inst);
				curr_new_edge2 = xpos(nodes_edge1[1], nodes_edge3[0], inst);
				curr_new_edge3 = xpos(nodes_edge3[1], nodes_edge2[1], inst);
				/*-----------------UPDATE SELECTION----------------------*/
				set_new_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);
				/*-----CHECK IF AREN'T PRESENT CONNECTED COMPONENTS------*/
				if (kruskal_sst(inst) == 1)
				{
					if (deltaB < min_delta)
					{
						min_delta = deltaB;
						/*----------------CHOOSE THE THREE TWO EDGES---------------*/
						min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[0], inst);
						min_new_edge2 = xpos(nodes_edge1[1], nodes_edge3[0], inst);
						min_new_edge3 = xpos(nodes_edge3[1], nodes_edge2[1], inst);
					}
				}
				/*-----------------------RESET EDGES IN BEST SOL-----------*/
				reset_old_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);

				/******************************************************************************/

				/*---------3) TAKE LENGTH OF THIRD NEW PRESUMED EGDES-------------------------*/
				new_distC1 = dist(nodes_edge1[0], nodes_edge2[1], inst);
				new_distC2 = dist(nodes_edge1[1], nodes_edge3[1], inst);
				new_distC3 = dist(nodes_edge3[0], nodes_edge2[0], inst);
				//---------SEE THE DIFFERENCE
				double deltaC = new_distC1 + new_distC2 + new_distC3 - edge1_length - edge2_length - edge3_length;
				//---------TRY TO SELECT THESE EDGES	
				curr_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[1], inst);
				curr_new_edge2 = xpos(nodes_edge1[1], nodes_edge3[1], inst);
				curr_new_edge3 = xpos(nodes_edge3[0], nodes_edge2[0], inst);
				/*-----------------UPDATE SELECTION----------------------*/
				set_new_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);
				/*-----CHECK IF AREN'T PRESENT CONNECTED COMPONENTS------*/
				if (kruskal_sst(inst) == 1)
				{
					if (deltaC < min_delta)
					{
						min_delta = deltaC;
						/*----------------CHOOSE THE THREE TWO EDGES---------------*/
						min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[1], inst);
						min_new_edge2 = xpos(nodes_edge1[1], nodes_edge3[1], inst);
						min_new_edge3 = xpos(nodes_edge3[0], nodes_edge2[0], inst);
					}
				}
				/*-----------------------RESET EDGES IN BEST SOL-----------*/
				reset_old_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);

				/******************************************************************************/

				/*---------4) TAKE LENGTH OF FOURTH NEW PRESUMED EGDES------------------------*/
				new_distD1 = dist(nodes_edge1[1], nodes_edge2[1], inst);
				new_distD2 = dist(nodes_edge1[0], nodes_edge3[0], inst);
				new_distD3 = dist(nodes_edge3[1], nodes_edge2[0], inst);
				//---------SEE THE DIFFERENCE
				double deltaD = new_distD1 + new_distD2 + new_distD3 - edge1_length - edge2_length - edge3_length;
				//---------TRY TO SELECT THESE EDGES	
				curr_new_edge1 = xpos(nodes_edge1[1], nodes_edge2[1], inst);
				curr_new_edge2 = xpos(nodes_edge1[0], nodes_edge3[0], inst);
				curr_new_edge3 = xpos(nodes_edge3[1], nodes_edge2[0], inst);
				/*-----------------UPDATE SELECTION----------------------*/
				set_new_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);
				/*-----CHECK IF AREN'T PRESENT CONNECTED COMPONENTS------*/
				if (kruskal_sst(inst) == 1)
				{
					if (deltaD < min_delta)
					{
						min_delta = deltaD;
						/*----------------CHOOSE THE THREE TWO EDGES---------------*/
						min_new_edge1 = xpos(nodes_edge1[1], nodes_edge2[1], inst);
						min_new_edge2 = xpos(nodes_edge1[0], nodes_edge3[0], inst);
						min_new_edge3 = xpos(nodes_edge3[1], nodes_edge2[0], inst);
					}
				}
				/*-----------------------RESET EDGES IN BEST SOL-----------*/
				reset_old_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);

				/******************************************************************************/

				/*---------5) TAKE LENGTH OF FIFTH NEW PRESUMED EGDES-------------------------*/
				new_distD1 = dist(nodes_edge1[1], nodes_edge2[1], inst);
				new_distD2 = dist(nodes_edge2[0], nodes_edge3[0], inst);
				new_distD3 = dist(nodes_edge1[0], nodes_edge3[1], inst);
				//---------SEE THE DIFFERENCE
				double deltaE = new_distD1 + new_distD2 + new_distD3 - edge1_length - edge2_length - edge3_length;
				//---------TRY TO SELECT THESE EDGES	
				curr_new_edge1 = xpos(nodes_edge1[1], nodes_edge2[1], inst);
				curr_new_edge2 = xpos(nodes_edge2[0], nodes_edge3[0], inst);
				curr_new_edge3 = xpos(nodes_edge1[0], nodes_edge3[1], inst);
				/*-----------------UPDATE SELECTION----------------------*/
				set_new_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);
				/*-----CHECK IF AREN'T PRESENT CONNECTED COMPONENTS------*/
				if (kruskal_sst(inst) == 1)
				{
					if (deltaE < min_delta)
					{
						min_delta = deltaE;
						/*----------------CHOOSE THE THREE TWO EDGES---------------*/
						min_new_edge1 = xpos(nodes_edge1[1], nodes_edge2[1], inst);
						min_new_edge2 = xpos(nodes_edge2[0], nodes_edge3[0], inst);
						min_new_edge3 = xpos(nodes_edge1[0], nodes_edge3[1], inst);
					}
				}
				/*-----------------------RESET EDGES IN BEST SOL-----------*/
				reset_old_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);

				/******************************************************************************/

				/*---------6) TAKE LENGTH OF SIXTH NEW PRESUMED EGDES-------------------------*/
				new_distD1 = dist(nodes_edge1[1], nodes_edge2[0], inst);
				new_distD2 = dist(nodes_edge1[0], nodes_edge3[0], inst);
				new_distD3 = dist(nodes_edge2[1], nodes_edge3[1], inst);
				//---------SEE THE DIFFERENCE
				double deltaF = new_distD1 + new_distD2 + new_distD3 - edge1_length - edge2_length - edge3_length;
				//---------TRY TO SELECT THESE EDGES	
				curr_new_edge1 = xpos(nodes_edge1[1], nodes_edge2[0], inst);
				curr_new_edge2 = xpos(nodes_edge1[0], nodes_edge3[0], inst);
				curr_new_edge3 = xpos(nodes_edge2[1], nodes_edge3[1], inst);
				/*-----------------UPDATE SELECTION----------------------*/
				set_new_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);
				/*-----CHECK IF AREN'T PRESENT CONNECTED COMPONENTS------*/
				if (kruskal_sst(inst) == 1)
				{
					if (deltaF < min_delta)
					{
						min_delta = deltaF;
						/*----------------CHOOSE THE THREE TWO EDGES---------------*/
						min_new_edge1 = xpos(nodes_edge1[1], nodes_edge2[0], inst);
						min_new_edge2 = xpos(nodes_edge1[0], nodes_edge3[0], inst);
						min_new_edge3 = xpos(nodes_edge2[1], nodes_edge3[1], inst);
					}
				}
				/*-----------------------RESET EDGES IN BEST SOL-----------*/
				reset_old_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);

				/******************************************************************************/

				/*---------7) TAKE LENGTH OF SEVENTH NEW PRESUMED EGDES-----------------------*/
				new_distD1 = dist(nodes_edge1[1], nodes_edge2[0], inst);
				new_distD2 = dist(nodes_edge1[0], nodes_edge3[1], inst);
				new_distD3 = dist(nodes_edge2[1], nodes_edge3[0], inst);
				//---------SEE THE DIFFERENCE
				double deltaG = new_distD1 + new_distD2 + new_distD3 - edge1_length - edge2_length - edge3_length;
				//---------TRY TO SELECT THESE EDGES	
				curr_new_edge1 = xpos(nodes_edge1[1], nodes_edge2[0], inst);
				curr_new_edge2 = xpos(nodes_edge1[0], nodes_edge3[1], inst);
				curr_new_edge3 = xpos(nodes_edge2[1], nodes_edge3[0], inst);
				/*-----------------UPDATE SELECTION----------------------*/
				set_new_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);
				/*-----CHECK IF AREN'T PRESENT CONNECTED COMPONENTS------*/
				if (kruskal_sst(inst) == 1)
				{
					if (deltaG < min_delta)
					{
						min_delta = deltaG;
						/*----------------CHOOSE THE THREE TWO EDGES---------------*/
						min_new_edge1 = xpos(nodes_edge1[1], nodes_edge2[0], inst);
						min_new_edge2 = xpos(nodes_edge1[0], nodes_edge3[1], inst);
						min_new_edge3 = xpos(nodes_edge2[1], nodes_edge3[0], inst);
					}
				}
				/*-----------------------RESET EDGES IN BEST SOL-----------*/
				reset_old_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);

				/******************************************************************************/

				/*---------8) TAKE LENGTH OF EIGHTH NEW PRESUMED EGDES------------------------*/
				new_distD1 = dist(nodes_edge1[1], nodes_edge3[1], inst);
				new_distD2 = dist(nodes_edge1[0], nodes_edge2[0], inst);
				new_distD3 = dist(nodes_edge2[1], nodes_edge3[0], inst);
				//---------SEE THE DIFFERENCE
				double deltaH = new_distD1 + new_distD2 + new_distD3 - edge1_length - edge2_length - edge3_length;
				//---------TRY TO SELECT THESE EDGES	
				curr_new_edge1 = xpos(nodes_edge1[1], nodes_edge3[1], inst);
				curr_new_edge2 = xpos(nodes_edge1[0], nodes_edge2[0], inst);
				curr_new_edge3 = xpos(nodes_edge2[1], nodes_edge3[0], inst);
				/*-----------------UPDATE SELECTION----------------------*/
				set_new_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);
				/*-----CHECK IF AREN'T PRESENT CONNECTED COMPONENTS------*/
				if (kruskal_sst(inst) == 1)
				{
					if (deltaH < min_delta)
					{
						min_delta = deltaH;
						/*----------------CHOOSE THE THREE TWO EDGES---------------*/
						min_new_edge1 = xpos(nodes_edge1[1], nodes_edge3[1], inst);
						min_new_edge2 = xpos(nodes_edge1[0], nodes_edge2[0], inst);
						min_new_edge3 = xpos(nodes_edge2[1], nodes_edge3[0], inst);
					}
				}
				/*-----------------------RESET EDGES IN BEST SOL-----------*/
				reset_old_edges3(inst, old_edge1, old_edge2, old_edge3, curr_new_edge1, curr_new_edge2, curr_new_edge3);


				/******************************************************************************/
				/******************************************************************************/


				if ((inst->best_sol[min_new_edge1] == 1) || (inst->best_sol[min_new_edge2] == 1)
					|| (inst->best_sol[min_new_edge3] == 1)) {
					continue;
				}

				if (THREE_OPT > 400)
				{
					printf("best_sol[%d]=%f\n", old_edge1, inst->best_sol[old_edge1]);
					printf("best_sol[%d]=%f\n", old_edge2, inst->best_sol[old_edge2]);
					printf("best_sol[%d]=%f\n", old_edge3, inst->best_sol[old_edge3]);

					printf("best_sol[%d]=%f\n", min_new_edge1, inst->best_sol[min_new_edge1]);
					printf("best_sol[%d]=%f\n", min_new_edge2, inst->best_sol[min_new_edge2]);
					printf("best_sol[%d]=%f\n", min_new_edge3, inst->best_sol[min_new_edge3]);

				}

				/*SE NON HO TROVATO UN MIGLIORAMENTO(O PER CICLI O PER DELTA MAI NEGATIVO) ALLORA
				VENGONO SETTATI A ZERO E POI A 1 GLI ARCHI VECCHI OVVERO NON FACCIO MOSSE POICHE'
				min_new_edge1	min_new_edge2	min_new_edge3 NON SONO CAMBIATI E SONO UGUALI A
				old_edge1 old_edge2 old_edge3*/
				/*-----------------UPDATE SELECTION----------------------*/
				set_new_edges3(inst, old_edge1, old_edge2, old_edge3, min_new_edge1, min_new_edge2, min_new_edge3);

				if (THREE_OPT > 400)
				{
					printf("RESET %d;%d , %d;%d and %d;%d, and add %d;%d , %d;%d e %d;%d \n",
						nodes_edge1[0] + 1, nodes_edge1[1] + 1, nodes_edge2[0] + 1, nodes_edge2[1] + 1,
						nodes_edge3[0] + 1, nodes_edge3[1] + 1,
						nodes_edge1[0] + 1, nodes_edge2[1] + 1, nodes_edge2[0] + 1, nodes_edge3[1] + 1,
						nodes_edge3[0] + 1, nodes_edge1[1] + 1);
					printf("STEP BY %d,%d AND %d TO %d,%d e %d\n",
						old_edge1, old_edge2, old_edge3, min_new_edge1, min_new_edge2, min_new_edge3);
				}

				free(nodes_edge1);
				free(nodes_edge2);
				free(nodes_edge3);
				free(edges);

				if (THREE_OPT > 400 && (min_delta == 0))
				{
					printf("CHANGE NOTHING \n");
				}

				if (min_delta < 0)
				{
					return min_delta;
				}
			}
		}
	}
	free(nodes_edge1);
	free(nodes_edge2);
	free(nodes_edge3);
	free(edges);
	return min_delta;

}
/*-----------------------------------ALGORITHM TO RESET EDGES---------------------------------------*/
void reset_old_edges3(instance *inst, int old1, int old2, int old3, int new1, int new2, int new3)
{
	inst->best_sol[old1] = 1.0;
	inst->best_sol[old2] = 1.0;
	inst->best_sol[old3] = 1.0;

	inst->best_sol[new1] = 0.0;
	inst->best_sol[new2] = 0.0;
	inst->best_sol[new3] = 0.0;
}

/*-----------------------------------ALGORITHM TO SET EDGES-------------------------------------*/
void set_new_edges3(instance *inst, int old1, int old2, int old3, int new1, int new2, int new3)
{
	inst->best_sol[old1] = 0.0;
	inst->best_sol[old2] = 0.0;
	inst->best_sol[old3] = 0.0;

	inst->best_sol[new1] = 1.0;
	inst->best_sol[new2] = 1.0;
	inst->best_sol[new3] = 1.0;
}


double vns(instance *inst, CPXENVptr env, CPXLPptr lp) {
	double edge1_length, edge2_length, edge3_length;
	double new_dist1, new_dist2, new_dist3;
	int new_edge1, new_edge2, new_edge3;
	int old_edge1, old_edge2, old_edge3;

	//EDGES OF THE OPTIMAL SOLUTION STORED INTO AN ARRAY
	int* edges = (int*)calloc(inst->nnodes, sizeof(int));
	int n = 0;
	for (int i = 0; i < inst->ncols; i++) {
		if (inst->best_sol[i] > TOLERANCE) {
			edges[n] = i;
			if (THREE_OPT > 400)
			{
				printf("edge[%d]=%d\n", n, i);
			}
			n++;
		}
	}
	srand(time(NULL));
	int random_edge1 = edges[rand() % inst->nnodes];
	int random_edge2 = edges[rand() % inst->nnodes];
	int random_edge3 = edges[rand() % inst->nnodes];
	int *random_edge1_nodes = (int*)calloc(2, sizeof(int));
	int *random_edge2_nodes = (int*)calloc(2, sizeof(int));
	int *random_edge3_nodes = (int*)calloc(2, sizeof(int));
	reverse_xpos(random_edge1, inst, random_edge1_nodes);
	reverse_xpos(random_edge2, inst, random_edge2_nodes);
	reverse_xpos(random_edge3, inst, random_edge3_nodes);
	if ((random_edge1_nodes[0] == random_edge2_nodes[0]) || (random_edge1_nodes[0] == random_edge2_nodes[1]) ||
		(random_edge1_nodes[0] == random_edge3_nodes[0]) || (random_edge1_nodes[0] == random_edge3_nodes[1]) ||
		(random_edge1_nodes[1] == random_edge2_nodes[0]) || (random_edge1_nodes[1] == random_edge2_nodes[1]) ||
		(random_edge1_nodes[1] == random_edge3_nodes[0]) || (random_edge1_nodes[1] == random_edge3_nodes[1]) ||
		(random_edge2_nodes[0] == random_edge3_nodes[0]) || (random_edge2_nodes[0] == random_edge3_nodes[1]) ||
		(random_edge2_nodes[1] == random_edge3_nodes[0]) || (random_edge2_nodes[1] == random_edge3_nodes[1])) {

		free(random_edge1_nodes);
		free(random_edge2_nodes);
		free(random_edge3_nodes);
		return -1;
	}

	/*-------------------------TAKE THE LENGTH OF OLD EDGES-----------------------*/
	edge1_length = dist(random_edge1_nodes[0], random_edge1_nodes[1], inst);
	edge2_length = dist(random_edge2_nodes[0], random_edge2_nodes[1], inst);
	edge3_length = dist(random_edge3_nodes[0], random_edge3_nodes[1], inst);

	old_edge1 = xpos(random_edge1_nodes[0], random_edge1_nodes[1], inst);
	old_edge2 = xpos(random_edge2_nodes[0], random_edge2_nodes[1], inst);
	old_edge3 = xpos(random_edge3_nodes[0], random_edge3_nodes[1], inst);

	new_dist1 = dist(random_edge1_nodes[0], random_edge2_nodes[1], inst);
	new_dist2 = dist(random_edge2_nodes[0], random_edge3_nodes[1], inst);
	new_dist3 = dist(random_edge3_nodes[0], random_edge1_nodes[1], inst);

	new_edge1 = xpos(random_edge1_nodes[0], random_edge2_nodes[1], inst);
	new_edge2 = xpos(random_edge2_nodes[0], random_edge3_nodes[1], inst);
	new_edge3 = xpos(random_edge3_nodes[0], random_edge1_nodes[1], inst);

	//---------SEE THE DIFFERENCE
	double delta = new_dist1 + new_dist2 + new_dist3 - edge1_length - edge2_length - edge3_length;
	if ((inst->best_sol[new_edge1] == 1) || (inst->best_sol[new_edge2] == 1)
		|| (inst->best_sol[new_edge3] == 1)) {
		free(random_edge1_nodes);
		free(random_edge2_nodes);
		free(random_edge3_nodes);
		return -1;
	}

	if (THREE_OPT > 400)
	{
		printf("best_sol[%d]=%f\n", old_edge1, inst->best_sol[old_edge1]);
		printf("best_sol[%d]=%f\n", old_edge2, inst->best_sol[old_edge2]);
		printf("best_sol[%d]=%f\n", old_edge3, inst->best_sol[old_edge3]);

		printf("best_sol[%d]=%f\n", new_edge1, inst->best_sol[new_edge1]);
		printf("best_sol[%d]=%f\n", new_edge2, inst->best_sol[new_edge2]);
		printf("best_sol[%d]=%f\n", new_edge3, inst->best_sol[new_edge3]);

	}
	/*-----------------UPDATE SELECTION----------------------*/
	inst->best_sol[old_edge1] = 0.0;
	inst->best_sol[old_edge2] = 0.0;
	inst->best_sol[old_edge3] = 0.0;

	inst->best_sol[new_edge1] = 1.0;
	inst->best_sol[new_edge2] = 1.0;
	inst->best_sol[new_edge3] = 1.0;

	/*-----------------CHECK IF CONNECTED COMPONENTS ARE PRESENT-------------*/
	if (kruskal_sst(inst) == 1) {
		if (THREE_OPT > 400)
		{
			printf("RESET %d;%d , %d;%d e %d;%d, AND ADD %d;%d , %d;%d e %d;%d \n",
				random_edge1_nodes[0] + 1, random_edge1_nodes[1] + 1, random_edge2_nodes[0] + 1, random_edge2_nodes[1] + 1,
				random_edge3_nodes[0] + 1, random_edge3_nodes[1],
				random_edge1_nodes[0] + 1, random_edge2_nodes[1] + 1, random_edge2_nodes[0] + 1, random_edge3_nodes[1] + 1,
				random_edge3_nodes[0] + 1, random_edge1_nodes[1] + 1);
			printf("STEP BY %d,%d AND %d TO %d,%d e %d\n",
				old_edge1, old_edge2, old_edge3, new_edge1, new_edge2, new_edge3);
		}

		free(random_edge1_nodes);
		free(random_edge2_nodes);
		free(random_edge3_nodes);
		free(edges);
		return delta;
	}
	else {
		if (THREE_OPT > 400)
		{
			printf("CHANGE NOTHING \n");
		}
		inst->best_sol[old_edge1] = 1.0;
		inst->best_sol[old_edge2] = 1.0;
		inst->best_sol[old_edge3] = 1.0;

		inst->best_sol[new_edge1] = 0.0;
		inst->best_sol[new_edge2] = 0.0;
		inst->best_sol[new_edge3] = 0.0;
		free(random_edge1_nodes);
		free(random_edge2_nodes);
		free(random_edge3_nodes);
		return -1;
	}

}