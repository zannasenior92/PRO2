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
	char input_file_name[100];

	//-----------------------PARAMETERS-----------------------
	char input_file[1000];
	double best_obj_val;



	//----------------------GLOBAL DATA-------------------------
	double *best_sol;						// best sol. available
	int *choosen_edge;						// Nodi scelti 


} instance; //"instance" would be the name that we give to an instance of our structure

#endif   /* TSP_H_ */