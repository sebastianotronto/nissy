#define SOLVE_C

#include "solve.h"

/* Local functions ***********************************************************/

static bool        allowed_next(Move move, Step *s, Move l0, Move l1);
static bool        cancel_niss(DfsArg *arg);
static void        copy_dfsarg(DfsArg *src, DfsArg *dst);
static void        dfs(DfsArg *arg);
static void        dfs_add_sol(DfsArg *arg);
static void        dfs_niss(DfsArg *arg);
static bool        dfs_move_checkstop(DfsArg *arg);
static void *      instance_thread(void *arg);
static void        multidfs(DfsArg *arg);
static bool        niss_makes_sense(DfsArg *arg);
static bool        solvestop(int d, int op, SolveOptions *opts, AlgList *sols);

/* Local functions ***********************************************************/

static bool
allowed_next(Move m, Step *s, Move l0, Move l1)
{
	bool allowed, order;
	uint64_t mbit;

	mbit    = ((uint64_t)1) << m;
	allowed = mbit & s->moveset->mask[l1][l0];
	order   = !commute(l0, m) || l0 < m;

	return allowed && order;
}

static bool
cancel_niss(DfsArg *arg)
{
	Moveset *ms;
	Move i1, i2;
	bool p, p1, p2, q, q1, q2;

	if (arg->lastinv[0] == NULLMOVE)
		return false;

	ms = arg->s->moveset;
	i1 = inverse_move(arg->lastinv[0]);
	i2 = inverse_move(arg->lastinv[1]);

	p1 = !ms->allowed_next(arg->last[1], arg->last[0], i1);
	p2 = !ms->allowed_next(arg->last[1], i1, arg->last[0]);
	p = p1 || (commute(i1, arg->last[0]) && p2);

	q1 = !ms->allowed_next(arg->last[1], arg->last[0], i2);
	q2 = !ms->allowed_next(arg->last[1], i2, arg->last[0]);
	q = q1 || (commute(i2, arg->last[0]) && q2);

	return p || (commute(i1, i2) && q);
}

static void
copy_dfsarg(DfsArg *src, DfsArg *dst)
{
	int i;

	dst->cube        = src->cube;
	dst->t           = src->t;
	dst->s           = src->s;
	dst->opts        = src->opts;
	dst->d           = src->d;
	dst->bound       = src->bound; /* In theory not needed */
	dst->niss        = src->niss;
	dst->sols        = src->sols;
	dst->sols_mutex  = src->sols_mutex;
	dst->current_alg = src->current_alg;

	for (i = 0; i < 2; i++) {
		dst->last[i]    = src->last[i];
		dst->lastinv[i] = src->lastinv[i];
	}

	for (i = 0; i < src->s->n_coord; i++) {
		dst->ind[i].val = src->ind[i].val;
		dst->ind[i].t   = src->ind[i].t;
	}

/*
	src->s->copy_extra(src, dst);
*/
}

static void
dfs(DfsArg *arg)
{
	int i;
	Move m;
	DfsArg newarg;

	if (dfs_move_checkstop(arg))
		return;

	if (arg->bound == 0) {
		if (arg->current_alg->len == arg->d)
			dfs_add_sol(arg);
		return;
	}

	for (i = 0; arg->s->moveset->sorted_moves[i] != NULLMOVE; i++) {
		m = arg->s->moveset->sorted_moves[i];
		if (allowed_next(m, arg->s, arg->last[0], arg->last[1])) {
			copy_dfsarg(arg, &newarg);
			newarg.last[1] = arg->last[0];
			newarg.last[0] = m;
			append_move(arg->current_alg, m, newarg.niss);
			dfs(&newarg);
			arg->current_alg->len--;
		}
	}

	if (niss_makes_sense(arg))
		dfs_niss(arg);
}

static void
dfs_add_sol(DfsArg *arg)
{
	bool valid, accepted, nisscanc;

	valid = arg->s->is_valid==NULL || arg->s->is_valid(arg->current_alg);
	accepted = valid || arg->opts->all;
	nisscanc = arg->s->final && cancel_niss(arg);

	if (accepted && !nisscanc) {
		pthread_mutex_lock(arg->sols_mutex);

		if (arg->sols->len < arg->opts->max_solutions) {
			append_alg(arg->sols, arg->current_alg);
			transform_alg(
			    inverse_trans(arg->t), arg->sols->last->alg);
			if (arg->opts->verbose)
				print_alg(arg->sols->last->alg, false);
		}

		pthread_mutex_unlock(arg->sols_mutex);
	}
}

