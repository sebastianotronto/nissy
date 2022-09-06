#define STEPS_C

#include "steps.h"

/* TODO: change all checkers to use coordinates! */

bool
check_centers(Cube *cube)
{
	int i;

	for (i = 0; i < 6; i++)
		if (cube->xp[i] != i)
			return false;

	return true;
}

bool
check_coud_HTM(Cube *cube)
{
	int i;

	for (i = 0; i < 8; i++)
		if (cube->co[i] != 0)
			return false;

	return true;
}

bool
check_coud_URF(Cube *cube)
{
	Cube c2, c3;

	copy_cube(cube, &c2);
	copy_cube(cube, &c3);

	apply_move(z, &c2);
	apply_move(x, &c3);

	return check_coud_HTM(cube) ||
	       check_coud_HTM(&c2)  ||
	       check_coud_HTM(&c3);
}

bool
check_cp_HTM(Cube *cube)
{
	int i;

	for (i = 0; i < 8; i++)
		if (cube->cp[i] != i)
			return false;

	return true;
}

bool
check_corners_HTM(Cube *cube)
{
	return check_coud_HTM(cube) && check_cp_HTM(cube);
}

bool
check_corners_URF(Cube *cube)
{
	Cube c;
	Trans i;

	for (i = 0; i < NROTATIONS; i++) {
		copy_cube(cube, &c);
		apply_alg(rotation_alg(i), &c);
		if (check_corners_HTM(&c))
			return true;
	}

	return false;
}

bool
check_cornershtr(Cube *cube)
{
	/* TODO (use coord) */
	return true;
}

bool
check_eofb(Cube *cube)
{
	/* TODO (use coord) */
	return true;
}

bool
check_drud(Cube *cube)
{
	/* TODO (use coord) */
	return true;
}

bool
check_htr(Cube *cube)
{
	/* TODO (check_drud(cube) and coord_htr_drud == 0) */
	return true;
}

bool
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

/* Public functions **********************************************************/

void
compute_ind(StepAlt *a, Cube *cube, Movable *ind)
{
	int i;
	Cube mvd;
	Trans t, tt;

	for (i = 0; i < a->n_coord; i++) {
		t = a->coord_trans[i];
		copy_cube(cube, &mvd);
		apply_trans(t, &mvd);

		ind[i].val = index_coord(a->coord[i], &mvd, &tt);
		ind[i].t = transform_trans(tt, t);
	}
}

int
estimate_stepalt(StepAlt *a, Movable *ind, int goal)
{
	int i, ret, est[a->n_coord];

	for (i = 0; i < a->n_coord; i++) {
		est[i] = ptableval(a->pd[i], ind[i].val);
		if (est[i] == a->pd[i]->base && a->compact_pd[i])
			est[i] = ptableval(a->fallback_pd[i],
			    ind[i].val/a->fbmod[i]);
		if (ind[i].val != 0 && est[i] == 0) /* est == 0 iff solved */
			est[i] = 1;
/*
TODO: remove this debug code
printf("%d: est=%d | ", i, est[i]);
*/

		if (est[i] > goal)
			return est[i];
	}

	for (i = 0; i < a->n_dbtrick; i++)
		if (est[a->dbtrick[i][0]] > 0 &&
		    est[a->dbtrick[i][0]] == est[a->dbtrick[i][1]] &&
		    est[a->dbtrick[i][0]] == est[a->dbtrick[i][2]])
			est[a->dbtrick[i][0]] += 1;

	for (i = 0, ret = -1; i < a->n_coord; i++)
		ret = MAX(ret, est[i]);

/*
TODO: remove this debug code
printf("Final estimate: %d\n", ret);
*/

	return ret;
}

void
prepare_step(Step *step, SolveOptions *opts)
{
	int i, j;
	PDGenData pdg;
	StepAlt *a;

	for (i = 0; step->alt[i] != NULL; i++) {
		a = step->alt[i];
		init_moveset(a->moveset);
		pdg.moveset = a->moveset;
		for (j = 0; j < a->n_coord; j++) {
			gen_coord(a->coord[j]);

			pdg.coord   = a->coord[j];
			pdg.compact = a->compact_pd[j];
			pdg.pd      = NULL;

			a->pd[j] = genptable(&pdg, opts->nthreads);

			if (a->compact_pd[j]) {
				gen_coord(a->fallback_coord[j]);

				pdg.coord   = a->fallback_coord[j];
				pdg.compact = false;
				pdg.pd      = NULL;

				a->fallback_pd[j] =
				    genptable(&pdg, opts->nthreads);
			}
		}
	}
}
