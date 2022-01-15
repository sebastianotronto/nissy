#include "commands.h"

/* Some of the following functions are new, some are copied from steps.c */
bool
allowed(Move m)
{
	return base_move(m) == U || m == R2 || m == F2;
}

bool
allowed_next(Move l2, Move l1, Move m)
{
	return base_move(m) != base_move(l1);
}

bool
check_cornershtr(Cube c)
{
	return coord_cornershtr.index(c) == 0;
}

bool
check_coud_and_dbl(Cube c)
{
	return c.coud == 0 && what_corner_at(c, DBL) == DBL;
}

static int
estimate_cornershtr_HTM(DfsArg *arg)
{
	return ptableval(&pd_cornershtr_HTM, arg->cube);
}

static bool
validate_singlecw_ending(Alg *alg)
{
	int i;
	bool nor, inv;
	Move l2 = NULLMOVE, l1 = NULLMOVE, l2i = NULLMOVE, l1i = NULLMOVE;

	for (i = 0; i < alg->len; i++) {
		if (alg->inv[i]) {
			l2i = l1i;
			l1i = alg->move[i];
		} else {
			l2 = l1;
			l1 = alg->move[i];
		}
	}

	nor = l1 ==base_move(l1)  && (!commute(l1, l2) ||l2 ==base_move(l2));
	inv = l1i==base_move(l1i) && (!commute(l1i,l2i)||l2i==base_move(l2i));

	return nor && inv;
}

int
main()
{
	Moveset moveset_UR2F2 = {
		.allowed      = allowed,
		.allowed_next = allowed_next,
	};

	init_moveset(&moveset_UR2F2);

	/*
	 * This step is the same as cornershtr_HTM in steps.c, except for
	 * the ready() function (which is not relevant anyway, it is there
	 * more for testing than anything else) and the moveset.
	 */
	Step step = {
		.final    = false,
		.is_done  = check_cornershtr,
		.estimate = estimate_cornershtr_HTM,
		.ready    = check_coud_and_dbl,
		.is_valid = validate_singlecw_ending,
		.moveset  = &moveset_UR2F2,

		.pre_trans = uf,

		.tables   = {&pd_cornershtr_HTM},
		.ntables  = 1,
	};

	SolveOptions opts = {
		.min_moves     = 0,
		.max_moves     = 20,
		.max_solutions = 1,
		.nthreads      = 4,
		.optimal       = 0,
		.can_niss      = false,
		.verbose       = false,
		.all           = false,
		.print_number  = false,
		.count_only    = false
	};

	init_symcoord();

	bool cphtr_state_done[BINOM8ON4*6];
	for (unsigned long int i = 0; i < BINOM8ON4*6; i++)
		cphtr_state_done[i] = false;

	for (unsigned long int i = 0; i < FACTORIAL8; i++) {
		AlgList *sols;
		Cube c = {0};
		c.cp = i; /* inconsistent state because of side CO */

		if (what_corner_at(c, DBL) != DBL ||
		    cphtr_state_done[coord_cphtr.index(c)])
			continue;

		fprintf(stderr, "Doing cp %ld (cphtr state %ld)\n",
				i, coord_cphtr.index(c));

		cphtr_state_done[coord_cphtr.index(c)] = true;
		/* Comment next two lines to get non-reduced list */
		Cube mirror = apply_trans(ur_mirror, c);
		cphtr_state_done[coord_cphtr.index(mirror)] = true;

		sols = solve(c, &step, &opts);
		printf("%.2d\t", sols->first->alg->len);
		print_alglist(sols, opts.print_number);
	}

	return 0;
}
