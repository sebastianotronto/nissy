#include "solve.h"

/* Local functions ***********************************************************/

static bool        allowed_next(Move move, DfsData *dd);
static void        dfs(Cube c, Step *s, SolveOptions *opts, DfsData *dd);
static void        dfs_branch(Cube c, Step *s, SolveOptions *os, DfsData *dd);
static bool        dfs_check_solved(Step *s, SolveOptions *opts, DfsData *dd);
static void        dfs_niss(Cube c, Step *s, SolveOptions *opts, DfsData *dd);
static bool        dfs_stop(Cube c, Step *s, SolveOptions *opts, DfsData *dd);

/* Local functions ***********************************************************/

static bool
allowed_next(Move move, DfsData *dd)
{

/* TODO: remove the commented part, was added to moves.c 
	static bool initialized = false;
	static bool commute[NMOVES][NMOVES], pnext[NMOVES][NMOVES][NMOVES];

	if (!initialized) {
		Cube c1, c2;
		int i, j, k;
		bool p1, p2, cij;

		for (i = 0; i < NMOVES; i++) {
			for (j = 0; j < NMOVES; j++) {
				c1 = apply_move(i, apply_move(j, (Cube){0}));
				c2 = apply_move(j, apply_move(i, (Cube){0}));
				commute[i][j] = equal(c1, c2) && i && j;
				for (k = 0; k < NMOVES; k++) {
					p1 = j && base_move(j) == base_move(k);
					p2 = i && base_move(i) == base_move(k);
					cij = commute[i][j];
					pnext[i][j][k] = !(p1 || (cij && p2));
				}
			}
		}

		initialized = true;
	}

	if (!pnext[dd->last2][dd->last1][move])
		return false;

	if (commute[dd->last1][move])
		return dd->move_position[dd->last1] < dd->move_position[move];

	return true;
*/

	if (!possible_next(dd->last2, dd->last1, move))
		return false;

	if (commute(dd->last1, move))
		return dd->move_position[dd->last1] < dd->move_position[move];

	return true;
}

static void
dfs(Cube c, Step *s, SolveOptions *opts, DfsData *dd)
{
	if (dfs_stop(c, s, opts, dd))
		return;

	if (dfs_check_solved(s, opts, dd))
		return;

	dfs_branch(c, s, opts, dd);

	if (opts->can_niss && !dd->niss)
		dfs_niss(c, s, opts, dd);
}

static void
dfs_branch(Cube c, Step *s, SolveOptions *opts, DfsData *dd)
{
	Move m, l1 = dd->last1, l2 = dd->last2, *moves = dd->sorted_moves;

	int i, maxnsol = opts->max_solutions;

	for (i = 0; moves[i] != NULLMOVE && dd->sols->len < maxnsol; i++) {
		m = moves[i];
		if (allowed_next(m, dd)) {
			dd->last2 = dd->last1;
			dd->last1 = m;
			append_move(dd->current_alg, m, dd->niss);

			dfs(apply_move(m, c), s, opts, dd);

			dd->current_alg->len--;
			dd->last2 = l2;
			dd->last1 = l1;
		}
	}
}

static bool
dfs_check_solved(Step *s, SolveOptions *opts, DfsData *dd)
{
	if (dd->lb != 0)
		return false;

	if (dd->current_alg->len == dd->d) {
		if (s->is_valid(dd->current_alg) || opts->all)
			append_alg(dd->sols, dd->current_alg);

		if (opts->feedback)
			print_alg(dd->current_alg, false);
	}

	return true;
}

static void
dfs_niss(Cube c, Step *s, SolveOptions *opts, DfsData *dd)
{
	Move l1 = dd->last1, l2 = dd->last2;
	CubeTarget ct;

	ct.cube = apply_move(inverse_move(l1), (Cube){0});
	ct.target = 1;

	if (dd->current_alg->len == 0 || s->estimate(ct)) {
		dd->niss  = true;
		dd->last1 = NULLMOVE;
		dd->last2 = NULLMOVE;

		dfs(inverse_cube(c), s, opts, dd);

		dd->last1 = l1;
		dd->last2 = l2;
		dd->niss  = false;
	}
}

static bool
dfs_stop(Cube c, Step *s, SolveOptions *opts, DfsData *dd)
{
	CubeTarget ct = {
		.cube   = c,
		.target = dd->d - dd->current_alg->len
	};

	if (dd->sols->len >= opts->max_solutions)
		return true;

	dd->lb = s->estimate(ct);
	if (opts->can_niss && !dd->niss)
		dd->lb = MIN(1, dd->lb);

	if (dd->current_alg->len + dd->lb > dd->d)
		return true;

	return false;
}

/* Public functions **********************************************************/

AlgList *
solve(Cube cube, Step *step, SolveOptions *opts)
{
	AlgListNode *node;
	AlgList *sols = new_alglist();
	Cube c;

	if (step->detect != NULL)
		step->pre_trans = step->detect(cube);
	c = apply_trans(step->pre_trans, cube);

	DfsData dd = {
		.m           = 0,
		.niss        = false,
		.lb          = -1,
		.last1       = NULLMOVE,
		.last2       = NULLMOVE,
		.sols        = sols,
		.current_alg = new_alg("")
	};

	if (step->ready != NULL && !step->ready(c)) {
		fprintf(stderr, "Cube not ready for solving step: ");
		fprintf(stderr, "%s\n", step->ready_msg);
		return sols;
	}

	moveset_to_list(step->moveset, dd.sorted_moves);
	movelist_to_position(dd.sorted_moves, dd.move_position);

	for (dd.d = opts->min_moves;
	     dd.d <= opts->max_moves &&
	         !(sols->len && opts->optimal_only) &&
		 sols->len < opts->max_solutions;
	     dd.d++) {
		if (opts->feedback)
			fprintf(stderr,
				"Found %d solutions, searching depth %d...\n",
				sols->len, dd.d);
		dfs(c, step, opts, &dd);
	}

	for (node = sols->first; node != NULL; node = node->next)
		transform_alg(inverse_trans(step->pre_trans), node->alg);

	free_alg(dd.current_alg);
	return sols;
}
