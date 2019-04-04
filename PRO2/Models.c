#include "TSP.h"

/*METODO CHE CONTROLLA IL TIPO DI MODELLO E COSTRUISCE QUELLO RELATIVO*/
void select_and_build_model(instance *inst, CPXENVptr env, CPXLPptr lp) {
	if (inst->model_type == 0) {
		printf("Modello TSP normale\n");
		build_model(inst, env, lp);
	}
	if (inst->model_type == 1) {
		printf("Modello Flow1\n");
		inst->compact = 1;
		build_modelFlow1(inst, env, lp);
	}
	if (inst->model_type == 2) {
		printf("Modello MTZ\n");
		inst->compact = 1;
		build_modelMTZ(inst, env, lp);
	}
	if (inst->model_type == 3) {
		printf("Modello Fischetti\n");
		build_modelFischetti(inst, env, lp);
	}

}
