#include "TSP.h"
#pragma warning(disable : 4996)
void parse_command_line(int argc, char** argv, instance *inst);
void read_input(instance *inst);
/*void free_instance(instance *inst)
{
	free(inst->xcoord);
	free(inst->ycoord);
}*/

int main(int argc, char **argv) {
	/*printf("Ci sono %d elementi.\n", argc);
	for (int i = 0; i < argc; i++) {
		printf(argv[i]);
		printf("\n");
	}*/

	//creiamo l'istanza
	instance inst;
	//prendiamo gli argomenti della riga di comando
	parse_command_line(argc, argv, &inst);
	printf("Il file di input e': %s\n", inst.input_file);
	read_input(&inst);
	//free_instance(&inst);
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
	FILE *input = fopen(inst->input_file, "r");
	if (input == NULL) { printf(" input file not found!"); exit(1); }
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
		if (strlen(line) <= 1) continue; // salta righe vuote
		par_name = strtok(line, " :");
		
		if (strncmp(par_name, "DIMENSION", 9) == 0)
		{
			//if (inst->nnodes >= 0) print_error(" repeated DIMENSION section in input file");
			token1 = strtok(NULL, " :"); //prende quello che c'è dopo i due punti fino al null
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
			int i = atoi(par_name) - 1;
			printf("par_name %d\n", i);
			token1 = strtok(NULL, " ");
			token2 = strtok(NULL, " ");
			printf(token1);
			printf("\n");
			printf(token2);
			printf("\n");

			/*if (i < 0 || i >= inst->nnodes) { printf(" unknown node in NODE_COORD_SECTION section"); exit(1); }
			token1 = strtok(NULL, " :,");
			token2 = strtok(NULL, " :,");
			inst->xcoord[i] = atof(token1);
			inst->ycoord[i] = atof(token2);
			printf(" node %4d at coordinates ( %15.7lf , %15.7lf )\n", i + 1, inst->xcoord[i], inst->ycoord[i]);*/
			continue;
		
		}
	}

}