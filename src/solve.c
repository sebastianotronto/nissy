#include "solve.h"

/* Local functions ***********************************************************/

static bool        allowed_next(Move move, DfsArg *arg);
static bool        cancel_niss(DfsArg *arg);
static void        copy_dfsarg(DfsArg *src, DfsArg *dst);
static void        dfs(DfsArg *arg);
static void        dfs_branch(DfsArg *arg);
static bool        dfs_check_solved(DfsArg *arg);
static bool        dfs_switch(DfsArg *arg);
static void        dfs_niss(DfsArg *arg);
static bool        dfs_stop(DfsArg *arg);
static void *      instance_thread(void *arg);
static void        invert_branch(DfsArg *arg);
static void        multidfs(Cube c, Step *s, SolveOptions *opts, AlgList *sols, int d);
static bool        niss_makes_sense(DfsArg *arg);

/* Local functions ***********************************************************/

static bool
allowed_next(Move m, DfsArg *arg)
{
	bool bad, allowed, order;
	uint64_t mbit;

	mbit    = ((uint64_t)1) << m;
	bad     = mbit & arg->badmoves;
	allowed = mbit & arg->step->moveset->mask[arg->last2][arg->last1];
	order   = !commute(arg->last1, m) || arg->last1 < m;

	return allowed && !bad && order;
}

static bool
cancel_niss(DfsArg *arg)
{
	Moveset *ms;
	Move i1, i2;
	bool p, p1, p2, q, q1, q2;

	if (arg->last1inv == NULLMOVE)
		return false;

	ms = arg->step->moveset;
	i1 = inverse_move(arg->last1inv);
	i2 = inverse_move(arg->last2inv);

	p1 = !ms->allowed_next(arg->last2, arg->last1, i1);
	p2 = !ms->allowed_next(arg->last2, i1, arg->last1);
	p = p1 || (commute(i1, arg->last1) && p2);

	q1 = !ms->allowed_next(arg->last2, arg->last1, i2);
	q2 = !ms->allowed_next(arg->last2, i2, arg->last1);
	q = q1 || (commute(i2, arg->last1) && q2);

	return p || (commute(i1, i2) && q);
}

static void
copy_dfsarg(DfsArg *src, DfsArg *dst)
{
	dst->step          = src->step;
	dst->opts          = src->opts;
	dst->cube          = src->cube;
	dst->inverse       = src->inverse;
	dst->d             = src->d;
	dst->badmoves      = src->badmoves;
	dst->badmovesinv   = src->badmovesinv;
	dst->niss          = src->niss;
	dst->last1         = src->last1;
	dst->last2         = src->last2;
	dst->last1inv      = src->last1inv;
	dst->last2inv      = src->last2inv;
	dst->sols          = src->sols;
	dst->sols_mutex    = src->sols_mutex;
	dst->current_alg   = src->current_alg;

	copy_estimatedata(src->ed, dst->ed);
}

static void
dfs(DfsArg *arg)
{
	bool sw = false;

	if (dfs_stop(arg))
		return;

	if (dfs_check_solved(arg))
		return;

	if (arg->step->final && (sw = dfs_switch(arg)))
		invert_branch(arg);
	dfs_branch(arg);

	if (arg->opts->can_niss && !arg->niss && niss_makes_sense(arg))
		dfs_niss(arg);

	if (sw)
		invert_branch(arg);
}

static void
dfs_branch(DfsArg *arg)
{
	int i;
	Move m;
	DfsArg *newarg;

	newarg = malloc(sizeof(DfsArg));
	newarg->ed = malloc(sizeof(EstimateData));

	for (i = 0; arg->step->moveset->sorted_moves[i] != NULLMOVE; i++) {
		m = arg->step->moveset->sorted_moves[i];
		if (allowed_next(m, arg)) {
			copy_dfsarg(arg, newarg);
			newarg->last2 = arg->last1;
			newarg->last1 = m;
			newarg->cube  = apply_move(m, arg->cube);
			append_move(arg->current_alg, m, newarg->niss);

			dfs(newarg);

			arg->current_alg->len--;
		}
	}

	free(newarg->ed);
	free(newarg);
}

