#include "test.h"

static bool run_test(Test *);
static bool run_suite(TestSuite *);

static bool
run_test(Test *test)
{
	int i;

	printf("Running test %s...", test->name);
	for (i = 0; test->cases[i] != NULL; i++) {
		if (!test->t(test->cases[i])) {
			printf("FAILED!\n");
			return false;
		}
	}

	printf("OK\n");
	return true;
}

static bool
run_suite(TestSuite *suite)
{
	int i;

	if (suite->setup != NULL)
		suite->setup();
	for (i = 0; suite->tests[i] != NULL; i++)
		if(!run_test(suite->tests[i]))
			return false;
	if (suite->teardown != NULL)
		suite->teardown();

	return true;
}

int main() {
	init_env();
	init_trans();

	if (!run_suite(&fst_pre_init_suite))
		return 1;
	if (!run_suite(&fst_post_init_suite))
		return 1;

	printf("All tests passed.\n");
	return 0;
}
