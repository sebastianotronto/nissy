#include <stdio.h>

#include "fst_tests.h"

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

static bool
module_in_args(char *module, int argc, char *argv[])
{
	for (int i = 0; i < argc; i++)
		if (!strcmp(module, argv[i]))
			return true;
	return false;
}

int main(int argc, char *argv[]) {
	/* TODO: init should be in testsuites */
	init_env();
	init_trans();
	/**************************************/

	TestModule fst = { .name = "fst", .suites = fst_testsuites };
	TestModule *modules[999] = {
		&fst,
		NULL
	};


	bool all = argc == 1 || module_in_args("all", argc, argv);
	int count = 0;
	for (int i = 0; modules[i] != NULL; i++) {
		if (all || module_in_args(modules[i]->name, argc, argv)) {
			for (int j = 0; modules[i]->suites[j] != NULL; j++) {
				if (!run_suite(modules[i]->suites[j])) {
					return 1;
				} else {
					count++;
				}
			}
		}
	}

	printf("All tests passed (%d test suites).\n", count);
	return 0;
}
