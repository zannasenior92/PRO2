// Gnuplot/C interface library
// Please ensure that the system path includes an entry for the gnuplot binary folder

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gnuplot_c.h"
#include "..\tsp.h"
#include <time.h>

#define GPC_REPLOT 0                // This is being used for testing multiplot/replot which has some strange side effects so is not used in the release

#if defined (_MSC_VER)              // Defined by Microsoft compilers
#include <windows.h>
#if (GPC_DEBUG == 1)
#define GNUPLOT_CMD "pgnuplot"                      // Window pipe version
// #define GNUPLOT_CMD "gnuplot"                    // Do not pipe the text output to null so that it can be used for debugging
// #define GNUPLOT_CMD "gnuplot > debug.log 2>&1"   // Pipe the text output to debug.log for debugging
#else
#define GNUPLOT_CMD "gnuplot > /nul 2>&1"           // Pipe the text output to null for higher performance
#endif
#define popen _popen
#define pclose _pclose
#define mssleep Sleep
// #pragma warning(disable:4996)                           // -D "_CRT_SECURE_NO_WARNINGS=1"
#else                               // Use GNU compiler
#include <unistd.h>
#include <time.h>
#if (GPC_DEBUG == 1)
#define GNUPLOT_CMD "tee debug.log | gnuplot -persist"    // Do not pipe the text output to null so that it can be used for debugging
// #define GNUPLOT_CMD "gnuplot > debug.log"          // Pipe the text output to debug.log for debugging
#else
#define GNUPLOT_CMD "gnuplot > /dev/nul 2>&1"      // Pipe the text output to null for higher performance
#endif
#define mssleep(u) usleep(u*1000)
#endif

/********************************************************
* Function : gpc_init_2d
* From Gnuplot open source library
*
* Parameters :
*   const char *plotTitle,
*   const char *xLabel,
*   const char *yLabel,
*   const double scalingMode,
*   const enum gpcPlotSignMode signMode,
*   const enum gpcMultiFastMode multiFastMode,
*   const enum gpcKeyMode keyMode)
*
* Return value :
*   h_GPC_Plot - Plot handle
*
* Description : Initialize the 2d plot
*
********************************************************/

h_GPC_Plot *gpc_init_2d(const char *plotTitle,
	const char *xLabel,
	const char *yLabel,
	const double scalingMode,
	const enum gpcPlotSignMode signMode,
	const enum gpcMultiFastMode multiFastMode,
	const enum gpcKeyMode keyMode)

