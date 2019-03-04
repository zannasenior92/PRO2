#include "TSP.h"

void parse_command_line(int argc, char** argv, instance *inst);

int main(int argc, char **argv) {
	printf("Ci sono %d elementi.\n", argc);
	for (int i = 0; i < argc; i++) {
		printf(argv[i]);
		printf("\n");
	}
	instance inst;
	parse_command_line(argc, argv, &inst);
	return 0;
}

void parse_command_line(int argc, char** argv, instance *inst) {
	//default configurations for instance 
	//inst->input_file = "NULL";
	
	//parsing of the command line
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-input") == 0) { strcpy(inst->input_file, argv[++i]); continue; }
		
	}
}