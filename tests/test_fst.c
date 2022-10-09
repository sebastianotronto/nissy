#include "inc.h"
#include "../src/fst.h"

static bool test_cube_to_fst_to_cube(Cube *c);

static bool test_cube_to_fst_to_cube_solved();
static bool test_cube_to_fst_to_cube_unsolved();

static Tester test[] = {
	test_cube_to_fst_to_cube_solved,
	test_cube_to_fst_to_cube_unsolved,
	NULL
};

static char *name[] = {
	"Cube to FST to cube (solved)",
	"Cube to FST to cube (unsolved)",
};

static bool
test_cube_to_fst_to_cube(Cube *c)
{
	Cube d;
	FstCube fst;

	fst = cube_to_fst(c);
	fst_to_cube(fst, &d);

	if (!equal(c, &d)) {
		printf("Cubes are different:\n\n");
		printf("Cube 1:\n");
		print_cube(c);
		printf("\nCube 2:\n");
		print_cube(&d);
		printf("\n");
		return false;
	}

	return true;
}

static bool
test_cube_to_fst_to_cube_solved()
{
	Cube c;

	make_solved(&c);
	return test_cube_to_fst_to_cube(&c);
}

static bool
test_cube_to_fst_to_cube_unsolved()
{
	bool b;
	int i;
	Alg *a;
	Cube c;
	char *algs[] = {
		"U2 R2 U2 R2 U2",
		"U2 F2 R2 B2 U2 D2 F2 L2 B2",
		"RUR'URU2R'",
		"L2 D R U2 B2 L",
		"R'U'F",
		"F2 U' R2 D' B2 D2 R2 D2 R2 U' F L' U' R B F2 R B' D2",
		"D L2 F2 R2 D R2 U L2 U' B2 D L' F2 U2 B' L D' U' R' B2 F2",
		"F' L2 F' D' R F2 L U L' D2 R2 F2 D2 R2 B' L2 B2 U2 F D2 B",
		NULL,
	};

	for (i = 0; algs[i] != NULL; i++) {
		make_solved(&c);
		a = new_alg(algs[i]);
		apply_alg(a, &c);
		b = test_cube_to_fst_to_cube(&c);
		free_alg(a);
		if (!b) {
			printf("Cube to FST to cube failed with alg %s\n",
			    algs[i]);
			return false;
		}
	}
	return true;
}

void test_fst_all() {
	int i;

	init_trans();

	for (i = 0; test[i] != NULL; i++) {
		printf("Test: %s\n", name[i]);
		if (!test[i]()) {
			printf("Failed!\n");
			exit(1);
		}
		printf("Passed.\n\n");
	}
	printf("All FST tests passed.\n\n");
}
