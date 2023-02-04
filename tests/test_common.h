#ifndef TEST_COMMON_H
#define TEST_COMMON_H

/*
 * Common utilities for testing.
 * A VoidMethod can be used as a setup or teardown method for testing, see
 * the TestSuite struct. A TestMethod takes a void pointer (usually cast
 * to some data to be used for testing, but can also be ignored) and returns
 * a bool: true for pass, false for fail.
 * A Test consists of a name (string), a TestMethod and an array of void
 * pointers that describe the test cases. Each element of this array is
 * passed to the test method sequentially, and the test session stops on
 * the first failure.
 * A test suite is just a list of tests with a setup and a teardown method.
 * The setup method is run once and for all before the first test, and the
 * teardown is run at the end of the testsuite.
 * Finally, a TestModule roughly corresponds to a test file. This type is
 * only used in test.c to collect multiple test modules.
 */

typedef void (*VoidMethod)(void);
typedef bool (*TestMethod)(void *);

typedef struct {
	char *          name;
	TestMethod      t;
	void **         cases;
} Test;

typedef struct {
	VoidMethod      setup;
	Test **         tests;
	VoidMethod      teardown;
} TestSuite;

typedef struct {
	char *          name;
	TestSuite **    suites;
} TestModule;

#endif
