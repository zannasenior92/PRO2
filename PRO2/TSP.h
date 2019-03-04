#pragma once
#include <stdio.h>
#include <math.h>

#define VERBOSE 50

typedef struct {
	int nnodes;
	double *xcoord;
	double *ycoord;
	double timelimit;
} instance;
