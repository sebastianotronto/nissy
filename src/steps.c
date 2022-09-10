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
compute_ind(Step *s, Cube *cube, Movable *ind)
{
	int i;
	Cube mvd;
	Trans t, tt;

	for (i = 0; i < s->n_coord; i++) {
		t = s->coord_trans[i];
		copy_cube(cube, &mvd);
		apply_trans(t, &mvd);

		ind[i].val = index_coord(s->coord[i], &mvd, &tt);
		ind[i].t = transform_trans(tt, t);
	}
}

void
prepare_cs(ChoiceStep *cs, SolveOptions *opts)
{
	int i, j;
	Step *s;

	for (i = 0; cs->step[i] != NULL; i++) {
		s = cs->step[i];
		for (j = 0; j < s->n_coord; j++) {
			s->pd[j] = malloc(sizeof(PruneData));
			s->pd[j]->moveset = s->moveset;
			s->pd[j]->coord   = s->coord[j];
			s->pd[j]->compact = s->pd_compact[j];
			s->pd[j] = genptable(s->pd[j], opts->nthreads);
		}
	}
}
