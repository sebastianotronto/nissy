#ifndef FST_TESTS_H
#define FST_TESTS_H

#include "../src/fst.h"
#include "test_common.h"

extern char *algs[];

extern Test test_fst_is_consistent;
extern Test test_cube_to_fst_to_cube;
extern Test test_fst_move;
extern Test test_fst_inverse;

extern TestSuite fst_pre_init_suite;
extern TestSuite fst_post_init_suite;

extern TestSuite *fst_testsuites[];

#endif
