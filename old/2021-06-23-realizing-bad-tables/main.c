#include <stdio.h>
#include "cube.h"
#include "steps.h"

int main() {
	Alg *algo;
	AlgList *sols;
	Cube cube;
	SolveOptions opts;
	char line[1000];
	int i, ns = 3;
/*	int nrand = 10000, sum1, sum2, sum3;*/

	Step *stps[20]    = {&corners_HTM, &eofb_HTM, &optimal_HTM};
	char sss[30][30] = {"Corners", "EO on F/B", "Optimal solution"};

	opts = (SolveOptions) {
		.min_moves     = 0,
		.max_moves     = 20,
		.optimal_only  = true,
		.max_solutions = 1,
		.can_niss      = false,
		.feedback      = true,
		.pre_trans     = uf
	};

	init();

	print_ptable(&pd_corners_HTM);

/*
	srand(time(NULL));
	sum1 = 0;
	sum2 = 0;
	sum3 = 0;
	for (i = 0; i < nrand; i++) {
		cube = random_cube();
		sum1 += corners_HTM.check(cube, 20);
		sum2 += cornershtr_HTM.check(cube, 20);
		sum3 += optimal_HTM.check(cube, 20);
	}
	printf("Average corners pruning: %lf\n", ((double)sum1) / ((double) nrand));
	printf("Average corners htr pruning: %lf\n", ((double)sum2) / ((double) nrand));
	printf("Average optimal pruning: %lf\n", ((double)sum3) / ((double) nrand));
*/

	printf("Welcome to nissy 2.0! Insert a scramble:\n");

	if (fgets(line, 1000, stdin) != NULL) {
		algo = new_alg(line);
		cube = apply_alg(algo, (Cube){0});

		print_cube(cube);
		printf("Index: %lu\n", pd_cornershtr_HTM.index(cube));

		for (i = 0; i < ns; i++) {
			if (stps[i]->check == NULL)
				fprintf(stderr, "Check function for step %d is null\n", i);
			printf("Check: %d\n", stps[i]->check(cube, 20));
			sols = solve(cube, *stps[i], &opts);
			printf("%s: %d solutions found:\n", sss[i], sols->len);
			print_alglist(sols, true);
			free_alglist(sols);
		}
		free_alg(algo);
	}

	return 0;
}

