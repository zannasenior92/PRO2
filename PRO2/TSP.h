#ifndef VRP_H_

#define VRP_H_

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define VERBOSE 50

typedef struct {
	int nnodes;
	double *xcoord;
	double *ycoord;
	double timelimit;
	char input_file[1000];
} instance;//"instance" sarebbe il nome che diamo ad una istanza della nostra struttura

#endif VRP_H_