{
	h_GPC_Plot *plotHandle;                                 // Create plot

	plotHandle = (h_GPC_Plot*)malloc(sizeof(h_GPC_Plot));    // Malloc plot and check for error
	if (plotHandle == NULL)
	{
		print_error("Fail to create plot handle");
		return (plotHandle);
	}

	plotHandle->pipe = popen(GNUPLOT_CMD, "w");            // Open pipe to Gnuplot and check for error
	if (plotHandle->pipe == NULL)
	{
		printf("\n\nGnuplot/C Error\n");
		printf("Gnuplot/C can not find the required Gnuplot executable.\n");
		printf("Please ensure you have installed Gnuplot from (http://www.gnuplot.info)\n");
		printf("and that the executable program is located in the system PATH.\n\n");

		free(plotHandle);
		return (plotHandle);
	}

	strcpy(plotHandle->plotTitle, plotTitle);              // Set plot title in handle
	plotHandle->multiFastMode = multiFastMode;              // Set multiplot / fastplot mode in handle

	if (multiFastMode == GPC_MULTIPLOT)
	{
		plotHandle->tempFilesUsedFlag = GPC_TRUE;           // Temporary files used - need to delete them in gpc_close ()
		plotHandle->filenameRootId = -1;                    // Initialize filename root id
	}
	else
	{
		plotHandle->tempFilesUsedFlag = GPC_FALSE;          // Temporary files NOT used - DON'T need to delete them in gpc_close ()
	}

	fprintf(plotHandle->pipe, "set term wxt 0 title \"%s\" size %d, %d\n", plotHandle->plotTitle, CANVAS_WIDTH, CANVAS_HEIGHT); // Set the plot
	fprintf(plotHandle->pipe, "set lmargin at screen %4.8lf\n", PLOT_LMARGIN); // Define the margins so that the graph is 512 pixels wide
	fprintf(plotHandle->pipe, "set rmargin at screen %4.8lf\n", PLOT_RMARGIN);
	fprintf(plotHandle->pipe, "set border back\n");            // Set border behind plot

	fprintf(plotHandle->pipe, "set xlabel \"%s\"\n", xLabel);  // Set the X label
	fprintf(plotHandle->pipe, "set ylabel \"%s\"\n", yLabel);  // Set the Y label
	fprintf(plotHandle->pipe, "set grid x y\n");               // Turn on the grid
	fprintf(plotHandle->pipe, "set tics out nomirror\n");      // Tics format

	fprintf(plotHandle->pipe, "set mxtics 4\n");
	fprintf(plotHandle->pipe, "set mytics 2\n");

	if (keyMode == GPC_KEY_ENABLE)
	{
		fprintf(plotHandle->pipe, "set key out vert nobox\n"); // Legend / key location
	}
	else
	{
		fprintf(plotHandle->pipe, "unset key\n");              // Disable legend / key
	}

	if (scalingMode == GPC_AUTO_SCALE)                          // Set the Y axis scaling
	{
		fprintf(plotHandle->pipe, "set autoscale  \n");    // Auto-scale Y axis
	}
	else
	{
		if (signMode == GPC_SIGNED)                             // Signed numbers (positive and negative)
		{
			fprintf(plotHandle->pipe, "set yrange [%1.3le:%1.3le]\n", -scalingMode, scalingMode);
		}
		else if (signMode == GPC_POSITIVE)                      // 0 to +ve Max
		{
			fprintf(plotHandle->pipe, "set yrange [0.0:%1.3le]\n", scalingMode);
		}
		else                                                    // GPC_NEGAIVE - -ve Min to 0
		{
			fprintf(plotHandle->pipe, "set yrange [%1.3le:0.0]\n", -scalingMode);
		}
	}

	fflush(plotHandle->pipe);                                  // flush the pipe
	mssleep(100);
	return (plotHandle);
}


/********************************************************
* Function : gpc_plot_2d
* From Gnuplot open source library, never used in our code, but left as a guideline
*
* Parameters :
*   h_GPC_Plot *plotHandle,
*   const double *pData,
*   const int graphLength,
*   const char *pDataName,
*   const double xMin,
*   const double xMax,
*   const char *plotType,
*   const char *pColour,
*   const enum gpcNewAddGraphMode addMode)
*
* Return value :
*   int - error flag
*
* Description : Generate the 2d plot
*
********************************************************/

int gpc_plot_2d(h_GPC_Plot *plotHandle,
	const double *pData,
	const int graphLength,
	const char *pDataName,
	const double xMin,
	const double xMax,
	const char *plotType,
	const char *pColour,
	const enum gpcNewAddGraphMode addMode)

