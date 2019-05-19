
/*-------------------------------------------3-OPT FUNCTION-------------------------------------------------*/

#include "TSP.h"

/*-----------------------------FUNCTIONS & METHODS-----------------------------------*/
double dist(int i, int j, instance *inst);
int xpos(int i, int j, instance *inst);
void reverse_xpos(int x, instance* inst, int* nodes);
void update_choosen_edge(instance* inst);
void add_edge_to_file(instance *inst);
void plot_gnuplot(instance *inst);
int kruskal_sst(CPXENVptr env, CPXLPptr lp, instance *inst);

double three_opt(instance *inst, CPXENVptr env, CPXLPptr lp)
{
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

	double min_delta = 0;
	double edge1_length, edge2_length, edge3_length;

	double new_distA1, new_distA2, new_distA3;
	double new_distB1, new_distB2, new_distB3;
	double new_distC1, new_distC2, new_distC3;
	double new_distD1, new_distD2, new_distD3;

	int min_new_edge1, min_new_edge2, min_new_edge3;
	int old_edge1, old_edge2, old_edge3;
	int *nodes_edge1 = (int*)calloc(2, sizeof(int));
	int *nodes_edge2 = (int*)calloc(2, sizeof(int));
	int *nodes_edge3 = (int*)calloc(2, sizeof(int));
	int done = 0;

	for (int i = 0; i < inst->nnodes; i++)
	{
		reverse_xpos(edges[i], inst, nodes_edge1);						//TAKE FIRST EDGE
		
		for (int j = i + 1; j < inst->nnodes-2; j++)
		{
			reverse_xpos(edges[j],inst, nodes_edge2);					//TAKE SECOND EDGE
		
			for (int k = j + 1; k < inst->nnodes-1; k++)
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

				/*-------------------------TAKE THE LENGTH OF OLD EDGES-----------------------*/
				edge1_length = dist(nodes_edge1[0], nodes_edge1[1], inst);
				edge2_length = dist(nodes_edge2[0], nodes_edge2[1], inst);
				edge3_length = dist(nodes_edge3[0], nodes_edge3[1], inst);
				/*MIN NEW EDGES = OLD EDGES*/
				min_new_edge1 = xpos(nodes_edge1[0], nodes_edge1[1], inst);
				min_new_edge2 = xpos(nodes_edge2[0], nodes_edge2[1], inst);
				min_new_edge3 = xpos(nodes_edge3[0], nodes_edge3[1], inst);
				/******************************************************************************/
				
				//---------1) TAKE LENGTH OF FIRST NEW PRESUMED EGDES
				new_distA1 = dist(nodes_edge1[0], nodes_edge2[1], inst);
				new_distA2 = dist(nodes_edge2[0], nodes_edge3[1], inst);
				new_distA3 = dist(nodes_edge3[0], nodes_edge1[1], inst);
				//---------SEE THE DIFFERENCE
				double deltaA = new_distA1 + new_distA2 + new_distA3 - edge1_length - edge2_length - edge3_length;
				if (deltaA < min_delta)
				{
					min_delta = deltaA;
					/*----------------CHOOSE THE THREE TWO EDGES---------------*/
					min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[1], inst);
					min_new_edge2 = xpos(nodes_edge2[0], nodes_edge3[1], inst);
					min_new_edge3 = xpos(nodes_edge3[0], nodes_edge1[1], inst);
				}
				//--------------------------------------------------

				//---------2) TAKE LENGTH OF SECOND NEW PRESUMED EGDES
				new_distB1 = dist(nodes_edge1[0], nodes_edge2[0], inst);
				new_distB2 = dist(nodes_edge1[1], nodes_edge3[0], inst);
				new_distB3 = dist(nodes_edge3[1], nodes_edge2[1], inst);
				//---------SEE THE DIFFERENCE
				double deltaB = new_distB1 + new_distB2 + new_distB3 - edge1_length - edge2_length - edge3_length;
				if (deltaB < min_delta)
				{
					min_delta = deltaB;
					/*----------------CHOOSE THE NEW THREE EDGES---------------*/
					min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[0], inst);
					min_new_edge2 = xpos(nodes_edge1[1], nodes_edge3[0], inst);
					min_new_edge3 = xpos(nodes_edge3[1], nodes_edge2[1], inst);
				}
				//--------------------------------------------------

				//---------3) TAKE LENGTH OF THIRD NEW PRESUMED EGDES
				new_distC1 = dist(nodes_edge1[0], nodes_edge2[1], inst);
				new_distC2 = dist(nodes_edge1[1], nodes_edge3[1], inst);
				new_distC3 = dist(nodes_edge3[0], nodes_edge2[0], inst);
				//---------SEE THE DIFFERENCE
				double deltaC = new_distC1 + new_distC2 + new_distC3 - edge1_length - edge2_length - edge3_length;
				if (deltaC < min_delta)
				{
					min_delta = deltaC;
					/*----------------CHOOSE THE NEW THREE EDGES---------------*/
					min_new_edge1 = xpos(nodes_edge1[0], nodes_edge2[1], inst);
					min_new_edge2 = xpos(nodes_edge1[1], nodes_edge3[1], inst);
					min_new_edge3 = xpos(nodes_edge3[0], nodes_edge2[0], inst);
				}
				//--------------------------------------------------

				//---------4) TAKE LENGTH OF FOURTH NEW PRESUMED EGDES
				new_distD1 = dist(nodes_edge1[1], nodes_edge2[1], inst);
				new_distD2 = dist(nodes_edge1[0], nodes_edge3[0], inst);
				new_distD3 = dist(nodes_edge3[1], nodes_edge2[0], inst);
				//---------SEE THE DIFFERENCE
				double deltaD = new_distD1 + new_distD2 + new_distD3 - edge1_length - edge2_length - edge3_length;
				if (deltaD < min_delta)
				{
					min_delta = deltaD;
					/*----------------CHOOSE THE NEW THREE EDGES---------------*/
					min_new_edge1 = xpos(nodes_edge1[1], nodes_edge2[1], inst);
					min_new_edge2 = xpos(nodes_edge1[0], nodes_edge3[0], inst);
					min_new_edge3 = xpos(nodes_edge3[1], nodes_edge2[0], inst);
				}
				/******************************************************************************/


				if ((inst->best_sol[min_new_edge1] == 1) || (inst->best_sol[min_new_edge2] == 1) 
					|| (inst->best_sol[min_new_edge3] == 1)) {
					continue;
				}
				old_edge1 = edges[i];
				old_edge2 = edges[j];
				old_edge3 = edges[k];
				if (THREE_OPT > 400)
					{
						printf("best_sol[%d]=%f\n", old_edge1, inst->best_sol[old_edge1]);
						printf("best_sol[%d]=%f\n", old_edge2, inst->best_sol[old_edge2]);
						printf("best_sol[%d]=%f\n", old_edge3, inst->best_sol[old_edge3]);

						printf("best_sol[%d]=%f\n", min_new_edge1, inst->best_sol[min_new_edge1]);
						printf("best_sol[%d]=%f\n", min_new_edge2, inst->best_sol[min_new_edge2]);
						printf("best_sol[%d]=%f\n", min_new_edge3, inst->best_sol[min_new_edge3]);

					}
				/*-----------------UPDATE SELECTION----------------------*/
				inst->best_sol[old_edge1] = 0.0;
				inst->best_sol[old_edge2] = 0.0;
				inst->best_sol[old_edge3] = 0.0;

				inst->best_sol[min_new_edge1] = 1.0;
				inst->best_sol[min_new_edge2] = 1.0;
				inst->best_sol[min_new_edge3] = 1.0;

				/*-----------------CHECK IF CONNECTED COMPONENTS ARE PRESENT-------------*/
				if (kruskal_sst(env, lp, inst) == 1) {
					if (THREE_OPT > 400)
						{
							printf("RESET %d;%d , %d;%d e %d;%d, AND ADD %d;%d , %d;%d e %d;%d \n",
								nodes_edge1[0] + 1, nodes_edge1[1] + 1, nodes_edge2[0] + 1, nodes_edge2[1] + 1,
								nodes_edge3[0] + 1, nodes_edge3[1], 
								nodes_edge1[0] + 1, nodes_edge2[1] + 1, nodes_edge2[0] + 1, nodes_edge3[1] + 1,
								nodes_edge3[0] + 1, nodes_edge1[1] + 1);
							printf("STEP BY %d,%d AND %d TO %d,%d e %d\n",
								old_edge1, old_edge2, old_edge3, min_new_edge1, min_new_edge2, min_new_edge3);
						}

					free(nodes_edge1);
					free(nodes_edge2);
					free(nodes_edge3);
					free(edges);
					return min_delta;
				}
				else {
					if (THREE_OPT > 400)
					{
						printf("CHANGE NOTHING \n");
					}
					inst->best_sol[old_edge1] = 1.0;
					inst->best_sol[old_edge2] = 1.0;
					inst->best_sol[old_edge3] = 1.0;

					inst->best_sol[min_new_edge1] = 0.0;
					inst->best_sol[min_new_edge2] = 0.0;
					inst->best_sol[min_new_edge3] = 0.0;
					continue;
				}
			}
		}
	}
	free(nodes_edge1);
	free(nodes_edge2);
	free(nodes_edge3);
	free(edges);
	return 0;

}