static bool
dfs_check_solved(DfsArg *arg)
{
	if (!arg->step->is_done(arg->cube))
		return false;

	if (arg->current_alg->len == arg->d) {
		if ((arg->step->is_valid(arg->current_alg) || arg->opts->all)
		    && (!arg->step->final || !cancel_niss(arg))) {

			pthread_mutex_lock(arg->sols_mutex);

			if (arg->sols->len < arg->opts->max_solutions) {
				append_alg(arg->sols, arg->current_alg);

				transform_alg(
				    inverse_trans(arg->step->pre_trans), 
				    arg->sols->last->alg
				);
				if (arg->step->final)
					unniss(arg->sols->last->alg);

				if (arg->opts->verbose)
					print_alg(arg->sols->last->alg, false);
			}

			pthread_mutex_unlock(arg->sols_mutex);
		}
	}

	return true;
}

static void
dfs_niss(DfsArg *arg)
{
	DfsArg *newarg;

	newarg = malloc(sizeof(DfsArg));
	newarg->ed = malloc(sizeof(EstimateData));

	copy_dfsarg(arg, newarg);
	swapmove(&(newarg->last1), &(newarg->last1inv));
	swapmove(&(newarg->last2), &(newarg->last2inv));
	newarg->niss = !(arg->niss);
	newarg->cube = inverse_cube(arg->cube);

	dfs(newarg);

	free(newarg->ed);
	free(newarg);
}

static bool
dfs_stop(DfsArg *arg)
{
	int lowerbound;
	bool b;

	lowerbound = arg->step->estimate(arg);
	if (arg->opts->can_niss && !arg->niss)
		lowerbound = MIN(1, lowerbound);

	if (arg->current_alg->len + lowerbound > arg->d) {
		 b = true;
	} else {
		pthread_mutex_lock(arg->sols_mutex);
		b = arg->sols->len >= arg->opts->max_solutions;
		pthread_mutex_unlock(arg->sols_mutex);
	}

	return b;
}

static bool
dfs_switch(DfsArg *arg)
{
	int i, bn, bi;

	bn = 0;
	for (i = 0; arg->step->moveset->sorted_moves[i] != NULLMOVE; i++)
		if (allowed_next(arg->step->moveset->sorted_moves[i], arg))
			bn++;
	
	swapmove(&(arg->last1), &(arg->last1inv));
	swapmove(&(arg->last2), &(arg->last2inv));
	swapu64(&(arg->badmoves), &(arg->badmovesinv));

	bi = 0;
	for (i = 0; arg->step->moveset->sorted_moves[i] != NULLMOVE; i++)
		if (allowed_next(arg->step->moveset->sorted_moves[i], arg))
			bi++;
	
	swapmove(&(arg->last1), &(arg->last1inv));
	swapmove(&(arg->last2), &(arg->last2inv));
	swapu64(&(arg->badmoves), &(arg->badmovesinv));

	return bi < bn;
}

static void *
instance_thread(void *arg)
{
	bool b;
	Cube c;
	ThreadDataSolve *td;
	AlgListNode *node;
	DfsArg darg;

	td = (ThreadDataSolve *)arg;

	while (1) {
		b = false;

		pthread_mutex_lock(td->start_mutex);
		if ((node = *(td->node)) == NULL)
			b = true;
		else
			*(td->node) = (*(td->node))->next;
		pthread_mutex_unlock(td->start_mutex);

		if (b)
			break;

		c = node->alg->inv[0] ?
		    apply_move(node->alg->move[0], inverse_cube(td->cube)) :
		    apply_move(node->alg->move[0], td->cube);

		darg.step            = td->step;
		darg.opts            = td->opts;
		darg.cube            = c;
		darg.d               = td->depth;
		darg.niss            = node->alg->inv[0];
		darg.last1           = node->alg->move[0];
		darg.last2           = NULLMOVE;
		darg.last1inv        = NULLMOVE;
		darg.last2inv        = NULLMOVE;
		darg.sols            = td->sols;
		darg.sols_mutex      = td->sols_mutex;
		darg.current_alg     = new_alg("");
		append_move(darg.current_alg, node->alg->move[0],
		            node->alg->inv[0]);
		darg.ed              = malloc(sizeof(EstimateData));
		reset_estimatedata(darg.ed);
		darg.badmoves        = 0;
		darg.badmovesinv     = 0;

		dfs(&darg);

		free_alg(darg.current_alg);
		free(darg.ed);
	}

	return NULL;
}

