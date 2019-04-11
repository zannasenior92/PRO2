/*-----------------------------FUNCTION FOR PRINT MAIN DATA ON FILE (USEFUL FOR )-----------------------------*/

#include "TSP.h"

void print_main_data(instance *inst) {

	FILE * main_data = fopen("data_instance.txt", "w");


	fprintf(main_data,"STAT,%s,%s",inst->input_file_name);


	fclose(main_data);
}

