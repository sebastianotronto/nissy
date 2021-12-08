#include "solve.h"

/* Local functions ***********************************************************/

static bool        allowed_next(Move move, DfsData *dd, uint64_t mm);
static void        dfs(Cube c, Step *s, SolveOptions *opts, DfsData *dd);
static void        dfs_branch(Cube c, Step *s, SolveOptions *os, DfsData *dd);
static bool        dfs_check_solved(Step *s, SolveOptions *opts, DfsData *dd);
static void        dfs_niss(Cube c, Step *s, SolveOptions *opts, DfsData *dd);
static bool        dfs_stop(Cube c, Step *s, SolveOptions *opts, DfsData *dd);
static void *      instance_thread(void *arg);
static void        multidfs(Cube c, Step *s, SolveOptions *opts, AlgList *sols, int d);

/* Local functions ***********************************************************/

static bool
allowed_next(Move move, DfsData *dd, uint64_t mm)
{
	if ((1 << move) & mm)
		return false;

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
	uint64_t mm;
	Move m, l1, l2;
	LocalInfo li;

	l1 = dd->last1;
	l2 = dd->last2;
	li = *(dd->ed->li);
	mm = dd->ed->movebitmask;

	for (i = 0; dd->sorted_moves[i] != NULLMOVE; i++) {
		if (b)
			break;
			
		m = dd->sorted_moves[i];
		if (allowed_next(m, dd, mm)) {
			dd->last2 = dd->last1;
			dd->last1 = m;
			append_move(dd->current_alg, m, dd->niss);

			dfs(apply_move(m, c), s, opts, dd);

			dd->current_alg->len--;
			dd->last2     = l2;
			dd->last1     = l1;
			*(dd->ed->li) = li;
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
	Move l1, l2;
	EstimateData *ed;

	l1 = dd->last1;
	l2 = dd->last2;

	ed = malloc(sizeof(EstimateData));
	ed->cube = apply_move(inverse_move(l1), (Cube){0});
	ed->target = 1;

	if (dd->current_alg->len == 0 || s->estimate(ed)) {
		dd->niss  = true;
		dd->last1 = NULLMOVE;
		dd->last2 = NULLMOVE;

		dfs(inverse_cube(c), s, opts, dd);

		dd->last1 = l1;
		dd->last2 = l2;
		dd->niss  = false;
	}

	free(ed);
}

static bool
dfs_stop(Cube c, Step *s, SolveOptions *opts, DfsData *dd)
{
	bool b;

	dd->ed->cube        = c;
	dd->ed->target      = dd->d - dd->current_alg->len;
	dd->ed->lastmove    = dd->last1;
	dd->ed->movebitmask = 0;

	dd->lb = s->estimate(dd->ed);
	if (opts->can_niss && !dd->niss)
		dd->lb = MIN(1, dd->lb);

	if (dd->current_alg->len + dd->lb > dd->d) {
		 b = true;
	} else {
		pthread_mutex_lock(dd->sols_mutex);
		b = dd->sols->len >= opts->max_solutions;
		pthread_mutex_unlock(dd->sols_mutex);
	}

	return b;
}

static void *
instance_thread(void *arg)
{
	bool b;
	Cube c;
	ThreadDataSolve *td;
	AlgListNode *node;
	DfsData dd;

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

		dd.d               = td->depth;
		dd.m               = 1;
		dd.niss            = node->alg->inv[0];
		dd.lb              = -1;
		dd.last1           = node->alg->move[0];
		dd.last2           = NULLMOVE;
		dd.sols            = td->sols;
		dd.sols_mutex      = td->sols_mutex;
		dd.current_alg     = new_alg("");
		append_move(dd.current_alg, node->alg->move[0],
		            node->alg->inv[0]);
		dd.sorted_moves    = td->sorted_moves;
		dd.move_position   = td->move_position;
		dd.ed              = malloc(sizeof(EstimateData));
		dd.ed->movebitmask = 0;
		dd.ed->li          = new_localinfo();

		dfs(c, td->step, td->opts, &dd);

		free_alg(dd.current_alg);
		free_localinfo(dd.ed->li);
		free(dd.ed);
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
	ThreadDataSolve td[opts->nthreads];
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
	AlgList *sols;
	AlgListNode *node;
	Cube c;
	EstimateData *ed;
	bool b;

	prepare_step(step, opts->nthreads);

	if (step->detect != NULL)
		step->pre_trans = step->detect(cube);
	c = apply_trans(step->pre_trans, cube);

	sols = new_alglist();

	if (step->ready != NULL && !step->ready(c)) {
		fprintf(stderr, "Cube not ready for solving step: ");
		fprintf(stderr, "%s\n", step->ready_msg);
		return sols;
	}

	if (opts->min_moves == 0) {
		ed = malloc(sizeof(EstimateData));
		ed->cube = cube;
		ed->target = 0;
		ed->li = new_localinfo();
		b = step->estimate(ed) == 0;
		free_localinfo(ed->li);
		free(ed);

		if (b) {
			append_alg(sols, new_alg(""));
			return sols;
		}
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

	for (node = sols->first; node != NULL; node = node->next)
		transform_alg(inverse_trans(step->pre_trans), node->alg);

	return sols;
}
