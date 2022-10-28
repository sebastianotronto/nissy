#include "inc.h"
#include "../src/fst.h"

static bool cube_to_fst_to_cube(Cube *c, Alg *a);
static bool fst_consistent(Cube *c, Alg *a);
static bool fst_move_test(Cube *c, Alg *a);
static bool fst_inverse_test(Cube *c, Alg *a);
static bool try_str(CubeTester f, char *algstr, char *msg);
static bool try_all_str(CubeTester f, char *msg);

static bool test_fst_consistent_algs();
static bool test_cube_to_fst_to_cube_algs();
static bool test_fst_move_algs();
static bool test_fst_inverse_algs();

static Tester test[] = {
	test_fst_consistent_algs,
	test_cube_to_fst_to_cube_algs,
	test_fst_move_algs,
	test_fst_inverse_algs,
	NULL
};

static char *name[] = {
	"Consistency of FST (converted from cube)",
	"Cube to FST to cube",
	"FST move",
	"FST inverse",
};

static char *algs[] = {
	"",
	"U", "U2", "U'", "D", "D2", "D'", "R", "R2", "R'",
	"L", "L2", "L'", "F", "F2", "F'", "B", "B2", "B'",
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

static bool
fst_consistent(Cube *c, Alg *a)
{
	FstCube fst;

	fst = cube_to_fst(c);

	/* TODO: check consistency of fr_* and rd_* with uf_* */

	return true;
}

static bool
cube_to_fst_to_cube(Cube *c, Alg *a)
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
fst_move_test(Cube *c, Alg *a)
{
	int i;
	Cube d;
	FstCube fst;

	fst = cube_to_fst(c);

	for (i = 0; i < a->len; i++) {
		if (a->inv[i] || a->move[i] > B3) {
			printf("Cannot apply the following alg to FST: ");
			print_alg(a, false);
			return false;
		}
		fst = fst_move(a->move[i], fst);
	}

	fst_to_cube(fst, &d);

	return equal(c, &d);
}

static bool
fst_inverse_test(Cube *c, Alg *a)
{
	Cube d;

	fst_to_cube(fst_inverse(cube_to_fst(c)), &d);
	invert_cube(c);

	return equal(c, &d);
}

static bool
try_str(CubeTester f, char *algstr, char *msg)
{
	bool b;
	Alg *a;
	Cube c;

	a = new_alg(algstr);
	make_solved(&c);
	apply_alg(a, &c);

	if (!(b = f(&c, a)))
		printf("%s with alg %s\n", msg, algstr);

	free_alg(a);

	return b;
}

static bool
try_all_str(CubeTester f, char *msg)
{
	bool b;
	int i;

	b = true;
	for (i = 0; algs[i] != NULL; i++)
		b = b && try_str(f, algs[i], msg);

	return b;
}

static bool
test_cube_to_fst_to_cube_algs()
{
	return try_all_str(cube_to_fst_to_cube, "Cube to FST to cube failed");
}

static bool
test_fst_consistent_algs()
{
	return try_all_str(fst_consistent, "FST from cube not consistent");
}

static bool
test_fst_move_algs()
{
	return try_all_str(fst_move_test, "FST move incorrect");
}

static bool
test_fst_inverse_algs()
{
	return try_all_str(fst_inverse_test, "FST test incorrect");
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
