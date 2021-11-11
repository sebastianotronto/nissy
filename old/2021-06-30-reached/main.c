#include <stdio.h>
#include "cube.h"
#include "steps.h"

int main() {
	Alg *algo;
	AlgList *sols;
	Cube cube;
	SolveOptions opts;
	char line[1000];
	int i, ns = 2;
/*	Move m;*/
/*	int nrand = 10000, sum1, sum2, sum3;*/

	Step *stps[20]    = {&drud_HTM, &optimal_HTM};
	char sss[30][30] = {"DR on UD", "Optimal solve"};

	opts = (SolveOptions) {
		.min_moves     = 0,
		.max_moves     = 20,
		.optimal_only  = true,
		.max_solutions = 1,
		.can_niss      = false,
		.feedback      = true,
	};

	init();

/*
	srand(time(NULL));
	sum1 = 0;
	sum2 = 0;
	sum3 = 0;
	for (i = 0; i < nrand; i++) {
		cube = random_cube();
		sum1 += drud_HTM.check(cube, 20);
		sum2 += optimal_HTM.check(cube, 20);
		sum3 += cornershtreofb_HTM.check(cube, 20);
	}
	printf("Average drud pruning: %lf\n", ((double)sum1) / ((double) nrand));
	printf("Average corners htr pruning: %lf\n", ((double)sum2) / ((double) nrand));
	printf("Average corners htr + eofb pruning: %lf\n", ((double)sum3) / ((double) nrand));
*/

/*
	for (m = U; m <= B3; m++) {
		printf("Class eofbepos after %d: ", m);
		printf("%lu\n", coord_khuge.index(apply_move(m,(Cube){0})));
	}
*/

	printf("Welcome to nissy 2.0! Insert a scramble:\n");

	if (fgets(line, 1000, stdin) != NULL) {
		algo = new_alg(line);
		cube = apply_alg(algo, (Cube){0});

		print_alg(inverse_alg(algo), false);
/*
		printf("After rb_mirror:\n");
		cube = apply_trans(rb_mirror, cube);
		print_cube(cube);
		printf("Going back:\n");
		cube = apply_trans(inverse_trans(rb_mirror), cube);
*/

		print_cube(cube);

		for (i = 0; i < ns; i++) {
			sols = solve(cube, *stps[i], &opts);
			printf("%s: %d solutions found:\n", sss[i], sols->len);
			print_alglist(sols, true);
			free_alglist(sols);
		}
		free_alg(algo);
	}

	return 0;
}

