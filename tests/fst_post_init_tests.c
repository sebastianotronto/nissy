#include "fst_test_util.h"

static bool fst_move_testcase(Cube *c, Alg *a);
static bool fst_inverse_testcase(Cube *c, Alg *a);

static bool fst_move_test();
static bool fst_inverse_test();

static Tester test[] = {
	fst_move_test,
	fst_inverse_test,
	NULL
};

static char *name[] = {
	"FST move",
	"FST inverse",
};

static bool
fst_move_testcase(Cube *c, Alg *a)
{
	int i;
	Cube d;
	FstCube fst;

	make_solved(&d);
	fst = cube_to_fst(&d);

	for (i = 0; i < a->len; i++) {
		if (a->inv[i] || a->move[i] > B3) {
			printf("Cannot apply the following alg to FST: ");
			print_alg(a, false);
			return false;
		}
		fst = fst_move(a->move[i], fst);
	}

	fst_to_cube(fst, &d);

	return equal_and_log(c, &d);
}

static bool
fst_inverse_testcase(Cube *c, Alg *a)
{
	Cube d;

	fst_to_cube(fst_inverse(cube_to_fst(c)), &d);
	invert_cube(c);

	return equal_and_log(c, &d);
}

static bool
fst_move_test()
{
	return try_all_str(fst_move_testcase, "FST move incorrect");
}

static bool
fst_inverse_test()
{
	return try_all_str(fst_inverse_testcase, "FST test incorrect");
}

void fst_post_init_testall() {
	int i;

	init_fst();

	for (i = 0; test[i] != NULL; i++) {
		printf("Test: %s\n", name[i]);
		if (!test[i]()) {
			printf("Failed!\n");
			exit(1);
		}
		printf("Passed.\n\n");
	}
	printf("All FST post-init tests passed.\n\n");
}
