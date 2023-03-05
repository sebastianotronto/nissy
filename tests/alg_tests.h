#ifndef ALG_TESTS_H
#define ALG_TESTS_H

#include "../src/alg.h"
#include "test_common.h"

extern Test test_append_move;
extern Test test_new_alg;
/*
extern Test remove_last_move;
extern Test test_compose_alg;
extern Test test_inverse_alg;
extern Test test_on_inverse;
extern Test test_unniss;
*/

extern TestSuite alg_suite;

extern TestSuite *alg_suites[];

#endif