static void
dfs_niss(DfsArg *arg)
{
	DfsArg newarg;
	Alg *inv;
	Cube *c;

	copy_dfsarg(arg, &newarg);

	/* Invert current alg and scramble */
	newarg.cube = malloc(sizeof(Cube));
	inv = inverse_alg(arg->current_alg);
	c = malloc(sizeof(Cube));
	make_solved(newarg.cube);
	apply_alg(inv, newarg.cube);
	copy_cube(arg->cube, c);
	invert_cube(c);
	compose(c, newarg.cube);

	/* New indexes */
	compute_ind(newarg.s, newarg.cube, newarg.ind);

	swapmove(&(newarg.last[0]), &(newarg.lastinv[0]));
	swapmove(&(newarg.last[1]), &(newarg.lastinv[1]));
	newarg.niss = !(arg->niss);

	dfs(&newarg);

	free_alg(inv);
	free(c);
	free(newarg.cube);
}

static bool
dfs_move_checkstop(DfsArg *arg)
{
	int i, goal, nsols;
	Move mm;
	Trans tt = uf; /* Avoid uninitialized warning */

	/* Moving and computing bound */
	arg->bound = 0;
	goal = arg->d - arg->current_alg->len;
	for (i = 0; i < arg->s->n_coord; i++) {
		if (arg->last[0] != NULLMOVE) {
			mm = transform_move(arg->ind[i].t, arg->last[0]);
			arg->ind[i].val = move_coord(arg->s->coord[i],
			    mm, arg->ind[i].val, &tt);
			arg->ind[i].t = transform_trans(tt, arg->ind[i].t);
		}

		arg->bound =
		    MAX(arg->bound, ptableval(arg->s->pd[i], arg->ind[i].val));
		if (arg->opts->can_niss && !arg->niss)
			arg->bound = MIN(1, arg->bound);

		if (arg->bound > goal)
			return true;
	}

	pthread_mutex_lock(arg->sols_mutex);
	nsols = arg->sols->len;
	pthread_mutex_unlock(arg->sols_mutex);

	return nsols >= arg->opts->max_solutions;
}

static void *
instance_thread(void *arg)
{
	bool b, inv;
	Cube c;
	Move m;
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

		inv = node->alg->inv[0];
		m = node->alg->move[0];

		copy_cube(td->arg.cube, &c);
		if (inv)
			invert_cube(&c);

		copy_dfsarg(&td->arg, &darg);
		compute_ind(td->arg.s, &c, darg.ind);
		darg.cube            = &c;

		darg.niss            = inv;
		darg.last[0]         = m;
		darg.last[1]         = NULLMOVE;
		darg.lastinv[0]      = NULLMOVE;
		darg.lastinv[1]      = NULLMOVE;
		darg.current_alg     = new_alg("");
		append_move(darg.current_alg, m, inv);

		dfs(&darg);

		free_alg(darg.current_alg);
	}

	return NULL;
}

static void
multidfs(DfsArg *arg)
{
	int i;
	Cube local_cube;
	Alg *alg;
	AlgList *start;
	AlgListNode **node;
	pthread_t t[arg->opts->nthreads];
	ThreadDataSolve td[arg->opts->nthreads];
	pthread_mutex_t *start_mutex, *sols_mutex;

	node        = malloc(sizeof(AlgListNode *));
	start_mutex = malloc(sizeof(pthread_mutex_t));
	sols_mutex  = malloc(sizeof(pthread_mutex_t));

	start = new_alglist();
	pthread_mutex_init(start_mutex, NULL);
	pthread_mutex_init(sols_mutex,  NULL);

	for (i = 0; arg->s->moveset->sorted_moves[i] != NULLMOVE; i++) {
		alg = new_alg("");
		append_move(alg, arg->s->moveset->sorted_moves[i], false);
		append_alg(start, alg);
		if (arg->opts->can_niss && !arg->s->final) {
			alg->inv[0] = true;
			append_alg(start, alg);
		}
		free_alg(alg);
	}
	*node = start->first;

	copy_cube(arg->cube, &local_cube);

	for (i = 0; i < arg->opts->nthreads; i++) {
		copy_dfsarg(arg, &(td[i].arg));
		td[i].arg.cube       = &local_cube;
		td[i].arg.sols_mutex = sols_mutex;

		td[i].thid           = i;
		td[i].start          = start;
		td[i].node           = node;
		td[i].start_mutex    = start_mutex;

		pthread_create(&t[i], NULL, instance_thread, &td[i]);
	}

	for (i = 0; i < arg->opts->nthreads; i++)
		pthread_join(t[i], NULL);

	free_alglist(start);
	free(node);
	free(start_mutex);
	free(sols_mutex);
}

static bool
niss_makes_sense(DfsArg *arg)
{
	Move m, mm;
	uint64_t u;
	int i;

	if (arg->s->final || arg->niss || !arg->opts->can_niss)
		return false;

	if (arg->last[0] == NULLMOVE)
		return true;

	m = inverse_move(arg->last[0]);
	for (i = 0; i < arg->s->n_coord; i++) {
		mm = transform_move(arg->ind[i].t, m);
		u = move_coord(arg->s->coord[i], mm, 0, NULL);
		if (ptableval(arg->s->pd[i], u) > 0)
			return true;
	}
	
	return false;
}