static void
invert_branch(DfsArg *arg)
{
	Cube aux;

	aux = arg->cube;
	arg->cube = is_solved(arg->inverse) ?
	            inverse_cube(arg->cube) : arg->inverse;
	arg->inverse = aux;

	swapu64(&(arg->badmoves), &(arg->badmovesinv));
	arg->niss = !(arg->niss);
	swapmove(&(arg->last1), &(arg->last1inv));
	swapmove(&(arg->last2), &(arg->last2inv));
	invert_estimatedata(arg->ed);
}

static void
multidfs(Cube c, Step *s, SolveOptions *opts, AlgList *sols, int d)
{
	int i;
	Alg *alg;
	AlgList *start;
	AlgListNode **node;
	pthread_t t[opts->nthreads];
	ThreadDataSolve td[opts->nthreads];
	pthread_mutex_t *start_mutex, *sols_mutex;

	node  = malloc(sizeof(AlgListNode *));
	start_mutex = malloc(sizeof(pthread_mutex_t));
	sols_mutex  = malloc(sizeof(pthread_mutex_t));

	start = new_alglist();
	pthread_mutex_init(start_mutex, NULL);
	pthread_mutex_init(sols_mutex,  NULL);

	for (i = 0; s->moveset->sorted_moves[i] != NULLMOVE; i++) {
		alg = new_alg("");
		/* TODO: start on inverse also in case of final step
		   and ed->sw true */
		append_move(alg, s->moveset->sorted_moves[i], false);
		append_alg(start, alg);
		if (opts->can_niss) {
			alg->inv[0] = true;
			append_alg(start, alg);
		}
		free_alg(alg);
	}
	*node = start->first;

	for (i = 0; i < opts->nthreads; i++) {
		td[i].thid          = i;
		td[i].cube          = c;
		td[i].step          = s;
		td[i].depth         = d;
		td[i].opts          = opts;
		td[i].start         = start;
		td[i].node          = node;
		td[i].sols          = sols;
		td[i].start_mutex   = start_mutex;
		td[i].sols_mutex    = sols_mutex;
		pthread_create(&t[i], NULL, instance_thread, &td[i]);
	}

	for (i = 0; i < opts->nthreads; i++)
		pthread_join(t[i], NULL);

	free_alglist(start);
	free(node);
	free(start_mutex);
	free(sols_mutex);
}

static bool
niss_makes_sense(DfsArg *arg)
{
	Cube testcube;

	testcube = apply_move(inverse_move(arg->last1), (Cube){0});
	return arg->current_alg->len == 0 || arg->step->is_done(testcube);
}

/* Public functions **********************************************************/

AlgList *
solve(Cube cube, Step *step, SolveOptions *opts)
{
	int d;
	AlgList *sols;
	Cube c;

	prepare_step(step, opts);

	if (step->detect != NULL)
		step->pre_trans = step->detect(cube);
	c = apply_trans(step->pre_trans, cube);

	sols = new_alglist();

	if (step->ready != NULL && !step->ready(c)) {
		fprintf(stderr, "Cube not ready for solving step: ");
		fprintf(stderr, "%s\n", step->ready_msg);
		return sols;
	}

	if (opts->min_moves == 0 && step->is_done(cube)) {
		append_alg(sols, new_alg(""));
		return sols;
	}

	for (d = opts->min_moves;
	     d <= opts->max_moves &&
	         !(sols->len && opts->optimal_only) &&
		 sols->len < opts->max_solutions;
	     d++) {
		if (opts->verbose)
			fprintf(stderr,
				"Found %d solutions, searching depth %d...\n",
				sols->len, d);
		multidfs(c, step, opts, sols, d);
	}

	return sols;
}
