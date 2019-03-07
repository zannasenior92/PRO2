#include <stdio.h>
#ifndef TSP_H_  

#define TSP_H_

#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <malloc.h> //to use malloch for allocation of memory
#include <ilcplex/cplex.h>

#define VERBOSE 50

typedef struct {
	int nnodes;
	double *xcoord;
	double *ycoord;
	double timelimit;
	char input_file[1000];
} instance; //"instance" sarebbe il nome che diamo ad una istanza della nostra struttura

#endif   /* TSP_H_ */