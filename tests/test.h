#ifndef TEST_H
#define TEST_H

typedef void (*VoidMethod)(void);
typedef bool (*TestMethod)(void *);

typedef struct {
	char *     name;
	TestMethod t;
	void **    cases;
} Test;

typedef struct {
	VoidMethod setup;
	Test **    tests;
	VoidMethod teardown;
} TestSuite;

#endif
