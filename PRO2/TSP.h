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


	//-----------------------PARAMETERS-----------------------
	char input_file[1000];


	//----------------------GLOBAL DATA-------------------------
	double *best_sol;						// best sol. available
	double *best_lb;						// best lower bound available 

	//---------------------MODEL--------------------------------
	int xstart;


} instance; //"instance" sarebbe il nome che diamo ad una istanza della nostra struttura

#endif   /* TSP_H_ */