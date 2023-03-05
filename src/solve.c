#define SOLVE_C

#include "solve.h"

void
dfs(DfsArg *arg, Solver *solver, Threader *threader)
{
	int i;
	DfsArg newarg;
	Alg *sol;
	Move m;

	if (arg->current_alg->len > arg->d)
		return;

	if (solver->is_solved(solver->param, arg->cubedata)) {
/* TODO: the "all" option should be re-implemented as setting
validate to null */

/* TODO: we also have to check if cancel with NISS;
we can't because we have no access to the s->final field
this should be done by the step's validator? */
		sol = solver->validate_solution(solver->param,arg->current_alg);
		bool accepted = sol != NULL;
		bool too_short = arg->current_alg->len != arg->d;

		if (accepted && !too_short) {
/* TODO: arg->t got lost in refactoring */
/*			transform_alg(inverse_trans(arg->t), sol);*/
			if (arg->opts->verbose)
				print_alg(sol, false);
			threader->append_sol(sol, arg->threaddata);
		}
		return;
	}

	if (arg->current_alg->len == arg->d)
		return;

/* TODO: do not alloc */
	newarg.cubedata = solver->alloc_cubedata(solver->param);
	for (i = 0; solver->moveset->sorted_moves[i] != NULLMOVE; i++) {
		m = solver->moveset->sorted_moves[i];
		if (solver->moveset->can_append(arg->current_alg, m, arg->niss)
		    && compare_last(arg->current_alg, m, arg->niss) >= 0) {
			append_move(arg->current_alg, m, arg->niss);

			solver->copy_cubedata(
			    solver->param, arg->cubedata, newarg.cubedata);
			newarg.threaddata  = arg->threaddata;
			newarg.opts        = arg->opts;
			newarg.d           = arg->d;
			newarg.niss        = arg->niss;
			newarg.current_alg = arg->current_alg;
			if (!solver->move_check_stop(
			    solver->param, &newarg, threader))
				dfs(&newarg, solver, threader);

			remove_last_move(arg->current_alg);
		}
	}
	solver->free_cubedata(solver->param, newarg.cubedata);

	if (arg->opts->can_niss && !arg->niss &&
	    solver->niss_makes_sense(
	    solver->param, arg->cubedata, arg->current_alg)) {
		solver->invert_cube(solver->param, arg->cubedata);
		arg->niss = true;
		dfs(arg, solver, threader);
	}
}

AlgList *
solve(Cube *cube, SolveOptions *opts, Solver **solver, Threader *threader)
{
	int i, d, optimal;
	bool ready[MAX_SOLVERS], stop, one_ready;
	DfsArg arg[MAX_SOLVERS];
	AlgList *sols;

	one_ready = false;
	for (i = 0; solver[i] != NULL; i++) {
		arg[i].cubedata =
		    solver[i]->prepare_cube(solver[i]->param, cube);
		arg[i].opts = opts;
		ready[i] = arg[i].cubedata != NULL;
		one_ready = one_ready || ready[i];
	}

	sols = new_alglist();
	if (!one_ready) {
		fprintf(stderr, "Cube not ready for solving\n");
		return sols;
	}

	optimal = opts->max_moves;
	stop = false;
	for (d = opts->min_moves; d <= opts->max_moves && !stop; d++) {
		if (opts->verbose)
			fprintf(stderr, "Searching depth %d\n", d);

		for (i = 0; solver[i] != NULL && !stop; i++) {
			if (!ready[i])
				continue;

			arg[i].d = d;
			threader->dispatch(&arg[i], sols, solver[i], threader);

			if (sols->len > 0)
				optimal = MIN(optimal, d);

			stop = sols->len >= opts->max_solutions;
		}
		stop = stop ||
		       (opts->optimal != -1 && d >= opts->optimal + optimal);
	}

/* TODO: some cleanup (free cubedata) */
/* TODO: actually, preparation should be done somewhere else */

	return sols;
}
