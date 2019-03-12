//-------------------FUNZIONE PARSER----------------------------------
#include "TSP.h"

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
		par_name = strtok(line, " :"); //prende spazio e : come delimitatore e quindi la prima parola � NAME
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