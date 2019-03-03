int main(int argc) {
	//
}
//-------------------------------------------------------------------------------
/*PROGRAMMARE IN MANIERA PULITA
 Creo un file di intestazione
unica struttura dati (instance) in cui creiamo una variabile inst che è un contenitore dentro al quale ci sono tutti
che definiscono la nostra istanza che contiene l'array con i nostri dati(ad esempio in modalità array)
Se vogliamo un timelimit (parametro) possiamo metterlo nell'istanza insieme ai dati di input
dentro i file .h vengono memorizzate le quantità globali che devono essere note a tutto il programma
 #include = include delle librerie standard che il programma userà.Ad esempio #include <stdio.h>
 #include <cplex.h> per usare cplex
 #define X  1e-9 in questo modo basta chiamare X dove la voglio usare
 definisco un nuovo tipo di dati  typedef struct{} istanza in cui definisco i campi che ci sono all'interno dell'istanza
 ci devono essere sia gli input che gli output
-non usare nomi troppo corti per le variabili.
 nell'istanza mi serve n, coordinata x , coordinata y
 double *xcoord rappresenta un vettore con coordinate x
double timelimit = tempo massimo di esecuzione del mio programma
 int model_type = per usare uno tra i modelli da utilizzare
 double best_solv = il valore ottimo trovato
 FILE MAIN = programma principale lanciato
 definisci un'istanza inst di tipo instance. instance inst = sto allocando una variabile inst di tipo insctance vuota
 (è come se stessi facendo "int a" solo che ho più campi perchè l'ho definita io nel file .h)
 devo però includere il file .h attraverso "#include file.h"
 funzione command line per leggere i parametri
 funzione read_input per riempire la mia istanza usando il puntatore &. read_input(&inst)
 prima che finisca la funzione main devo svuotare la memoria che ho utilizzato. Devo liberarla da tutti i vettori x, y
 per farlo free_instance(%inst) uso questa funzione
 Prendere i tempi di calcolo delle varie parti del programma. attraverso la funzione di c (cercare su internet)
 printf per stampare quello che mi interessa. Se non voglio vederle, ovvero per ridurle. Devo quindi avere un parametro
 VERBOSE che mi dice il numero di stampe che vengono fuori. quindi uso if (VERBOSE >100) printf ("Qui ho fatto questo")
 */
 /*
Il programma creerà un file eseguibile che poi viene lanciato. Magari devo comunicargli qualcosa
(ad esempio il nome del file).
per farlo posso usare la funzione parse_command_line. Posso prenderla selezionando solo le cose che mi servono.
int argc = quanti argomenti ho passato. Viene passato in un array di stringhe char** argv. La funzione
riceve anche l'istanza "instance *inst" (l'asterisco serve come puntatore)
Prendere -file e -timelimit
*/

/*
Se l'utente si dimentica un parametro ad esempio il timefault. Quindi definisco dei valori di default all'inizio del
void_parse_command_line.
"inst->timelimit" individua il campo dell'istanza nel main
NULL per default.
Poi la funzione fa un ciclo in cui controllo i parametri che ho dato da riga di comando.
funzione di C per comparare due elementi = strcmp(,el1,el2). Se trovo il parametro allora incremento l'argv facendo
argv[++i] e la copio dentro l'istanza inst con "inst->argv[++i]"
atof = serve per convertire un alfanumerico in un floatingpoint (cioè da stringa a valore double)
*/
/*
Finita la lettura stampa a video quello che ho letto. Se in un campo non ho inserito il parametro verrà fuori il
parametro di default.
*/

/*
PER LEGGERE L'INPUT
Leggiamo il file di testo riga per riga.
Poi usiamo una funzione per separare le parole della riga (uso la funzione token).
Apre il file in modalità "r" ovvero lettura.
Per vedere se mi manca qualcosa uso il -1 per segnare che quell'elemento non è stato preso.
fgets = funzione che prende il file e prende la riga corrente e la memorizza su char line[180];
Se la lunghezza della riga è <= 1 allora prendi la riga dopo.
strtok = funzione in cui gli passo una linea e gli specifico quali sono i separatori per riconoscere le parole diverse.
Ci possono essere più separatori.
Su par_name ci memorizzo il token.
funzione strncmp = se il parametro coincide con "NAME" allora sta iniziando la regione NOME.Quindi vai avanti
active_section = flag che serve per vedere in che sezione sono.
Quando so la lunghezza dei vettori allora gli alloco.
Quando è iniziata la sezione coordinate metto il flag active_section =1 e inizia a leggere.
*/

/*
Per provare il programma prendere un file piccolo dalla TSPlib
Inoltre vorrò crearmi una procedura di stampa che stampa a video la mia istanza.
Per stampare a video usare libreria gnuplot.
*/