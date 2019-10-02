#include "TSP.h"
#include <string.h>
/*-------------------------------GNUPLOT PLOT-------------------------------------------*/
void plot_gnuplot_start(instance *inst, FILE* gnuplotPipe) {

	char title[100];										//NAME FILE
	strcpy(title, "set title \"Punti TSP ");
	strcat(title, inst->input_file_name);

	char * commandsForGnuplot[] = {

		/*----------------PLOTTING COMMANDS TO PRINT SELECTED EDGES---------------------*/

		"set terminal windows 1", // set a different window to plot with gnuplot
		title,													//set title from input file
		"set key opaque box",										//set legend
		"set output 'nodes.eps'",
		"unset border",											//remove the bordes
		"unset xtics",											//remove axis x
		"unset ytics",											//remove axis y
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
		fprintf(temp, "%lf %lf %d %d\n", inst->xcoord[i], inst->ycoord[i], i + 1);  //WRITE DATA TO A TEMPORARY FILE
	}
	fclose(temp);
	/*----------------------------------------------------------------------------------*/


	/*----------------USING A PIPE FOR GNUPLOT TO PRINT POINTS--------------------------*/

	for (int i = 0; i < n_commands; i++)
	{
		fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]);					//Send commands to gnuplot one by one.
	}
	
}
/*--------------------------------------------------------------------------------------*/
void plot_gnuplot(instance *inst, FILE * gnuplotPipe) {

	char title[100];										//NAME FILE
	strcpy(title, "set title \"Punti TSP ");
	strcat(title, inst->input_file_name);

	char * commandsForGnuplot[] = {

		/*----------------PLOTTING COMMANDS TO PRINT SELECTED EDGES---------------------*/

		
		/*------------------TO STAMP CONNECTED COMPONENTS-------------------------------*/
		"set terminal windows 1",
		"plot 'connected_components.txt' with lp ls 1 lc variable, '' with point pointtype 7 lc variable",
		"pause 1",
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
		fprintf(temp, "%lf %lf %d %d\n", inst->xcoord[i], inst->ycoord[i], i + 1);  //WRITE DATA TO A TEMPORARY FILE
	}
	fclose(temp);
	/*----------------------------------------------------------------------------------*/


	/*----------------USING A PIPE FOR GNUPLOT TO PRINT POINTS--------------------------*/
	for (int i = 0; i < n_commands; i++)
	{
		fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]);					//Send commands to gnuplot one by one.
	}
	
}


/*----------------------METODO PER STAMPARE LE COMPONENTI CONNESSE SU FILE--------------*/
void add_edge_to_file(instance *inst) {
	FILE * file = fopen("connected_components.txt", "w");
	for (int i = 0; i < 2 * inst->nnodes; i = i + 2) {
		fprintf(file, "%lf %lf %d\n", inst->xcoord[inst->choosen_edge[i]], inst->ycoord[inst->choosen_edge[i]], inst->comp[inst->choosen_edge[i]]); //Write x_i to a temporary file
		fprintf(file, "%lf %lf %d\n", inst->xcoord[inst->choosen_edge[i + 1]], inst->ycoord[inst->choosen_edge[i + 1]], inst->comp[inst->choosen_edge[i + 1]]); //Write x_i to a temporary file

		fprintf(file, "\n");
	}
	fclose(file);
}


void plot_final_gnuplot(instance *inst) {

	char title[100];										//NAME FILE
	strcpy(title, "set title \"Punti TSP ");
	strcat(title, inst->input_file_name);

	char * commandsForGnuplot[] = {

		/*----------------PLOTTING COMMANDS TO PRINT SELECTED EDGES---------------------*/

		"set terminal windows 1", // set a different window to plot with gnuplot
		title,													//set title from input file
		"set key opaque box",										//set legend
		"set output 'nodes.eps'",
		"unset border",											//remove the bordes
		"unset xtics",											//remove axis x
		"unset ytics",											//remove axis y
		/*------------------TO STAMP CONNECTED COMPONENTS-------------------------------*/
		"plot 'connected_components.txt' with lp ls 1 lc variable, '' with point pointtype 7 lc variable",
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
		fprintf(temp, "%lf %lf %d %d\n", inst->xcoord[i], inst->ycoord[i], i + 1);  //WRITE DATA TO A TEMPORARY FILE
	}
	fclose(temp);
	/*----------------------------------------------------------------------------------*/


	/*----------------USING A PIPE FOR GNUPLOT TO PRINT POINTS--------------------------*/
	FILE * gnuplotPipe = _popen("C:/gnuplot/bin/gnuplot.exe -permanent", "w");	//"-persistent" KEEPS THE PLOT OPEN EVEN AFTER YOUR C PROGRAM QUIT

	for (int i = 0; i < n_commands; i++)
	{
		fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]);					//Send commands to gnuplot one by one.
	}
	_pclose(gnuplotPipe);
}