{
	int   i;
	FILE  *gpdtFile;
	char  tmpFilename[30];
	struct stat   fileStatBuffer;

	if (plotHandle->multiFastMode == GPC_MULTIPLOT)         // GPC_MULTIPLOT
	{
		if (addMode == GPC_NEW)                             // GPC_NEW
		{
			// fprintf (plotHandle->pipe, "set autoscale x\n");      // Auto-scale Y axis

			if (plotHandle->filenameRootId != -1)           // If NOT called immediately after gpc_init_2d () remove existing graphs
			{
				for (i = 0; i <= plotHandle->highestGraphNumber; i++)   // Remove all temporary files
				{
					remove(plotHandle->graphArray[i].filename);
				}
			}

			plotHandle->highestGraphNumber = 0;

			i = -1;
			do                                              // Create a unique local filename - Note this is NOT MT safe !
			{
				i++;
				sprintf(tmpFilename, "%d-0.gpdt", i);
			} while (stat(tmpFilename, &fileStatBuffer) == 0);
			plotHandle->filenameRootId = i;
		}
		else                                // GPC_ADD
		{
			plotHandle->highestGraphNumber++;
			if (plotHandle->highestGraphNumber >= (MAX_NUM_GRAPHS - 1)) // Check we haven't overflowed the maximum number of graphs
			{
				return (GPC_ERROR);
			}
		}

		sprintf(plotHandle->graphArray[plotHandle->highestGraphNumber].filename, "%d-%d.gpdt", plotHandle->filenameRootId, plotHandle->highestGraphNumber);
		sprintf(plotHandle->graphArray[plotHandle->highestGraphNumber].title, "%s", pDataName);
		sprintf(plotHandle->graphArray[plotHandle->highestGraphNumber].formatString, "%s lc rgb \"%s\"", plotType, pColour);

		gpdtFile = fopen(plotHandle->graphArray[plotHandle->highestGraphNumber].filename, "w");    // Open temporary files
		for (i = 0; i < graphLength; i++)                   // Write data to intermediate file
		{
			fprintf(gpdtFile, "%1.3le %1.3le\n", xMin + ((((double)i) * (xMax - xMin)) / ((double)(graphLength - 1))), pData[i]);
		}
		fclose(gpdtFile);
		mssleep(100);                                      // Slow down file accesses to avoid missing data


		fprintf(plotHandle->pipe, "plot \"%s\" using 1:2 title \"%s\" with %s", plotHandle->graphArray[0].filename, plotHandle->graphArray[0].title, plotHandle->graphArray[0].formatString);  // Send start of plot and first plot command
		for (i = 1; i <= plotHandle->highestGraphNumber; i++)   // Send individual plot commands
		{
			fprintf(plotHandle->pipe, ", \\\n \"%s\" using 1:2 title \"%s\" with %s", plotHandle->graphArray[i].filename, plotHandle->graphArray[i].title, plotHandle->graphArray[i].formatString);  // Set plot format
		}
		fprintf(plotHandle->pipe, "\n");                   // Send end of plot command
	}
	else                                                    // GPC_FASTPLOT
	{
		if (addMode == GPC_NEW)
		{
			fprintf(plotHandle->pipe, "set xrange [%1.3le:%1.3le]\n", xMin - (0.5 * ((xMax - xMin) / (graphLength - 1))),
				xMax + (0.5 * ((xMax - xMin) / (graphLength - 1))));  // Set length of X axis
		}

		fprintf(plotHandle->pipe, "plot '-' using 1:2 title \"%s\" with %s lc rgb \"%s\"\n", pDataName, plotType, pColour);  // Set plot format
		for (i = 0; i < graphLength; i++)                   // Copy the data to gnuplot
		{
			fprintf(plotHandle->pipe, "%1.3le %1.3le\n", xMin + ((((double)i) * (xMax - xMin)) / ((double)(graphLength - 1))), pData[i]);
		}
		fprintf(plotHandle->pipe, "e\n");                  // End of dataset
	}                                                       // End of GPC_MULTIPLOT/GPC_FASTPLOT

	fflush(plotHandle->pipe);                              // Flush the pipe

#if GPC_DEBUG
	mssleep(100);                                          // Slow down output so that pipe doesn't overflow when logging results
#endif

	return (GPC_NO_ERROR);
}



/********************************************************
* Function : gpc_my_plot
* Customized version of gpc_plot_2d, used in holding_read_output and keepreading
*
* Parameters :
*   h_GPC_Plot *plotHandle,
*   const char *pDataName,
*   const double xMin,
*   const double yMax,
*   const char *plotType,
*   const char *pColour,
*   const enum gpcNewAddGraphMode addMode)
*
* Return value :
*   h_GPC_Plot - Plot handle
*
* Description : Generate the 2d plot, overwrite any eventual existing plot
*
*
********************************************************/


h_GPC_Plot * gpc_my_plot(h_GPC_Plot *plotHandle,
	const char *pDataName,
	const double xMin,
	const double xMax,
	const char *plotType,
	const char *pColour,
	const enum gpcNewAddGraphMode addMode)

