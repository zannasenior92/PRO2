#include "TSP.h"

//#include "comandiGnuplot.c" //INCLUDE LA FUNZIONE PER STAMPARE IL GRAFICO
#pragma warning(disable : 4996)
void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);
void plot_coord(instance *inst);
double dist(int i, int j, instance *inst);
void free_instance(instance *inst){
	free(inst->xcoord);
	free(inst->ycoord);
}

int main(int argc, char **argv) {
	//we create the variable inst of type instance
	instance inst;
	//we keep the arguments of the command line
	parse_command_line(argc, argv, &inst);
	printf("Il file di input e': %s\n", inst.input_file);
	//we use the name of the variale related to the instance to access on the field of the instance
	read_input(&inst);
	for (int i = 0; i < inst.nnodes; i++) {
		printf("Capitale %d coord x:%.0f coord y:%.0f\n", i + 1, inst.xcoord[i], inst.ycoord[i]);
	}
	double dProva = dist(0,1, &inst);
	printf("Distanza tra 0 e 1 : %.2f\n", dProva);
	plot_coord(&inst);
	free_instance(&inst);
	
	return 0;
}

void parse_command_line(int argc, char** argv, instance *inst) {
	//default configurations for instance7
	//TODO
	//inst->input_file = "NULL";

	//parsing of the command line
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-input") == 0) { strcpy(inst->input_file, argv[++i]); continue; }
		if (strcmp(argv[i], "-timelimit") == 0) { inst->timelimit = atoi(argv[++i]); continue; }
	}
}

void read_input(instance *inst) {
	//input sarebbe la variabile che identifica il file di input
	FILE *input = fopen(inst->input_file, "r");//inst-> si prende il contenuto del file
	if (input == NULL) { printf(" input file not found!"); exit(1); } //exit(1) finisce il programma
	inst->nnodes = -1; /*dato che ho passato solo il puntatore all'istanza devo usare la freccetta per
					   prendermi il valore all'interno dell'istanza. Se fossi nel main dato che ho dichiarato
					   l'istanza posso usare inst.nnodes per prendermi il valore*/

	char line[180];
	char *par_name;
	char *token1;
	char *token2;
	//
	int coord_section = 0; // =1 NODE_COORD_SECTION
	while (fgets(line, sizeof(line), input) != NULL)
	{
		//printf("%s", line);
		//fflush(NULL);
		if (strlen(line) <= 1) continue; // skip the blank lines
		par_name = strtok(line, " :"); //prende spazio e : come delimitatore e quindi la prima parola è NAME
		//printf("par_name= %s\n", par_name);
		if (strncmp(par_name, "DIMENSION", 9) == 0)
		{
			//if (inst->nnodes >= 0) print_error(" repeated DIMENSION section in input file");
			token1 = strtok(NULL, " :"); //Con il NULL passa al token successivo nella linea
			inst->nnodes = atoi(token1); //string argument to integer
			printf("nnodes %d\n", inst->nnodes);
			inst->xcoord = (double *)calloc(inst->nnodes, sizeof(double));
			inst->ycoord = (double *)calloc(inst->nnodes, sizeof(double));
			continue;
		}
		if (strncmp(par_name, "NODE_COORD_SECTION", 18) == 0)
		{
			if (inst->nnodes <= 0) { printf(" DIMENSION section should appear before NODE_COORD_SECTION section"); exit(1); }
			coord_section = 1;
			continue;
		}
		if (strncmp(par_name, "EOF", 3) == 0)
		{
			coord_section = 0;
			break;
		}
		if (coord_section == 1) {
			int i = atoi(par_name) - 1; //prendo primo valore nelle coordinate, -1 per partire da zero
			token1 = strtok(NULL, " "); //prendo coordinata x
			token2 = strtok(NULL, " "); //prendo coordinata y

			inst->xcoord[i] = atof(token1);
			inst->ycoord[i] = atof(token2);
			continue;
		}
	}
}
void plot_coord(instance *inst) {
	char * commandsForGnuplot[] = {
		"set title \"Punti TSP att48\"",
		"plot \"C:/Users/marco/source/repos/PRO2/PRO2/coordinateAtt48.txt\" with labels point pointtype 7 offset char 1,-1.0 notitle",
		"set output 'nodes.eps'",
		"unset border",
		"unset xtics",
		"unset ytics",
		"exit"
	};
	//-----------------------------PATH COLLABORATORS--------------------------------------------
	/*"plot \"C:/Users/Luca/source/repos/PRO2/PRO2/coordinateAtt48.txt\" using 0:2 title 'title', \
	 '' using 0:2:0 with labels offset 0,char 1",
	//"plot \"C:/Users/marco/source/repos/PRO2/PRO2/coordinateAtt48.txt\" using 0:2 title 'title', \
	 '' using 0:2:0 with labels offset 0,char 1",
	*/
	FILE * temp = fopen("coordinateAtt48.txt", "w");
	/*Opens an interface that one can use to send commands as if they were typing into the
	 *     gnuplot command line.  "The -persistent" keeps the plot open even after your
	 *     C program terminates.
	 */


	for (int i = 0; i < inst->nnodes; i++)
	{
		fprintf(temp, "%lf %lf \n", inst->xcoord[i], inst->ycoord[i]); //Write the data to a temporary file
	}
	fclose(temp);

	FILE * gnuplotPipe = _popen("C:/gnuplot/bin/gnuplot.exe -persistent", "w");
	for (int i = 0; i < 6; i++)
	{
		fprintf(gnuplotPipe, "%s \n", commandsForGnuplot[i]); //Send commands to gnuplot one by one.
	}
	_pclose(gnuplotPipe);
}
