#include <pthread.h>
#include "threader_eager.h"

typedef struct {
	AlgList *          sols;
	pthread_mutex_t *  sols_mutex;
} ThreadData;

typedef struct {
	DfsArg *           arg;
	Solver *           solver;
	Threader *         threader;
	AlgList *          starts;
	AlgListNode **      node;
	pthread_mutex_t *  start_mutex;
} ThreadInitData;

static void             append_sol(Alg *, void *);
static void *           instance_thread(void *);
static void             dispatch(DfsArg *, AlgList *, Solver *, Threader *);
static AlgList *        possible_starts(DfsArg *, Solver *);
static int              get_nsol(void *);

Threader threader_eager = {
	.append_sol      = append_sol,
	.dispatch        = dispatch,
	.get_nsol        = get_nsol,
};

static void
append_sol(Alg *alg, void *threaddata)
{
	ThreadData *td = (ThreadData *)threaddata;

	pthread_mutex_lock(td->sols_mutex);
	append_alg(td->sols, alg);
	pthread_mutex_unlock(td->sols_mutex);
}

static AlgList *
possible_starts(DfsArg *arg, Solver *solver)
{
	AlgList *ret = new_alglist();

	if (solver->is_solved(solver->param, arg->cubedata)) {
		if (arg->opts->min_moves == 0 && arg->d == 0)
			append_sol(new_alg(""), arg->threaddata);
		return ret;
	}

	for (int i = 0; solver->moveset->sorted_moves[i] != NULLMOVE; i++) {
		Move m = solver->moveset->sorted_moves[i];
		Alg *alg = new_alg("");
		append_move(alg, m, false);
		append_alg(ret, alg);
		free_alg(alg);

/* TODO: check if step not final */
		if (arg->opts->can_niss) {
			alg = new_alg("");
			append_move(alg, m, true);
			append_alg(ret, alg);
			free_alg(alg);
		}
	}

	return ret;
}

static void *
instance_thread(void *arg)
{
	ThreadInitData *tid = (ThreadInitData *)arg;

	while (true) {
		pthread_mutex_lock(tid->start_mutex);
		AlgListNode *node = *(tid->node);
		if (node == NULL) {
			pthread_mutex_unlock(tid->start_mutex);
			break;
		}
		*(tid->node) = (*(tid->node))->next;
		pthread_mutex_unlock(tid->start_mutex);

/* TODO: adjust for longer (arbitrarily long?) starting sequences */
		void *data = tid->solver->alloc_cubedata(tid->solver->param);
		tid->solver->copy_cubedata(
			tid->solver->param, tid->arg->cubedata, data);
		bool inv = node->alg->inv[node->alg->len-1];
		if (inv)
			tid->solver->invert_cube(
				tid->solver->param, data);
		tid->solver->apply_move(
			tid->solver->param, data, node->alg->move[0]);

		DfsArg newarg;
		newarg.cubedata    = data;
		newarg.threaddata  = tid->arg->threaddata;
		newarg.opts        = tid->arg->opts;
		newarg.d           = tid->arg->d;
		newarg.niss        = inv;
		newarg.current_alg = new_alg("");
		copy_alg(node->alg, newarg.current_alg);

		dfs(&newarg, tid->solver, tid->threader);

		tid->solver->free_cubedata(tid->solver->param, data);
		free_alg(newarg.current_alg);
	}

	return NULL;
}

static void
dispatch(DfsArg *arg, AlgList *sols, Solver *solver, Threader *threader)
{
	int nthreads = arg->opts->nthreads;
	ThreadInitData tid[nthreads];
	pthread_t t[nthreads];

	pthread_mutex_t *sols_mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(sols_mutex, NULL);

	arg->threaddata = malloc(sizeof(ThreadData));
	ThreadData *td  = (ThreadData *)arg->threaddata;
	td->sols        = sols;
	td->sols_mutex  = sols_mutex;

	AlgList *starts = possible_starts(arg, solver);
	AlgListNode *node = starts->first;
	pthread_mutex_t *start_mutex = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(start_mutex, NULL);
	for (int i = 0; i < nthreads; i++) {
		tid[i].arg         = arg;
		tid[i].solver      = solver;
		tid[i].threader    = threader;
		tid[i].starts      = starts;
		tid[i].node        = &node;
		tid[i].start_mutex = start_mutex;

		pthread_create(&t[i], NULL, instance_thread, &tid[i]);
	}

	for (int i = 0; i < nthreads; i++)
		pthread_join(t[i], NULL);

	free(td);
	free(sols_mutex);
	free_alglist(starts);
	free(start_mutex);
}

static int
get_nsol(void *threaddata)
{
	ThreadData *td = (ThreadData *)threaddata;

	pthread_mutex_lock(td->sols_mutex);
	int n = td->sols->len;
	pthread_mutex_unlock(td->sols_mutex);

	return n;
}
