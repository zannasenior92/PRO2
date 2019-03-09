/*
File che contiene le funzioni che usano cplex
*/
#include "TSP.h"

//Funzione per trovare la distanza tra due punti; ritorna un double
double dist(int i, int j, instance *inst){
	double dx = inst->xcoord[i] - inst->xcoord[j];
	double dy = inst->ycoord[i] - inst->ycoord[j];
	return sqrt(dx*dx + dy * dy);
}