static bool
solvestop(int d, int op, SolveOptions *opts, AlgList *sols)
{
	bool opt_done, max_moves_exceeded, max_sols_exceeded;

	opt_done = opts->optimal != -1 && op != -1 && d > opts->optimal + op;
	max_moves_exceeded = d > opts->max_moves;
	max_sols_exceeded = sols->len >= opts->max_solutions;

	return opt_done || max_moves_exceeded || max_sols_exceeded;
}

/* Public functions **********************************************************/

AlgList *
solve(Cube *cube, ChoiceStep *cs, SolveOptions *opts)
{
	int i, j, d, op, est;
	bool ready[99], one_ready, zerosol;
	Movable ind[99][10];
	AlgList *s;
	Cube *c[99];
	DfsArg arg[99];

	prepare_cs(cs, opts);
	s = new_alglist();

	for (i = 0, one_ready = false; cs->step[i] != NULL; i++) {
		c[i] = malloc(sizeof(Cube));
		copy_cube(cube, c[i]);
		apply_trans(cs->t[i], c[i]);

		arg[i].cube = c[i];
		arg[i].t    = cs->t[i];
		arg[i].s    = cs->step[i];
		arg[i].opts = opts;
		arg[i].sols = s;

		if ((ready[i] = cs->step[i]->ready(c[i]))) {
			one_ready = true;
			/* Only for local use for 0 moves solutions */
			compute_ind(cs->step[i], c[i], ind[i]);
		}
	}
	if (!one_ready) {
		fprintf(stderr, "Cube not ready for solving step: ");
		fprintf(stderr, "%s\n", cs->ready_msg);
		return s;
	}

	/* If the empty moves sequence is a solution for one of the
	 * alternatives, all longer solutions will be discarded, so we may
	 * just set its ready[] value to false. If the solution is accepted
	 * we append it and start searching from d = 1. */
	for (i = 0, zerosol = false; cs->step[i] != NULL; i++) {
		if (ready[i]) {
			est = 0;
			for (j = 0; j < cs->step[i]->n_coord; j++)
				est = MAX(est, ptableval(cs->step[i]->pd[j],
				    ind[i][j].val));
			if (est == 0) {
				ready[i] = false;
				zerosol = true;
			}
		}
	}
	if (zerosol && opts->min_moves == 0) {
		append_alg(s, new_alg(""));
		opts->min_moves = 1;
		if (opts->verbose)
			printf("Step is already solved"
			    "(empty alg is a solution)\n");
	}

	for (d = opts->min_moves, op = -1; !solvestop(d, op, opts, s); d++) {
		if (opts->verbose)
			fprintf(stderr, "Searching depth %d\n", d);

		for (i=0; cs->step[i]!=NULL && !solvestop(d,op,opts,s); i++) {
			if (!ready[i])
				continue;

			arg[i].d = d;
			multidfs(&arg[i]);

			if (s->len > 0 && op == -1)
				op = d;
		}
	}

	for (i = 0; cs->step[i] != NULL; i++)
		free(c[i]);

	return s;
}

/* TODO: make more general! */
Alg *
solve_2phase(Cube *cube, int nthreads)
{
	int bestlen, newb;
	Alg *bestalg, *ret;
	AlgList *sols1, *sols2;
	AlgListNode *i;
	Cube c;
	SolveOptions opts1, opts2;

	opts1.min_moves     = 0;
	opts1.max_moves     = 13;
	opts1.max_solutions = 20;
	opts1.nthreads      = nthreads;
	opts1.optimal       = 3;
	opts1.can_niss      = false;
	opts1.verbose       = false;
	opts1.all           = true;

	opts2.min_moves     = 0;
	opts2.max_moves     = 19;
	opts2.max_solutions = 1;
	opts2.nthreads      = nthreads;
	opts2.can_niss      = false;
	opts2.verbose       = false;

	/* We skip step1 if it is solved on U/D */
	if (check_drud(cube)) {
		sols1 = new_alglist();
		append_alg(sols1, new_alg(""));
	} else {
		sols1 = solve(cube, &drud_HTM, &opts1);
	}
	bestalg = new_alg("");
	bestlen = 999;
	for (i = sols1->first; i != NULL; i = i->next) {
		copy_cube(cube, &c);
		apply_alg(i->alg, &c);
		sols2 = solve(&c, &dranyfin_DR, &opts2);
		
		if (sols2->len > 0) {
			newb = i->alg->len + sols2->first->alg->len;
			if (newb < bestlen) {
				bestlen = newb;
				copy_alg(i->alg, bestalg);
				compose_alg(bestalg, sols2->first->alg);
			}
		}

		free_alglist(sols2);
	}

	free_alglist(sols1);

	ret = cleanup(bestalg);
	free_alg(bestalg);

	return ret;
}
