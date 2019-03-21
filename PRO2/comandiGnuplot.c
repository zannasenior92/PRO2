#include "TSP.h"

/*-------------------------------GNUPLOT PLOT-------------------------------------------*/
void plot_gnuplot(instance *inst) {
	//char file_name = inst->input_file_name;
	char * commandsForGnuplot[] = {

		/*-------------------------PLOTTING COMMANDS TO PRINT NODES---------------------*/
		
		"set terminal windows",
		"set title \"Punti TSP att48\"", 
		"set output 'nodes.eps'",
		"set style line 1 \
    linecolor rgb '#0060ad' ",									//set the color line
		"unset border",											//remove the bordes
		"unset xtics",											//remove axis x
		"unset ytics",											//remove axis y
		"unset key",											//remove path legend

		"plot 'coordinateAtt48.txt' with labels offset char 1,-1.0 point pointtype 7 lc rgb '#0060ad' ",
		/*------------------------------------------------------------------------------*/


		/*----------------PLOTTING COMMANDS TO PRINT SELECTED EDGES---------------------*/

		"set terminal windows 1", // set a different window to plot with gnuplot
		"set title \"Lines TSP att48\"",
		"set output 'nodes.eps'",
		"set style line 1 \
    linecolor rgb '#0060ad' ",									//set the color line
		"unset border",											//remove the bordes
		"unset xtics",											//remove axis x
		"unset ytics",											//remove axis y
		"unset key",											//remove path legend
		"plot 'edge_to_plotMTZ.txt' with lp ls 1, '' with labels offset char 1,-1.0 point pointtype 7 lc rgb '#0060ad' ",
		"exit"
	};
	/*----------------------------------------------------------------------------------*/

	/*--------------------NUMBER OF GNUPLOT COMMANDS------------------------------------*/
	int n_commands = sizeof(commandsForGnuplot) / sizeof(commandsForGnuplot[0]);
	if (VERBOSE>200)
	{
		printf("Numero comandi gnuplot: %d \n", n_commands);
	}
	/*----------------------------------------------------------------------------------*/

	
	/*---------------------------PRINTING POINTS IN FILE--------------------------------*/
	FILE * temp = fopen("coordinateAtt48.txt", "w");
	/*Opens an interface that one can use to send commands as if they were typing into the
	  gnuplot command line.  "The -persistent" keeps the plot open even after your
	  C program terminates.
	 */
	for (int i = 0; i < inst->nnodes; i++)
	{
		fprintf(temp, "%lf %lf %d \n", inst->xcoord[i], inst->ycoord[i], i+1); //Write the data to a temporary file
	}
	fclose(temp);
	/*----------------------------------------------------------------------------------*/
	
	
	/*----------------USING A PIPE FOR GNUPLOT TO PRINT POINTS--------------------------*/
	FILE * gnuplotPipe = _popen("C:/gnuplot/bin/gnuplot.exe -persistent", "w");

	for (int i = 0; i < n_commands; i++)
	{
		fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); //Send commands to gnuplot one by one.
	}
	_pclose(gnuplotPipe);
}
/*--------------------------------------------------------------------------------------*/



/*---------------------------------FILE WITH NODES TO PLOT------------------------------*/ 
void add_edge_to_file(instance *inst) {
	FILE * file = fopen("edge_to_plotMTZ.txt", "w");
	for (int i = 0; i < 2 * inst->nnodes; i = i + 2) {
		fprintf(file, "%lf %lf %d \n", inst->xcoord[inst->choosen_edge[i]], inst->ycoord[inst->choosen_edge[i]], inst->choosen_edge[i] + 1); //Write x_i to a temporary file
		fprintf(file, "%lf %lf %d \n", inst->xcoord[inst->choosen_edge[i + 1]], inst->ycoord[inst->choosen_edge[i + 1]], inst->choosen_edge[i + 1] + 1); //Write x_i to a temporary file
		fprintf(file, "\n");
	}
	fclose(file);
}
