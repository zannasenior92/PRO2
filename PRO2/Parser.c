#include "TSP.h"

void parse_command_line(int argc, char** argv, instance *inst) {
	
	/*-----------------------------CHECK USER INPUT-------------------------------------*/
	printf("User?(marco/Luca): ");
	char user[10];
	strcpy(user, fgets(user, 10, stdin));
	while ((strncmp(user, "Luca", 4) != 0) && ((strncmp(user, "marco", 5) != 0)))		//CHECK INPUT
	{
		printf("INPUT ERROR! User(marco/Luca)?: ");
		strcpy(user, fgets(user, 10, stdin));
	}

	/*-----------------------INPUT LINE SHELL(yes/no)-----------------------------------*/
	printf("Input method from shell(yes/no)?: ");
	char decision[5];
	strcpy(decision, fgets(decision, 5, stdin));

	while ((strncmp(decision, "yes", 3) != 0) && ((strncmp(decision, "no", 2) != 0)))				//CHECK INPUT
	{
		printf("INPUT ERROR! Input method from shell(yes/no)?: ");
		strcpy(decision, fgets(decision, 5, stdin));
	}

	/*---------------------------------READ INPUT FROM ARGV------------------------------*/
	if (strncmp(decision, "no", 2) == 0)
	{
		for (int i = 0; i < argc; i++) {
			if (strcmp(argv[i], "-input") == 0) { strcpy(inst->input_file, argv[++i]); continue; }
			if (strcmp(argv[i], "-timelimit") == 0) { inst->timelimit = atoi(argv[++i]); continue; }
		}
	}

	/*----------------------------READ THE INPUT FILE FROM SHELL-------------------------*/
	else
	{
		char name_file[100];															//PATH STRING
		char in_file[30];																//FILE NAME
		printf("Insert name file used(Specify the file format es   .dat): ");
		strcpy(name_file, "C:\\Users\\");
		strcat(name_file, user);														//ADD USER STRING
		name_file[strlen(name_file) - 1] = '\0';										//UNCONSIDER \n

		if (strncmp(user, "marco", 5) == 0)
		{
			strcat(name_file, "\\Documents\\RO2\\");									//MARCO'S PATH
		}
		else
		{
			strcat(name_file, "\\source\\repos\\PRO2\\PRO2\\");							//LUCA'S PATH
		}
		fgets(in_file, 30, stdin);														//GET FILE NAME
		in_file[strlen(in_file) - 1] = '\0';											//UNCONSIDER \n
		strcat(name_file, in_file);														//ADD NAME FILE TO THE PATH STRING
		printf("\n");
		printf("Input file selected: %s \n\n", name_file);
		strcpy(inst->input_file, name_file);											//SAVE PATH

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
			inst->choosen_edge = (int *)calloc(inst->nnodes * 2, sizeof(int)); //inizializzo vettore grande 2*numero nodi
			continue;
		}
		/*-------------------------------------------------------------------
		0=distanza euclidea
		1=ATT
		*/
		if (strncmp(par_name, "EDGE_WEIGHT_TYPE", 16) == 0)
		{
			token1 = strtok(NULL, " :");
			if (strncmp(token1, "EUC_2D", 6) == 0) {
				inst->dist_type = 0;
			}
			if (strncmp(token1, "ATT", 3) == 0) {
				inst->dist_type = 1;
			}

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