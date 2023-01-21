#ifndef FST_TESTS_H
#define FST_TESTS_H

#include "../src/fst.h"
#include "test.h"

bool testmethod_fst_is_consistent(void *);
bool testmethod_cube_to_fst_to_cube(void *);
bool testmethod_fst_move(void *);
bool testmethod_fst_inverse(void *);

extern char *algs[];

Test test_fst_is_consistent = {
	.name  = "Consitency of FST (converted from cube)",
	.t     = testmethod_fst_is_consistent,
	.cases = (void **)algs,
};
Test test_cube_to_fst_to_cube = {
	.name  = "Cube to FST to cube",
	.t     = testmethod_cube_to_fst_to_cube,
	.cases = (void **)algs,
};
Test test_fst_move = {
	.name  = "FST move",
	.t     = testmethod_fst_move,
	.cases = (void **)algs,
};
Test test_fst_inverse = {
	.name  = "FST inverse",
	.t     = testmethod_fst_inverse,
	.cases = (void **)algs,
};

Test *pre_init[] = {
	&test_fst_is_consistent,
	&test_cube_to_fst_to_cube,
	NULL
};
TestSuite fst_pre_init_suite = {
	.setup    = NULL,
	.tests    = pre_init,
	.teardown = NULL,
};

Test *post_init[] = {
	&test_fst_move,
	&test_fst_inverse,
	NULL
};
TestSuite fst_post_init_suite = {
	.setup    = init_fst,
	.tests    = post_init,
	.teardown = NULL,
};

#endif
