#include "alg_tests.h"

bool testmethod_append_move(void *);
bool testmethod_new_alg(void *);
/*
bool testmethod_compose_alg(void *);
bool testmethod_inverse_alg(void *);
bool testmethod_on_inverse(void *);
bool testmethod_unniss(void *);
*/

typedef struct {
	Move *move;
	bool *inv;
	int len;
	Move m;
	bool inverse;
} append_move_t;

Move m_app1[] = {F, x, D3};
bool i_app1[] = {true, false, false};
append_move_t append_move_case1 = {
	.move = m_app1,
	.inv = i_app1,
	.len = 3,
	.m = L3,
	.inverse = false,
};

Move m_app2[] = {S, U, x2, M2};
bool i_app2[] = {true, false, true, true};
append_move_t append_move_case2 = {
	.move = m_app2,
	.inv = i_app2,
	.len = 4,
	.m = R,
	.inverse = true,
};

Move m_app3[] = {U, U, U, U, U};
bool i_app3[] = {false, false, false, false, false};
append_move_t append_move_case3 = {
	.move = m_app3,
	.inv = i_app3,
	.len = 5,
	.m = U,
	.inverse = false,
};

append_move_t *append_move_cases[] = {
	&append_move_case1,
	&append_move_case2,
	&append_move_case3,
};

Test test_append_move = {
	.name  = "Appending a move to and alg",
	.t     = testmethod_append_move,
	.cases = (void **)append_move_cases,
};

typedef struct {
	char *str;
	Move *move;
	bool *inv;
	int len;
	Move *move_normal;
	int len_normal;
	Move *move_inverse;
	int len_inverse;
} new_alg_t;

/* Alg F U B' (L3 D) x M (S) y3 */
Move m_new1[] = {F, U, B3, L3, D, x, M, S, y3};
Move mn_new1[] = {F, U, B3, x, M, y3};
Move mi_new1[] = {L3, D, S};
bool i_new1[] = {false, false, false, true, true, false, false, true, false};
new_alg_t new_alg_case1 = {
	.str = "F U B' (L3 D) x M (S) y3",
	.move = m_new1,
	.inv = i_new1,
	.len = 9,
	.move_normal = mn_new1,
	.len_normal = 6,
	.move_inverse = mi_new1,
	.len_inverse = 3,
};
new_alg_t *new_alg_cases[] = {&new_alg_case1};

Test test_new_alg = {
	.name  = "Initializing an alg from a string",
	.t     = testmethod_new_alg,
	.cases = (void **)new_alg_cases,
};

Test *alg_all_tests[] = {
	&test_append_move,
	&test_new_alg,
/*
	&test_append_alg,
	&test_compose_alg,
	&test_inverse_alg,
	&test_on_inverse,
	&test_unniss,
*/
	NULL
};
TestSuite alg_suite = {
	.setup    = NULL,
	.tests    = alg_all_tests,
	.teardown = NULL,
};

TestSuite *alg_suites[] = {
	&alg_suite,
	NULL
};

