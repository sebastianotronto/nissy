#include "solve.h"

/* Local functions ***********************************************************/

static bool        allowed_next(Move move, DfsData *dd);
static void        dfs(Cube c, Step *s, SolveOptions *opts, DfsData *dd);
static void        dfs_branch(Cube c, Step *s, SolveOptions *os, DfsData *dd);
static bool        dfs_check_solved(Step *s, SolveOptions *opts, DfsData *dd);
static void        dfs_niss(Cube c, Step *s, SolveOptions *opts, DfsData *dd);
static bool        dfs_stop(Cube c, Step *s, SolveOptions *opts, DfsData *dd);
static void *      instance_thread(void *arg);
static void        multidfs(Cube c, Step *s, SolveOptions *opts, AlgList *sols, int d);

/* Local functions ***********************************************************/

static bool
allowed_next(Move move, DfsData *dd)
{
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
	bool b = false;
	int i;
	Move m, l1, l2;

	l1 = dd->last1;
	l2 = dd->last2;

	for (i = 0; dd->sorted_moves[i] != NULLMOVE; i++) {
	/*
		pthread_mutex_lock(dd->sols_mutex);
		b = dd->sols->len >= opts->max_solutions;
		pthread_mutex_unlock(dd->sols_mutex);
	*/

		if (b)
			break;
			
		m = dd->sorted_moves[i];
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
		if (s->is_valid(dd->current_alg) || opts->all) {
			pthread_mutex_lock(dd->sols_mutex);
			if (dd->sols->len < opts->max_solutions)
				append_alg(dd->sols, dd->current_alg);
			pthread_mutex_unlock(dd->sols_mutex);
		}

		if (opts->verbose)
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
	bool b = false;

	CubeTarget ct = {
		.cube   = c,
		.target = dd->d - dd->current_alg->len
	};

	dd->lb = s->estimate(ct);
	if (opts->can_niss && !dd->niss)
		dd->lb = MIN(1, dd->lb);

	if (dd->current_alg->len + dd->lb > dd->d)
		return true;

	pthread_mutex_lock(dd->sols_mutex);
	b = dd->sols->len >= opts->max_solutions;
	pthread_mutex_unlock(dd->sols_mutex);

	return b;
}

static void *
instance_thread(void *arg)
{
	bool b;
	Cube c;
	ThreadData *td;
	AlgListNode *node;
	DfsData dd;

	td = (ThreadData *)arg;

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

		dd.d             = td->depth;
		dd.m             = 1;
		dd.niss          = node->alg->inv[0];
		dd.lb            = -1;
		dd.last1         = node->alg->move[0];
		dd.last2         = NULLMOVE;
		dd.sols          = td->sols;
		dd.sols_mutex    = td->sols_mutex;
		dd.current_alg   = new_alg("");
		append_move(dd.current_alg, node->alg->move[0],
		            node->alg->inv[0]);
		dd.sorted_moves  = td->sorted_moves;
		dd.move_position = td->move_position;

/*
		pthread_mutex_lock(td->sols_mutex);
		printf("Starting thread %d with move: ", td->thid);
		print_alg(dd.current_alg, false);
		pthread_mutex_unlock(td->sols_mutex);
*/

		dfs(c, td->step, td->opts, &dd);

		free_alg(dd.current_alg);
	}

	return NULL;
}

static void
multidfs(Cube c, Step *s, SolveOptions *opts, AlgList *sols, int d)
{
	int i, *move_position;
	Move *sorted_moves;
	Alg *alg;
	AlgList *start;
	AlgListNode **node;
	pthread_t t[opts->nthreads];
	ThreadData td[opts->nthreads];
	pthread_mutex_t *start_mutex, *sols_mutex;

	move_position = malloc(NMOVES * sizeof(int));
	sorted_moves  = malloc(NMOVES * sizeof(Move));
	node  = malloc(sizeof(AlgListNode *));
	start_mutex = malloc(sizeof(pthread_mutex_t));
	sols_mutex  = malloc(sizeof(pthread_mutex_t));

	start = new_alglist();
	pthread_mutex_init(start_mutex, NULL);
	pthread_mutex_init(sols_mutex,  NULL);

	moveset_to_list(s->moveset, sorted_moves);
	movelist_to_position(sorted_moves, move_position);
	for (i = 0; sorted_moves[i] != NULLMOVE; i++) {
		alg = new_alg("");
		append_move(alg, sorted_moves[i], false);
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
		td[i].sorted_moves  = sorted_moves;
		td[i].move_position = move_position;
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
	free(move_position);
	free(sorted_moves);
}

/* Public functions **********************************************************/

AlgList *
solve(Cube cube, Step *step, SolveOptions *opts)
{
	int d;
	AlgList *sols = new_alglist();
	AlgListNode *node;
	Cube c;

	prepare_step(step);

	if (step->detect != NULL)
		step->pre_trans = step->detect(cube);
	c = apply_trans(step->pre_trans, cube);

	if (step->ready != NULL && !step->ready(c)) {
		fprintf(stderr, "Cube not ready for solving step: ");
		fprintf(stderr, "%s\n", step->ready_msg);
		return sols;
	}

	if (step->estimate((CubeTarget){.cube = c, .target = 0}) == 0 &&
	    opts->min_moves == 0) {
		append_alg(sols, new_alg(""));
		return sols;
	}

	for (d = MAX(1, opts->min_moves);
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

	for (node = sols->first; node != NULL; node = node->next)
		transform_alg(inverse_trans(step->pre_trans), node->alg);

	return sols;
}
