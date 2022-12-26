#include "fst_test_util.h"

char *algs[] = {
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

bool
equal_and_log(Cube *c, Cube *d)
{
	bool ret = equal(c, d);

	if (!ret) {
		printf("These cubes should be equal, but are not:\n\n");
		print_cube(c);
		printf("\n");
		print_cube(d);
		printf("\n");
	}

	return ret;
}

bool
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

bool
try_all_str(CubeTester f, char *msg)
{
	bool b;
	int i;

	b = true;
	for (i = 0; algs[i] != NULL; i++)
		b = b && try_str(f, algs[i], msg);

	return b;
}
