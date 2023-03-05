#include "solver_step.h"

typedef struct {
	Cube *          cube;
	uint64_t *      val;
	Trans *         t;
} CubeData;

static void             apply_move_cubedata(void *, void *, Move);
static void             init_indexes(Step *, CubeData *);
static void *           prepare_cube(void *, Cube *);
static bool             move_check_stop_eager(void *, DfsArg *, Threader *);
static bool             move_check_stop_lazy(void *, DfsArg *, Threader *);
static bool             move_check_stop_nonsol(void *, DfsArg *, Threader *);
static bool             is_solved_step(void *, void *);
static Alg *            validate_solution(void *, Alg *);
static void *           alloc_cubedata(void *);
static void             copy_cubedata(void *, void *, void *);
static void             free_cubedata(void *, void *);
static void             invert_cubedata(void *, void *);
static bool             niss_makes_sense(void *, void *, Alg *);
static Solver *         new_stepsolver_nocheckstop(Step *step);

static void
apply_move_cubedata(void *param, void *cubedata, Move m)
{
	Step *s = (Step *)param;
	CubeData *data = (CubeData *)cubedata;

	Trans tt;
	for (int i = 0; i < s->n_coord; i++) {
		Move mm = transform_move(data->t[i], m);
		data->val[i] = move_coord(s->coord[i], mm, data->val[i], &tt);
		data->t[i]   = transform_trans(tt, data->t[i]);
	}
}

static void
init_indexes(Step *step, CubeData *data)
{
	int i;
	Cube moved;
	Trans t, tt;

	for (i = 0; i < step->n_coord; i++) {
		t = step->coord_trans[i];
		copy_cube(data->cube, &moved);
		apply_trans(t, &moved);
		data->val[i] = index_coord(step->coord[i], &moved, &tt);
		data->t[i]   = transform_trans(tt, t);
	}
}

static void *
prepare_cube(void *param, Cube *cube)
{
	int i;
	Step *s;
	CubeData *data;

	s = (Step *)param;

	for (i = 0; i < s->n_coord; i++) {
		s->pd[i] = malloc(sizeof(PruneData));
		s->pd[i]->moveset = s->moveset;
/* TODO: check if moveset initialization works fine,
   e.g. if there is a variable to save the initialized status
   or if it gets initialized multiple times */
		init_moveset(s->moveset);
		s->pd[i]->coord   = s->coord[i];
		gen_coord(s->coord[i]);
		s->pd[i]->compact = s->pd_compact[i];
		s->pd[i] = genptable(s->pd[i], 4); /* TODO: threads */
	}

	data = alloc_cubedata(param);
	data->cube = malloc(sizeof(Cube));
	copy_cube(cube, data->cube);
	init_indexes(s, data);

	return data;
}

static bool
move_check_stop_eager(void *param, DfsArg *arg, Threader *threader)
{
	int nsol;

	if (move_check_stop_nonsol(param, arg, threader))
		return true;

	nsol = threader->get_nsol(arg->threaddata);
	return nsol >= arg->opts->max_solutions;
}

static bool
move_check_stop_lazy(void *param, DfsArg *arg, Threader *threader)
{
	int nsol;

	nsol = threader->get_nsol(arg->threaddata);
	if (nsol >= arg->opts->max_solutions)
		return true;

	return move_check_stop_nonsol(param, arg, threader);
}

/* TODO: split in 2 (nissable / non-nissable) and only move cube
	when nissable */
static bool
move_check_stop_nonsol(void *param, DfsArg *arg, Threader *threader)
{
	int i, goal, bound;
	Move mm, lastmove;
	Trans tt = uf;
	CubeData *data;
	Step *s;

	s = (Step *)param;
	data = (CubeData *)arg->cubedata;


	bound = 0;
	goal = arg->d - arg->current_alg->len;
/* TODO: check if len is 0 */
	lastmove = arg->current_alg->move[arg->current_alg->len-1];
	for (i = 0; i < s->n_coord; i++) {
		mm = transform_move(data->t[i], lastmove);
		data->val[i] = move_coord(s->coord[i], mm, data->val[i], &tt);
		data->t[i]   = transform_trans(tt, data->t[i]);

		bound = MAX(bound, ptableval(s->pd[i], data->val[i]));
		if (arg->opts->can_niss && !arg->niss)
			bound = MIN(1, bound);

		if (bound > goal) {
			return true;
		}
	}
	if (arg->opts->can_niss && !arg->niss)
		apply_move(lastmove, data->cube);

	return false;
}

