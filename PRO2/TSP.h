#include <stdio.h>
#ifndef TSP_H_  

#define TSP_H_

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <malloc.h> //to use malloch for allocation of memory
#include <ilcplex/cplex.h>

#define VERBOSE 50
#define TSP 50
#define GNUPLOT 50
#define DIST_AND_POS 50
#define MODELS 50
#define PARSER 50
#define SELECTED_EDGES 50
//------------------HEURISTIC
#define HARD_FIXING 50
#define NEAREST_NEIGH 50
#define NEAREST_NEIGH_GRASP 50
#define TWO_OPT 50
#define THREE_OPT 50
#define GENETIC_ALG 50
//------------------MODELS
#define DEFAULT_MODEL 50
#define KRUSKAL 50
#define MYSEPARATION 50
#define FISCH 50
#define FLOW1 50
#define FLOW2 50
#define MTZ 50


#define TOLERANCE 0.5
typedef struct {


	//--------------------INPUT DATA--------------------------
	int nnodes;
	double *xcoord;
	double *ycoord;
	double timelimit;
	int dist_type;
	int model_type;
	int compact;
	char input_file_name[100];


	//-----------------------PARAMETERS-----------------------
	char input_file[1000];


	//----------------------GLOBAL DATA-------------------------
	double *best_sol;						// best sol. available
	int *choosen_edge;						// archi scelti
	int *choosen_nodes;						// nodi scelti
	double best_obj_val;
	int n_connected_comp;
	int *comp;		//COMPONENTI CONNESSE
	int *mycomp;
	int ncols;
	CPXLPptr lp;
	CPXENVptr env;
	/*-----------------LAST VARIABLE CPLEX INDEX--------------------*/
	int last_x_index;											//LATS INDEX OF x VARIABLES (LAST COLUMN IN CPLEX)
	double starting_solution_cost;

} instance;														//NAME OF THE INSTANCE

#endif   /* TSP_H_ */