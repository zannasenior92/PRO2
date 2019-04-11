#include "TSP.h"

/*------------------POSITION OF VARIABLE INSIDE THE MODEL----------------------------*/
int xpos(int i, int j, instance *inst) {
	if (i > j) return xpos(j, i, inst);
	return i * inst->nnodes + j - ((i + 1)*(i + 2) / 2);
}
int xpos_compact(int i, int j, instance *inst) {
	return i * inst->nnodes + j;
}
int ypos(int i, int j, instance *inst) {
	return (inst->last_x_index + 1) + i * inst->nnodes + j;
}
int zpos(int i, int j, instance *inst) {
	return ((xpos(inst->nnodes, inst->nnodes, inst) + (i * inst->nnodes + j)) + 1);
}
int zpos_flow2(int i, int j, instance *inst) {
	return ((ypos(inst->nnodes - 1, inst->nnodes - 1, inst) + (i * inst->nnodes + j)) + 1);
}
int upos(int i, instance *inst) {
	return inst->nnodes*inst->nnodes + i;
}

/*-------------------------DISTANCE BETWEEN TWO POINTS-------------------------------*/
double dist(int i, int j, instance *inst) {

	/*-------------------------EUC_2D DISTANCE--------------------------*/
	if (inst->dist_type == 0) {
		double dx = inst->xcoord[i] - inst->xcoord[j];
		double dy = inst->ycoord[i] - inst->ycoord[j];
		return (int)(sqrt((dx*dx + dy * dy)) + 0.5);
	}
	/*--------------------------ATT DISTANCE----------------------------*/
	if (inst->dist_type == 1) {
		double dx = inst->xcoord[i] - inst->xcoord[j];
		double dy = inst->ycoord[i] - inst->ycoord[j];
		double rij = sqrt((dx*dx + dy * dy) / 10.0);
		int tij = (int)(rij + 0.5);
		if (tij < rij)
			return (tij + 1);
		else
			return tij;
	}
	/*-------------------------GEO DISTANCE-----------------------------*/
	if (inst->dist_type == 2) {
		double PI = 3.141592;
		double deg = (int)(inst->xcoord[i]);
		double min = inst->xcoord[i] - deg;
		double lati = PI * (deg + 5.0*min / 3.0) / 180.0;
		deg = (int)(inst->ycoord[i]);
		min = inst->ycoord[i] - deg;
		double longi = PI * (deg + 5.0*min / 3.0) / 180.0;

		deg = (int)(inst->xcoord[j]);
		min = inst->xcoord[j] - deg;
		double latj = PI * (deg + 5.0*min / 3.0) / 180.0;
		deg = (int)(inst->ycoord[j]);
		min = inst->ycoord[j] - deg;
		double longj = PI * (deg + 5.0*min / 3.0) / 180.0;

		double RRR = 6378.388;
		double q1 = cos(longi - longj);
		double q2 = cos(lati - latj);
		double q3 = cos(lati + latj);
		int dij = (int)(RRR*acos(0.5*((1.0 + q1)*q2 - (1.0 - q1)*q3)) + 1.0);
		return dij;
	}
	else print_error("Something go wrong in distance");
}