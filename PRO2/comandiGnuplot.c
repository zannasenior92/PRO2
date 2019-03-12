#include "TSP.h"

//-------------------------COMANDS FOR GNUPLOT----------------------------------------
void plot_coord(instance *inst) {
	char * commandsForGnuplot[] = {
		
		//-----------------------------PATH COLLABORATORS--------------------------------------------
		//BASTA MODIFICARE modificare il pezzo di path da "marco" a "Luca" e viceversa
		
		"set title \"Punti TSP att48\"",
		"set output 'nodes.eps'",
		"set style line 1 \
    linecolor rgb '#0060ad' ", //set the color line
		"unset border", //remove the bordes
		"unset xtics", //remove axis x
		"unset ytics", //remove axis y
		"unset key", //toglie legenda path
		"plot 'C:/Users/Luca/source/repos/PRO2/PRO2/coordinateAtt48.txt' with lp ls 1, '' with labels offset char 1,-1.0 point pointtype 7 lc rgb '#0060ad' ",
		"exit"
	};

	//--------------------NUMBER OF GNUPLOT COMMANDS------------------------------
	int n_commands = sizeof(commandsForGnuplot) / sizeof(commandsForGnuplot[0]);
	
	//--------------------n� COMMANDS GNUPLOT-------------------------------------
	if (VERBOSE>200)
	{
		printf("Numero comandi gnuplot: %d \n", n_commands);
	}


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

	for (int i = 0; i < n_commands; i++)
	{
		fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); //Send commands to gnuplot one by one.
	}
	_pclose(gnuplotPipe);
}