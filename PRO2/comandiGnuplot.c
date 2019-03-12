#include "TSP.h"

//-------------------------COMANDS FOR GNUPLOT----------------------------------------
void plot_coord(instance *inst) {
	char * commandsForGnuplot[] = {
		"set title \"Punti TSP att48\"",
		//-----------------------------PATH COLLABORATORS--------------------------------------------
		//BASTA MODIFICARE modificare il pezzo di path da "marco" a "Luca" e viceversa
		"plot \"C:/Users/marco/source/repos/PRO2/PRO2/coordinateAtt48.txt\" with labels point pointtype 7 offset char 1,-1.0 notitle",
		"set output 'nodes.eps'",
		"unset border",
		"unset xtics",
		"unset ytics",
		"exit"
	};

	FILE * temp = fopen("coordinateAtt48.txt", "w");
	/*Opens an interface that one can use to send commands as if they were typing into the
	 *     gnuplot command line.  "The -persistent" keeps the plot open even after your
	 *     C program terminates.
	 */


	for (int i = 0; i < inst->nnodes; i++)
	{
		fprintf(temp, "%lf %lf %d \n", inst->xcoord[i], inst->ycoord[i], i+1); //Write the data to a temporary file
	}
	fclose(temp);

	FILE * gnuplotPipe = _popen("C:/gnuplot/bin/gnuplot.exe -persistent", "w");
	for (int i = 0; i < 6; i++)
	{
		fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); //Send commands to gnuplot one by one.
	}
	_pclose(gnuplotPipe);
}