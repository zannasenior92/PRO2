#pragma once
#include <stdio.h>
#include <math.h>
#include <string.h>

#define VERBOSE 50

typedef struct {
	int nnodes;
	double *xcoord;
	double *ycoord;
	double timelimit;
	char input_file[1000];
} instance;
