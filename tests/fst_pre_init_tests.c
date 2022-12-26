#include "fst_test_util.h"

static bool cube_to_fst_to_cube_testcase(Cube *c, Alg *a);
static bool fst_is_consistent_testcase(Cube *c, Alg *a);

static bool fst_is_consistent_test();
static bool cube_to_fst_to_cube_test();

static Tester test[] = {
	fst_is_consistent_test,
	cube_to_fst_to_cube_test,
	NULL
};

static char *name[] = {
	"Consistency of FST (converted from cube)",
	"Cube to FST to cube",
};

static bool
fst_is_consistent_testcase(Cube *c, Alg *a)
{
	FstCube fst_uf, fst_fr, fst_rd;
	Cube c_fr, c_rd;
	bool consistent_fr, consistent_rd;

	copy_cube(c, &c_fr);
	apply_trans(fr, &c_fr);

	copy_cube(c, &c_rd);
	apply_trans(rd, &c_rd);

	fst_uf = cube_to_fst(c);
	fst_fr = cube_to_fst(&c_fr);
	fst_rd = cube_to_fst(&c_rd);

	consistent_fr = fst_uf.fr_eofb == fst_fr.uf_eofb &&
			fst_uf.fr_eposepe == fst_fr.uf_eposepe &&
			fst_uf.fr_coud == fst_fr.uf_coud;

	consistent_rd = fst_uf.rd_eofb == fst_rd.uf_eofb &&
			fst_uf.rd_eposepe == fst_rd.uf_eposepe &&
			fst_uf.rd_coud == fst_rd.uf_coud;

	return consistent_fr && consistent_rd;
}

static bool
cube_to_fst_to_cube_testcase(Cube *c, Alg *a)
{
	Cube d;
	FstCube fst;

	fst = cube_to_fst(c);
	fst_to_cube(fst, &d);

	return equal_and_log(c, &d);;
}

static bool
cube_to_fst_to_cube_test()
{
	return try_all_str(
	    cube_to_fst_to_cube_testcase, "Cube to FST to cube failed");
}

static bool
fst_is_consistent_test()
{
	return try_all_str(
	    fst_is_consistent_testcase, "FST from cube not consistent");
}

void fst_pre_init_testall() {
	int i;

	init_env();
	init_trans();

	for (i = 0; test[i] != NULL; i++) {
		printf("Test: %s\n", name[i]);
		if (!test[i]()) {
			printf("Failed!\n");
			exit(1);
		}
		printf("Passed.\n\n");
	}
	printf("All FST pre-init tests passed.\n\n");
}
