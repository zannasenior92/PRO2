#include "TSP.h"

void parse_command_line(int argc, char** argv, instance *inst) {
	//default configurations for instance7
	//TODO
	//inst->input_file = "NULL";

	/*--------------------------PARSING OF THE COMMAND LINE-------------------------------*/
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-input") == 0) { strcpy(inst->input_file, argv[++i]); continue; }
		if (strcmp(argv[i], "-timelimit") == 0) { inst->timelimit = atoi(argv[++i]); continue; }
	}
}

/*--------------------------------------READ INPUT----------------------------------------*/
void read_input(instance *inst) {

	FILE *input = fopen(inst->input_file, "r");
	if (input == NULL) { printf(" input file not found!"); exit(1); } //exit(1) stop the program
	inst->nnodes = -1;

	char line[180];
	char *par_name;
	char *token1;
	char *token2;
	
	int coord_section = 0; // =1 NODE_COORD_SECTION
	while (fgets(line, sizeof(line), input) != NULL)
	{
		//printf("%s", line);
		//fflush(NULL);

		if (strlen(line) <= 1) continue; // skip the blank lines
		par_name = strtok(line, " :"); //take space and : as delimiter e then first word is NAME
		//printf("par_name= %s\n", par_name);

		/*--------------------------------CHECK FILE LINES--------------------------------*/
		if (strncmp(par_name, "DIMENSION", 9) == 0)
		{
			//if (inst->nnodes >= 0) print_error(" repeated DIMENSION section in input file");
			token1 = strtok(NULL, " :"); //take the successive item of the line
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
		/*----------------------INSERT COORDINATES IN STRUCTURE TSP.h----------------------*/
		if (coord_section == 1) {
			int i = atoi(par_name) - 1; //index of the point
			token1 = strtok(NULL, " "); //first coordinate x
			token2 = strtok(NULL, " "); //second coordinate y
			inst->xcoord[i] = atof(token1);//insert coordinate x
			inst->ycoord[i] = atof(token2);//insert coordinate y
			continue;
		}
	}
}