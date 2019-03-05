#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define VERBOSE 50

typedef struct {
	int nnodes;
	double *xcoord;
	double *ycoord;
	double timelimit;
	char input_file[1000];
} instance;//"instance" sarebbe il nome che diamo alla nostra struttura

/*Per denotare una struttura si usa la parola chiave struct seguita dal 
nome identificativo della struttura, che è opzionale.(Potrebbe tornare utile in futuro)

Per definire nuovi tipi di dato viene utilizzata la funzione typedef. L'uso
di typedef combinato con struct ci permette di creare 
tipi di dati molto complessi
*/