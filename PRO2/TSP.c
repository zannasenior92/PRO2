/*
File che contiene le funzioni che usano cplex
*/
#include "TSP.h"
#include <ilcplex/cplex.h>


//-----------------------------FUNCTIONS & METHODS-----------------------------------

//Funzione che mi restituisce la posizione della variabile all'interno del modello
int xpos(int i, int j, instance *inst) { i * inst->nnodes + j-((i+1)*(i+2)/2); }


//Funzione per trovare la distanza tra due punti; ritorna un double
double dist(int i, int j, instance *inst){
	double dx = inst->xcoord[i] - inst->xcoord[j];
	double dy = inst->ycoord[i] - inst->ycoord[j];
	return sqrt(dx*dx + dy * dy);
}



//--------------------------------------------------------------------------------
int TSPopt(instance *inst)
{
	// open cplex model
	int error;
	CPXENVptr env = CPXopenCPLEX(&error); //create the environment(env)
	CPXLPptr lp = CPXcreateprob(env, &error, "TSP"); //create the structure for our model(lp)
}

