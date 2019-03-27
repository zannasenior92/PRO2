/*-----------------------------------PARSER FUNCTION------------------------------------*/
#include "TSP.h"


/*----------------------------------COMMAND LINE PARSING--------------------------------*/
void parse_command_line(int argc, char** argv, instance *inst) {
	
	
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-input") == 0) { strcpy(inst->input_file, argv[++i]); continue; }
		if (strcmp(argv[i], "-model") == 0) { 
			/*MODEL TYPE=
				default=0
				FLOW1=1
				MTZ=2
				FISCHETTI=3
				*/
			if ((strcmp(argv[++i], "flow1") == 0)) {
				inst->model_type = 1;
				continue;
			}
			if (strcmp(argv[++i], "mtz") == 0) {
				inst->model_type = 2;
				continue;
			}
			if (strcmp(argv[++i], "fischetti") == 0) {
				inst->model_type = 3;
				continue;
			}
		}
		if (strcmp(argv[i], "-timelimit") == 0) { inst->timelimit = atoi(argv[++i]); continue; }
	}
}

/*-----------------------------------READ THE INPUT-------------------------------------*/
void read_input(instance *inst) {

	FILE *input = fopen(inst->input_file, "r");
	if (input == NULL) { printf(" input file not found!"); exit(1); } 
	inst->nnodes = -1; 

	char line[180];
	char *par_name;															//LINE TAKEN BY THE PARSER
	char *token1;
	char *token2;
	int coord_section = 0;													// =1 NODE_COORD_SECTION
	
	/*------------------------------------READER----------------------------------------*/
	while (fgets(line, sizeof(line), input) != NULL)
	{
		
		if (strlen(line) <= 1) continue;									// SKIP BLANK LINES
		par_name = strtok(line, " :");										// " :" as delimiter
		
		if (VERBOSE >= 300)
		{
			printf("par_name= %s \n", par_name);
		}
		
		if (strncmp(par_name, "NAME", 4) == 0)
		{
			token1 = strtok(NULL, " :");			
			strcpy(inst->input_file_name, token1);
			continue;
		}
		
		if (strncmp(par_name, "DIMENSION", 9) == 0)
		{
			token1 = strtok(NULL, " :");									//NULL gives the following word
			inst->nnodes = atoi(token1);									//string argument to integer
			printf("nnodes %d\n", inst->nnodes);
			inst->xcoord = (double *)calloc(inst->nnodes, sizeof(double));
			inst->ycoord = (double *)calloc(inst->nnodes, sizeof(double));
			inst->choosen_edge= (int *)calloc(inst->nnodes*2, sizeof(int));
			inst->comp = (int*)calloc(inst->nnodes, sizeof(int));

			continue;
		}
		
		/*---------------SELECT THE RIGHT TYPE DISTANCE--------------------*/
		if (strncmp(par_name, "EDGE_WEIGHT_TYPE", 16) == 0)
		{
			token1 = strtok(NULL, " :");
			if (strncmp(token1, "EUC_2D", 6) == 0) {						//0 = EUCLIDEAN DISTANCE
				inst->dist_type = 0;
			}
			if (strncmp(token1, "ATT", 3) == 0) {							//1 = ATT DISTANCE
				inst->dist_type = 1;
			}
			if (strncmp(token1, "GEO", 3) == 0) {							//2 = GEO DISTANCE
				inst->dist_type = 2;
			}
			continue;
		}
		if (strncmp(par_name, "NODE_COORD_SECTION", 18) == 0)
		{
			if (inst->nnodes <= 0) { printf(" DIMENSION section should appear before NODE_COORD_SECTION section"); exit(1); }
			coord_section = 1;
			continue;
		}
		if (strncmp(par_name, "EOF", 3) == 0)								//END OF FILE
		{
			coord_section = 0;
			break;
		}
		if (coord_section == 1) {
			int i = atoi(par_name) - 1;										//FIRST COORD INDEX (-1 because indexes start from 0)
			token1 = strtok(NULL, " ");										// x COORDINATE
			token2 = strtok(NULL, " ");										// y COORDINATE
			inst->xcoord[i] = atof(token1);
			inst->ycoord[i] = atof(token2);
			continue;
		}
	}
}