static bool
is_solved_step(void *param, void *cubedata)
{
	int i;
	Step *s;
	CubeData *data;

	s = (Step *)param;
	data = (CubeData *)cubedata;

	for (i = 0; i < s->n_coord; i++)
		if (data->val[i] != 0)
			return false;

	return true;
}

static Alg *
validate_solution(void *param, Alg *alg)
{
	return ((Step *)param)->is_valid(alg);
}

static void *
alloc_cubedata(void *param)
{
	Step *s;
	CubeData *data;

	s = (Step *)param;

	data = malloc(sizeof(CubeData));
	/* We do not need to allocate a cube */
	data->val  = malloc(s->n_coord * sizeof(uint64_t));
	data->t    = malloc(s->n_coord * sizeof(Trans));

	return data;
}

static void
copy_cubedata(void *param, void *src, void *dst)
{
	int i;
	Step *s;
	CubeData *newdata, *olddata;

	s = (Step *)param;
	olddata = (CubeData *)src;
	newdata = (CubeData *)dst;

/* TODO: do not copy if not nissable */
	newdata->cube = malloc(sizeof(Cube));
	copy_cube(olddata->cube, newdata->cube);
	for (i = 0; i < s->n_coord; i++) {
		newdata->val[i] = olddata->val[i];
		newdata->t[i]   = olddata->t[i];
	}
}

static void
free_cubedata(void *param, void *cubedata)
{
	CubeData *data;

	data = (CubeData *)cubedata;

	free(data->t);
	free(data->val);
	free(data->cube);
	free(data);
}

static void
invert_cubedata(void *param, void *cubedata)
{
	Step *s;
	CubeData *data;

	s = (Step *)param;
	data = (CubeData *)cubedata;

	invert_cube(data->cube);
	init_indexes(s, data);
}

static bool
niss_makes_sense(void *param, void *cubedata, Alg *alg)
{
	Step *s;
	CubeData *data;

	s = (Step *)param;
	data = (CubeData *)cubedata;

	if (s->final)
		return false;

	if (alg->len_normal == 0)
		return true;

	Move m = inverse_move(alg->move_normal[alg->len_normal-1]);
        for (int i = 0; i < s->n_coord; i++) {
                Move mm = transform_move(data->t[i], m);
                uint64_t u = move_coord(s->coord[i], mm, 0, NULL);
                if (ptableval(s->pd[i], u) > 0)
                        return true;
        }

        return false;
}

static Solver *
new_stepsolver_nocheckstop(Step *step)
{
	Solver *solver;

	solver = malloc(sizeof(Solver));

	solver->moveset = step->moveset;
	solver->param   = step;

	solver->apply_move        = apply_move_cubedata;
	solver->prepare_cube      = prepare_cube;
	solver->is_solved         = is_solved_step;
	solver->validate_solution = validate_solution;
	solver->alloc_cubedata    = alloc_cubedata;
	solver->copy_cubedata     = copy_cubedata;
	solver->free_cubedata     = free_cubedata;
	solver->invert_cube       = invert_cubedata;
	solver->niss_makes_sense  = niss_makes_sense;

	return solver;
}

Solver *
new_stepsolver_eager(Step *step)
{
	Solver *solver;

	solver = new_stepsolver_nocheckstop(step);
	solver->move_check_stop = move_check_stop_eager;

	return solver;
}

Solver *
new_stepsolver_lazy(Step *step)
{
	Solver *solver;

	solver = new_stepsolver_nocheckstop(step);
	solver->move_check_stop = move_check_stop_lazy;

	return solver;
}

void
free_stepsolver(Solver *solver)
{
	free(solver);
}