bool
testmethod_append_move(void *a)
{
	append_move_t *b = (append_move_t *)a;
	int li, ln;
	Alg *alg;

	/* Small to test reallocation */
	alg = malloc(sizeof(Alg));
	alg->allocated    = 5;
	alg->move         = malloc(alg->allocated * sizeof(Move));
	alg->inv          = malloc(alg->allocated * sizeof(bool));
	alg->len          = b->len;
	memcpy(alg->move, b->move, alg->len * sizeof(Move));
	memcpy(alg->inv,  b->inv,  alg->len * sizeof(bool));
	alg->move_normal  = malloc(alg->allocated * sizeof(Move));
	alg->move_inverse = malloc(alg->allocated * sizeof(Move));

	li = ln = 0;
	for (int i = 0; i < alg->len; i++) {
		if (alg->inv[i])
			alg->move_inverse[li++] = alg->move[i];
		else
			alg->move_normal[ln++] = alg->move[i];
	}
	alg->len_inverse = li;
	alg->len_normal = ln;

	append_move(alg, b->m, b->inverse);

	if (alg->len != b->len + 1) {
		printf("Alg has wrong len (%d instead of %d)\n",
			alg->len, b->len + 1);
		goto append_move_fail;
	}
	if (alg->move[alg->len-1] != b->m) {
		printf("Wrong last move (%s instead of %s)\n",
			move_string(alg->move[alg->len-1]),
			move_string(b->m));
		goto append_move_fail;
	}
	if (alg->inv[alg->len-1] != b->inverse) {
		printf("Wrong inverse flag for last move "
			"(%s instead of %s)\n",
			b->inverse ? "normal" : "inverse",
			b->inverse ? "inverse" : "normal");
		goto append_move_fail;
	}
	if (b->inverse) {
		if (alg->len_inverse != li + 1 ||
		    alg->len_normal  != ln) {
			printf("%d moves on normal (should be %d)"
			       " and %d on inverse (should be %d)\n",
			       alg->len_normal, ln,
			       alg->len_inverse, li + 1);
			goto append_move_fail;
		}
		if (alg->move_inverse[alg->len_inverse-1] != b->m) {
			printf("Wrong move on inverse (%s instead of %s)\n",
				move_string(alg->move_inverse[alg->len-1]),
				move_string(b->m));
			goto append_move_fail;
		}
	} else {
		if (alg->len_inverse != li ||
		    alg->len_normal  != ln + 1) {
			printf("%d moves on normal (should be %d)"
			       " and %d on inverse (should be %d)\n",
			       alg->len_normal, ln,
			       alg->len_inverse, li + 1);
			goto append_move_fail;
		}
		if (alg->move_normal[alg->len_normal-1] != b->m) {
			printf("Wrong move on normal (%s instead of %s)\n",
				move_string(alg->move_normal[alg->len-1]),
				move_string(b->m));
			goto append_move_fail;
		}
	}

	free(alg);
	return true;

append_move_fail:
	free(alg);
	return false;
}

bool
testmethod_new_alg(void *a)
{
	new_alg_t *b = (new_alg_t *)a;
	Alg *alg = new_alg(b->str);

	if (alg->len != b->len) {
		printf("Algs have different length (%d instead of %d)\n",
			alg->len, b->len);
		goto new_alg_fail;
	}

	for (int i = 0; i < alg->len; i++) {
		if (alg->move[i] != b->move[i] || alg->inv[i] != b->inv[i]) {
			printf("Algs differ on move %d\n", i);
			printf("Expected: %s\nActual:   ", b->str);
			print_alg(alg, false);
			goto new_alg_fail;
		}
	}

	if (alg->len_normal != b->len_normal) {
		printf("Algs have different number of moves on normal"
			" (%d instead of %d)\n",
			alg->len_normal, b->len_normal);
		goto new_alg_fail;
	}
	for (int i = 0; i < alg->len_normal; i++) {
		if (alg->move_normal[i] != b->move_normal[i]) {
			printf("Algs have different move %d on normal"
				" (%s instead of %s)\n", i,
				move_string(alg->move_normal[i]),
				move_string(b->move_normal[i]));
			goto new_alg_fail;
		}
	}

	if (alg->len_inverse != b->len_inverse) {
		printf("Algs have different number of moves on inverse"
			" (%d instead of %d)\n",
			alg->len_inverse, b->len_inverse);
		goto new_alg_fail;
	}
	for (int i = 0; i < alg->len_inverse; i++) {
		if (alg->move_inverse[i] != b->move_inverse[i]) {
			printf("Algs have different move %d on inverse:\n"
				" (%s instead of %s)\n", i,
				move_string(alg->move_inverse[i]),
				move_string(b->move_inverse[i]));
			goto new_alg_fail;
		}
	}

	free(alg);
	return true;

new_alg_fail:
	free(alg);
	return false;
}