{

	int   i;
	FILE  *gpdtFile;
	char  tmpFilename[30];
	struct stat   fileStatBuffer;

	if (plotHandle->filenameRootId != -1)           // If NOT called immediately after gpc_init_2d () remove existing graphs
	{
		for (i = 0; i <= plotHandle->highestGraphNumber; i++)   // Remove all temporary files
		{
			remove(plotHandle->graphArray[i].filename);
		}
	}

	plotHandle->highestGraphNumber = 0;

	i = -1;
	do                                              // Create a unique local filename - Note this is NOT MT safe !
	{
		i++;
		sprintf(tmpFilename, "%d-0.gpdt", i);
	} while (stat(tmpFilename, &fileStatBuffer) == 0);
	plotHandle->filenameRootId = i;


	sprintf(plotHandle->graphArray[plotHandle->highestGraphNumber].filename, "%d-%d.gpdt", plotHandle->filenameRootId, plotHandle->highestGraphNumber);
	sprintf(plotHandle->graphArray[plotHandle->highestGraphNumber].title, "%s", pDataName);
	sprintf(plotHandle->graphArray[plotHandle->highestGraphNumber].formatString, "%s lc rgb \"%s\"", plotType, pColour);

	mssleep(100);                                      // Slow down file accesses to avoid missing data

	fprintf(plotHandle->pipe, "plot \"src/graph.txt\" title \"%s\" with %s", plotHandle->graphArray[0].title, plotHandle->graphArray[0].formatString);

	fprintf(plotHandle->pipe, "\n");                   // Send end of plot command

	fflush(plotHandle->pipe);                              // Flush the pipe
	mssleep(100);
	return plotHandle;
}



/********************************************************
* Function : gpc_close
* From Gnuplot open source library
*
* Parameters :
*   h_GPC_Plot *plotHandle
*
* Return value :
*   void
*
* Description : Delete the plot and temporary files
*   associated with the handle.
*
********************************************************/

void gpc_close(h_GPC_Plot *plotHandle)
{

	int i;

	mssleep(500);                                          // Wait - ensures pipes flushed

	fprintf(plotHandle->pipe, "exit\n");                   // Close GNUPlot
	pclose(plotHandle->pipe);                              // Close the pipe to Gnuplot
	if (plotHandle->tempFilesUsedFlag == GPC_TRUE)          // If we have used temporary files we need to delete them
	{
		for (i = 0; i <= plotHandle->highestGraphNumber; i++)   // Remove all temporary files
		{
			remove(plotHandle->graphArray[i].filename);
		}
	}
	free(plotHandle);                                      // Free the plot
}


/********************************************************
* Function : holding_read_output
*
* Parameters :
*   instance *inst
*	CPXENVptr env
*	CPXLPptr lp
*
* Return value :
*   void
*
* Description : Initialize and draw a new ovrewritable
*	plot
*	Data taken from Cplex
*
********************************************************/

void holding_read_output(instance *inst, CPXENVptr env, CPXLPptr lp)
{
	int ncols = CPXgetnumcols(env, lp);
	//Save the current solution in inst->bestsol
	CPXgetx(env, lp, inst->best_sol, 0, ncols - 1);

	FILE *fp;
	int i, j;
	fp = fopen("src/graph.txt", "w");
	i = 0;
	j = 1;

	double XMIN = 0.0, XMAX = 0.0;

	//write data in graph.txt
	for (int k = 0; k < ncols; k++)
	{
		if (inst->best_sol[k] > EPSILON)
		{
			fprintf(fp, "%f\t%f\n%f\t%f\n\n", inst->xcoord[i], inst->ycoord[i], inst->xcoord[j], inst->ycoord[j]);
			if (inst->xcoord[i] > XMAX) XMAX = inst->xcoord[i];
			if (inst->xcoord[j] > XMAX) XMAX = inst->xcoord[i];
		}
		if (j < inst->nnodes - 1) j++;
		else
		{
			i++;
			j = i + 1;
		}
	}

	if (fclose(fp)) print_error("error while closing graph.txt");

	// Initialize plot using gnuplot default function
	// A pointer to the graph is saved in instance, so we can eventually overwrite or close it later.    
	inst->graph =
		gpc_init_2d("TSP",           // Plot title
			"X COORD",               // X-Axis label
			"Y COORD",               // Y-Axis label
			GPC_AUTO_SCALE,          // Scaling mode
			GPC_SIGNED,              // Sign mode
			GPC_MULTIPLOT,           // Multiplot / fast plot mode
			GPC_KEY_DISABLE);        // Legend / key mode

	if (inst->graph == NULL)                       // Plot creation failed - e.g is server running ?
	{
		printf("\nPlot creation failure. Please ensure gnuplot is located on your system path\n");
		exit(1);
	}

	inst->graph = gpc_my_plot(inst->graph, "TSP", XMIN, XMAX, "linespoints", "blue", GPC_NEW);


}

