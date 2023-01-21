#include "fst_tests.h"

static bool check_equal_and_log(Cube *, Cube *);
static void void_to_cube(void *, Cube *);

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

static bool
check_equal_and_log(Cube *c, Cube *d)
{
	bool ret = equal(c, d);

	if (!ret) {
		printf("\n");
		printf("These cubes should be equal, but are not:\n\n");
		print_cube(c);
		printf("\n");
		print_cube(d);
		printf("\n");
	}

	return ret;
}

static void
void_to_cube(void *a, Cube *c)
{
	char *algstr;
	Alg *alg;

	algstr = (char *)a;
	alg = new_alg(algstr);
	make_solved(c);
	apply_alg(alg, c);
	free_alg(alg);
}

bool
testmethod_fst_is_consistent(void *a)
{
	FstCube fst_uf, fst_fr, fst_rd;
	Cube c, c_fr, c_rd;
	bool consistent_fr, consistent_rd, result;

	void_to_cube(a, &c);
	copy_cube(&c, &c_fr);
	apply_trans(fr, &c_fr);

	copy_cube(&c, &c_rd);
	apply_trans(rd, &c_rd);

	fst_uf = cube_to_fst(&c);
	fst_fr = cube_to_fst(&c_fr);
	fst_rd = cube_to_fst(&c_rd);

	consistent_fr = fst_uf.fr_eofb == fst_fr.uf_eofb &&
			fst_uf.fr_eposepe == fst_fr.uf_eposepe &&
			fst_uf.fr_coud == fst_fr.uf_coud;

	consistent_rd = fst_uf.rd_eofb == fst_rd.uf_eofb &&
			fst_uf.rd_eposepe == fst_rd.uf_eposepe &&
			fst_uf.rd_coud == fst_rd.uf_coud;

	result = consistent_fr && consistent_rd;

	if (!result)
		printf("\nFailed with alg %s\n", (char *)a);

	return result;
}

bool
testmethod_cube_to_fst_to_cube(void *a)
{
	Cube c, d;
	FstCube fst;

	void_to_cube(a, &c);
	fst = cube_to_fst(&c);
	fst_to_cube(fst, &d);

	return check_equal_and_log(&c, &d);;
}

bool
testmethod_fst_move(void *a)
{
	int i;
	Alg *alg;
	Cube c, d;
	FstCube fst;

	void_to_cube(a, &d);
	alg = new_alg((char *)a);
	make_solved(&d);
	fst = cube_to_fst(&d);

	for (i = 0; i < alg->len; i++)
		fst = fst_move(alg->move[i], fst);

	fst_to_cube(fst, &d);

	free_alg(alg);

	return check_equal_and_log(&c, &d);
}

bool
testmethod_fst_inverse(void *a)
{
	Cube c, d;

	void_to_cube(a, &c);
	fst_to_cube(fst_inverse(cube_to_fst(&c)), &d);
	invert_cube(&c);

	return check_equal_and_log(&c, &d);
}
