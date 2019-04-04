#include <stdio.h>
#ifndef TSP_H_  

#define TSP_H_

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <malloc.h> //to use malloch for allocation of memory
#include <ilcplex/cplex.h>

#define VERBOSE 50
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
	double best_obj_val;
	int n_connected_comp;
	int *comp;		//COMPONENTI CONNESSE
	int *mycomp;
	int ncols;
	/*-----------------LAST VARIABLE CPLEX INDEX--------------------*/
	int last_x_index;											//LATS INDEX OF x VARIABLES (LAST COLUMN IN CPLEX)
	

} instance;														//NAME OF THE INSTANCE

#endif   /* TSP_H_ */