/********************************************************
* Function : keepreading
*
* Parameters :
*   instance *inst
*	CPXENVptr env
*	CPXLPptr lp
*
* Return value :
*   void
*
* Description : Draw a new plot over the existing one
*
*	Data taken from Cplex
*
********************************************************/

void keepreading(instance *inst, CPXENVptr env, CPXLPptr lp) {

	int ncols = CPXgetnumcols(env, lp);
	CPXgetx(env, lp, inst->best_sol, 0, ncols - 1);

	FILE *fp;
	int i, j;
	fp = fopen("src/graph.txt", "w");
	i = 0;
	j = 1;

	double XMIN = 0.0, XMAX = 0.0;


	for (int k = 0; k < ncols; k++)
	{
		if (inst->best_sol[k] > EPSILON)
		{
			fprintf(fp, "%f\t%f\n%f\t%f\n\n", inst->xcoord[i], inst->ycoord[i], inst->xcoord[j], inst->ycoord[j]);
			if (inst->xcoord[i] > XMAX) XMAX = inst->xcoord[i];
			if (inst->xcoord[j] > XMAX) XMAX = inst->xcoord[i];
		}
		if (j < inst->nnodes - 1) j++;
		else
		{
			i++;
			j = i + 1;
		}
	}

	if (fclose(fp)) print_error("error while closing graph.txt");

	inst->graph = gpc_my_plot(inst->graph,              // Plot handle
		"TSP",           // Dataset title
		XMIN,            // Minimum X value
		XMAX,            // Maximum X value
		"linespoints",   // Plot type
		"blue",          // Colour
		GPC_NEW);        // New plot

}

/********************************************************
* Function : heuristic_read_output
*
* Parameters :
*   instance *inst, int * arr
*
* Return value :
*   int-errorflag
*
* Description : Print the tour identified as a sequence of
*	vertex in the specified array arr.
*	The best solution ever encountered should always be
*	saved in inst->global; the function can also print other
*	instance's arrays (tour, DueOptSwapper, vert) if needed
*
********************************************************/


int heuristic_read_output(instance * inst, int * arr) {

	if (arr != inst->global && arr != inst->tour && arr != inst->vert && arr != inst->two_opt_swapper) {
		print_error("\nhro WARNING: Unvalid array selected\n");
		return 0;
	}

	FILE *fp;
	int i, j;
	fp = fopen("src/grafo.txt", "w");
	i = 0;
	j = 0;
	for (int k = 0; k < inst->nnodes - 1; k++) {
		i = arr[k];
		j = arr[k + 1];
		fprintf(fp, "%f\t%f\n%f\t%f\n\n", inst->xcoord[i], inst->ycoord[i], inst->xcoord[j], inst->ycoord[j]);
	}
	fprintf(fp, "%f\t%f\n%f\t%f\n\n", inst->xcoord[arr[0]], inst->ycoord[arr[0]], inst->xcoord[arr[inst->nnodes - 1]], inst->ycoord[arr[inst->nnodes - 1]]);
	if (fclose(fp)) {
		print_error("error closing graph.txt");
		return 0;
	}

	system("gnuplot -p src/comando.txt");
	return 1;
}