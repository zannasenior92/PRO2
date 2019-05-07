#include "TSP.h"
#include <string.h>
/*-------------------------------GNUPLOT PLOT-------------------------------------------*/
void plot_gnuplot(instance *inst) {

	char title[100];										//NAME FILE
	strcpy(title, "set title \"Punti TSP ");
	strcat(title, inst->input_file_name);

	char * commandsForGnuplot[] = {

		/*-------------------------PLOTTING COMMANDS TO PRINT NODES---------------------*/
		/*
		"set terminal windows",
		title,													//set title from input file
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
		title,													//set title from input file
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
	if (VERBOSE > 200)
	{
		printf("Numero comandi gnuplot: %d \n", n_commands);
	}
	/*----------------------------------------------------------------------------------*/


	/*---------------------------PRINTING POINTS IN FILE--------------------------------*/
	FILE * temp = fopen("coordinateAtt48.txt", "w");

	for (int i = 0; i < inst->nnodes; i++)
	{
		fprintf(temp, "%lf %lf %d \n", inst->xcoord[i], inst->ycoord[i], i + 1);  //WRITE DATA TO A TEMPORARY FILE
	}
	fclose(temp);
	/*----------------------------------------------------------------------------------*/


	/*----------------USING A PIPE FOR GNUPLOT TO PRINT POINTS--------------------------*/
	FILE * gnuplotPipe = _popen("C:/gnuplot/bin/gnuplot.exe -persistent", "w");	//"-persistent" KEEPS THE PLOT OPEN EVEN AFTER YOUR C PROGRAM QUIT

	for (int i = 0; i < n_commands; i++)
	{
		fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]);					//Send commands to gnuplot one by one.
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

void update_choosen_edge(instance* inst) {
	int n = 0;
	for (int i = 0; i < inst->nnodes; i++) {
		for (int j = i + 1; j < inst->nnodes; j++) {
			if (inst->best_sol[xpos(i, j, inst)] > 0.5) {

				if (VERBOSE >= 100) {
					printf("Il nodo (%d,%d) e' selezionato\n", i + 1, j + 1);
				}
				/*--ADD EDGES(VECTOR LENGTH = 2*nnodes TO SAVE NODES OF EVERY EDGE)--*/
				inst->choosen_edge[n] = i;
				inst->choosen_edge[n + 1] = j;
				n += 2;

			}
		}
	}
}
