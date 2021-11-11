#include <stdio.h>
#include "cube.h"
#include "steps.h"

int main() {
	Alg algo;
	AlgList *sols;
	Cube cube;
	SolveOptions opts;

	init();

	algo = new_alg("R U R' F");
	cube = apply_alg(algo, (Cube){0});

	print_cube(apply_trans(rd, cube));
	
	opts = (SolveOptions) {
		.min_moves     = 0,
		.max_moves     = 5,
		.optimal_only  = true,
		.max_solutions = 3,
		.can_niss      = false,
		.moveset       = standard_moveset,
		.pre_trans     = fu
	};
	sols = solve(cube, step_eofb, opts);
	if (sols->len == 0)
		fprintf(stderr, "No solution found\n");
	else
		print_alg(sols->first->alg);


	return 0;
}
