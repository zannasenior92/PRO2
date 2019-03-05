#include "TSP.h" //per includere il file .h nel main

int main(int argc, char **argv) {
	printf("Ci sono %d elementi.\n", argc);
	for (int i = 0; i < argc; i++) {
		printf(argv[i]);
		printf("\n");
	}
	return 0;
}