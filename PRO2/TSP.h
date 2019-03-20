#include <stdio.h>
#ifndef TSP_H_  

#define TSP_H_

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <malloc.h> //to use malloch for allocation of memory


#define VERBOSE 50

typedef struct {


	//--------------------INPUT DATA--------------------------
	int nnodes;
	double *xcoord;
	double *ycoord;
	double timelimit;
	int dist_type;

	//-----------------------PARAMETERS-----------------------
	char input_file[1000];


	//----------------------GLOBAL DATA-------------------------
	double *best_sol;						// best sol. available
	int *choosen_edge;						// Nodi scelti 
	double best_obj_val;

} instance; //"instance" sarebbe il nome che diamo ad una istanza della nostra struttura

#endif   /* TSP_H_ */