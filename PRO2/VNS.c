/*------------------------VNS-------------------------*/
#include "TSP.h"
#include <ilcplex/cplex.h>
#include <time.h>


void VNS(instance *inst,CPXENVptr env, CPXLPptr lp, double opt_current, double min_cost)
{
	time_t timelimit = time(NULL) + 40;
	double delta, delta_vns;
	double *min_solution = (double*)calloc(inst->ncols, sizeof(double));
	double opt = opt_current;
	double min = min_cost;

	while (time(NULL) < timelimit) {

		delta = two_opt(inst, env, lp);
		printf("Delta: %f\n", delta);
		opt += delta;
		printf("New objective function: %f\n", opt);

		if (delta == 0.0) {

			if (opt < min) {
				min_solution = (double*)calloc(inst->ncols, sizeof(double));

				for (int k = 0; k < inst->ncols; k++) {
					min_solution[k] = inst->best_sol[k];

				}
				min = opt;
			}
			int done = 0;
			while (done == 0) {
				delta_vns = vns(inst, env, lp);
				if (delta_vns == -1) continue;
				else {
					printf("DELTA VNS=%f\n", delta_vns);
					opt += delta_vns;
					done = 1;
				}
			}

			printf("----------------------------------CALCIO 3-OPT Opt current=%f\n", opt);
		}
	}

	if (opt < min) {
		printf("Best Object function founded %f\n", opt);
	}
	else {
		printf("Best Object function founded %f\n", min);
		for (int k = 0; k < inst->ncols; k++) {
			inst->best_sol[k] = min_solution[k];
		}
	}

	free(min_solution);
}
