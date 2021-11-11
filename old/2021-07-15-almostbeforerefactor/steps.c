#include "steps.h"

/* Standard checkers (return lower bound) ************************************/

static int              estimate_eofb_HTM(CubeTarget ct);
static int              estimate_coud_HTM(CubeTarget ct);
static int              estimate_coud_URF(CubeTarget ct);
static int              estimate_corners_HTM(CubeTarget ct);
static int              estimate_cornershtr_HTM(CubeTarget ct);
static int              estimate_corners_URF(CubeTarget ct);
static int              estimate_cornershtr_URF(CubeTarget ct);
static int              estimate_drud_HTM(CubeTarget ct);
static int              estimate_optimal_HTM(CubeTarget ct);

/* Steps *********************************************************************/

Step
eofb_HTM = {
	.estimate  = estimate_eofb_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
coud_HTM = {
	.estimate  = estimate_coud_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
coud_URF = {
	.estimate  = estimate_coud_URF,
	.ready     = check_nothing,
	.moveset   = moveset_URF
};

Step
corners_HTM = {
	.estimate  = estimate_corners_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
cornershtr_HTM = {
	.estimate  = estimate_cornershtr_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
cornershtr_URF = {
	.estimate  = estimate_cornershtr_URF,
	.ready     = check_nothing,
	.moveset   = moveset_URF
};

Step
corners_URF = {
	.estimate  = estimate_corners_URF,
	.ready     = check_nothing,
	.moveset   = moveset_URF
};

Step
drud_HTM = {
	.estimate  = estimate_drud_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
optimal_HTM = {
	.estimate  = estimate_optimal_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};


/* Pruning tables ************************************************************/

PruneData
pd_eofb_HTM = {
	.filename = "ptable_eofb_HTM",
	.coord    = &coord_eofb,
	.moveset  = moveset_HTM,
	.ntrans   = 1,
	.trans    = trans_group_trivial
};

PruneData
pd_coud_HTM = {
	.filename = "ptable_coud_HTM",
	.coord    = &coord_coud,
	.moveset  = moveset_HTM,
	.ntrans   = 1,
	.trans    = trans_group_trivial
};

PruneData
pd_cornershtr_HTM = {
	.filename = "ptable_cornershtr_withcosets_HTM",
	.coord    = &coord_cornershtr,
	.moveset  = moveset_HTM,
	.ntrans   = 1,
	.trans    = trans_group_trivial
};

PruneData
pd_corners_HTM = {
	.filename = "ptable_corners_HTM",
	.coord    = &coord_corners,
	.moveset  = moveset_HTM,
	.ntrans   = 1,
	.trans    = trans_group_trivial
};

PruneData
pd_drud_HTM = {
	.filename = "ptable_drud_HTM",
	.coord    = &coord_drud,
	.moveset  = moveset_HTM,
	.ntrans   = 1,
	.trans    = trans_group_trivial
};

PruneData
pd_drud_sym16_HTM = {
	.filename = "ptable_drud_sym16_HTM",
	.coord    = &coord_drud_sym16,
	.moveset  = moveset_HTM,
	.ntrans   = 16,
	.trans    = trans_group_udfix
};

PruneData
pd_khuge_HTM = {
	.filename = "ptable_khuge_HTM",
	.coord    = &coord_khuge,
	.moveset  = moveset_HTM,
	.ntrans   = 16,
	.trans    = trans_group_udfix
};


/* Standard checkers (return lower bound) ************************************/

static int
estimate_eofb_HTM(CubeTarget ct)
{
	if (!pd_eofb_HTM.generated)
		genptable(&pd_eofb_HTM);

	return ptableval(&pd_eofb_HTM, ct.cube);
}

static int
estimate_coud_HTM(CubeTarget ct)
{
	if (!pd_coud_HTM.generated)
		genptable(&pd_coud_HTM);

	return ptableval(&pd_coud_HTM, ct.cube);
}

static int
estimate_coud_URF(CubeTarget ct)
{
	/* TODO: I can improve this by checking first the orientation of
	 * the corner in DBL and use that as a reference */

	CubeTarget ct2 = {.cube = apply_move(z, ct.cube), .target = ct.target};
	CubeTarget ct3 = {.cube = apply_move(x, ct.cube), .target = ct.target};

	int ud = estimate_coud_HTM(ct);
	int rl = estimate_coud_HTM(ct2);
	int fb = estimate_coud_HTM(ct3);

	return MIN(ud, MIN(rl, fb));
}

static int
estimate_corners_HTM(CubeTarget ct)
{
	if (!pd_corners_HTM.generated)
		genptable(&pd_corners_HTM);

	return ptableval(&pd_corners_HTM, ct.cube);
}

static int
estimate_cornershtr_HTM(CubeTarget ct)
{
	if (!pd_cornershtr_HTM.generated)
		genptable(&pd_cornershtr_HTM);

	return ptableval(&pd_cornershtr_HTM, ct.cube);
}

static int
estimate_cornershtr_URF(CubeTarget ct)
{
	/* TODO: I can improve this by checking first the corner in DBL
	 * and use that as a reference */

	int c, ret = 15;
	Trans i;

	for (i = 0; i < NROTATIONS; i++) {
		ct.cube = apply_alg(rotation_alg(i), ct.cube);
		c = estimate_cornershtr_HTM(ct);
		ret = MIN(ret, c);
	}

	return ret;
}

static int
estimate_corners_URF(CubeTarget ct)
{
	/* TODO: I can improve this by checking first the corner in DBL
	 * and use that as a reference */

	int c, ret = 15;
	Trans i;

	for (i = 0; i < NROTATIONS; i++) {
		ct.cube = apply_alg(rotation_alg(i), ct.cube);
		c = estimate_corners_HTM(ct);
		ret = MIN(ret, c);
	}

	return ret;
}

static int
estimate_drud_HTM(CubeTarget ct)
{
/*
	if (!pd_drud_HTM.generated)
		genptable(&pd_drud_HTM);

	return ptableval(&pd_drud_HTM, ct.cube);
*/

	if (!pd_drud_sym16_HTM.generated)
		genptable(&pd_drud_sym16_HTM);

	return ptableval(&pd_drud_sym16_HTM, ct.cube);
}

static int
estimate_optimal_HTM(CubeTarget ct)
{
	int dr1, dr2, dr3, cor, ret;
	Cube cube = ct.cube;

	if (!pd_khuge_HTM.generated)
		genptable(&pd_khuge_HTM);

	dr1 = ptableval(&pd_khuge_HTM, cube);
	cor = estimate_corners_HTM(ct);
	ret = MAX(dr1, cor);

	if (ret > ct.target)
		return ret;

	cube = apply_trans(rf, ct.cube);
	dr2 = ptableval(&pd_khuge_HTM, cube);
	ret = MAX(ret, dr2);

	if (ret > ct.target)
		return ret;

	cube = apply_trans(fd, ct.cube);
	dr3 = ptableval(&pd_khuge_HTM, cube);

	/* Michiel de Bondt's trick */
	if (dr1 == dr2 && dr2 == dr3 && dr1 != 0)
		dr3++;

	return MAX(ret, dr3);
}
