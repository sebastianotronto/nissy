#include "steps.h"

/* Check functions ***********************************************************/

static int              check_nothing(Cube cube);
static int              check_eofb_HTM(Cube cube);
static int              check_coud_HTM(Cube cube);
static int              check_coud_URF(Cube cube);
static int              check_corners_HTM(Cube cube);
static int              check_corners_URF(Cube cube);
static int              check_edges_HTM(Cube cube);
static int              check_drud_HTM(Cube cube);
static int              check_optimal_HTM(Cube cube);


/* Index functions ***********************************************************/

static uint64_t         index_eofb(Cube cube);
static uint64_t         index_coud(Cube cube);
static uint64_t         index_corners(Cube cube);
static uint64_t         index_ep(Cube cube);
static uint64_t         index_drud(Cube cube);


/* Steps *********************************************************************/

Step
eofb_HTM = {
	.check     = check_eofb_HTM,
	.ready     = check_nothing,
	.moveset   = moveset_HTM
};

Step
coud_HTM = {
	.check     = check_coud_HTM,
	.ready     = check_nothing,
	.moveset   = moveset_HTM
};

Step
coud_URF = {
	.check     = check_coud_URF,
	.ready     = check_nothing,
	.moveset   = moveset_URF
};

Step
corners_HTM = {
	.check     = check_corners_HTM,
	.ready     = check_nothing,
	.moveset   = moveset_HTM
};

Step
corners_URF = {
	.check     = check_corners_URF,
	.ready     = check_nothing,
	.moveset   = moveset_URF
};

Step
edges_HTM = {
	.check     = check_edges_HTM,
	.ready     = check_nothing,
	.moveset   = moveset_HTM
};

Step
drud_HTM = {
	.check     = check_drud_HTM,
	.ready     = check_nothing,
	.moveset   = moveset_HTM
};

Step
optimal_HTM = {
	.check     = check_optimal_HTM,
	.ready     = check_nothing,
	.moveset   = moveset_HTM,
	.cd        = &cd_optimal_HTM_6
};


/* Cache solutions ***********************************************************/

CacheData
cd_optimal_HTM_6 = {
	.filename  = "cd_optimal_HTM_6",
	.maxind0   = POW3TO7 * FACTORIAL8,
	.len       = 6,
	.nind      = 3,
	.index     = { index_corners, index_eofb, index_ep },
	.moveset   = moveset_HTM
};


/* Pruning tables ************************************************************/

PruneData
pd_eofb_HTM = {
	.filename = "ptable_eofb_HTM",
	.size     = POW2TO11,
	.index    = index_eofb,
	.moveset  = moveset_HTM
};

PruneData
pd_coud_HTM = {
	.filename = "ptable_coud_HTM",
	.size     = POW3TO7,
	.index    = index_coud,
	.moveset  = moveset_HTM
};

PruneData
pd_corners_HTM = {
	.filename = "ptable_corners_HTM",
	.size     = POW3TO7 * FACTORIAL8,
	.index    = index_corners,
	.moveset  = moveset_HTM
};

PruneData
pd_ep_HTM = {
	.filename = "ptable_ep_HTM",
	.size     = FACTORIAL12,
	.index    = index_ep,
	.moveset  = moveset_HTM
};

PruneData
pd_drud_HTM = {
	.filename = "ptable_drud_HTM",
	.size     = POW2TO11 * POW3TO7 * BINOM12ON4,
	.index    = index_drud,
	.moveset  = moveset_HTM
};


/* Check functions implementation ********************************************/

static int
check_nothing(Cube cube)
{
	/* At least we check that it is admissible */
	return is_admissible(cube);
}

static int
check_eofb_HTM(Cube cube)
{
	if (!pd_eofb_HTM.generated)
		generate_ptable(&pd_eofb_HTM);

	return PTABLEVAL(pd_eofb_HTM.ptable, cube.eofb);
}

static int
check_coud_HTM(Cube cube)
{
	if (!pd_coud_HTM.generated)
		generate_ptable(&pd_coud_HTM);

	return PTABLEVAL(pd_coud_HTM.ptable, cube.coud);
}

static int
check_coud_URF(Cube cube)
{
	/* TODO: I can improve this by checking first the orientation of
	 * the corner in DBL and use that as a reference */

	int ud = check_coud_HTM(cube);
	int rl = check_coud_HTM(apply_move(z, cube));
	int fb = check_coud_HTM(apply_move(x, cube));

	return MIN(ud, MIN(rl, fb));
}

static int
check_corners_HTM(Cube cube)
{
	if (!pd_corners_HTM.generated)
		generate_ptable(&pd_corners_HTM);

	return PTABLEVAL(pd_corners_HTM.ptable, index_corners(cube));
}

static int
check_corners_URF(Cube cube)
{
	/* TODO: I can improve this by checking first the corner in DBL
	 * and use that as a reference */

	int ret = 15;
	Trans i;

	for (i = 0; i < NROTATIONS; i++)
		ret = MIN(ret,check_corners_HTM(apply_alg(trans_alg(i),cube)));

	return ret;
}

static int
check_edges_HTM(Cube cube)
{
	int ret = 0;

	if (!pd_ep_HTM.generated)
		generate_ptable(&pd_ep_HTM);

	ret = MAX(ret, PTABLEVAL(pd_ep_HTM.ptable, index_ep(cube)));
	ret = MAX(ret, check_eofb_HTM(cube));
	ret = MAX(ret, check_eofb_HTM(apply_trans(ur, cube)));
	ret = MAX(ret, check_eofb_HTM(apply_trans(fd, cube)));

	return ret;
}

static int
check_drud_HTM(Cube cube)
{
	if (!pd_drud_HTM.generated)
		generate_ptable(&pd_drud_HTM);

	return PTABLEVAL(pd_drud_HTM.ptable, index_drud(cube));
}

static int
check_optimal_HTM(Cube cube)
{
	int dr1, dr2, dr3, drmax, cor; /*ep;*/

	if (!pd_drud_HTM.generated)
		generate_ptable(&pd_drud_HTM);
	if (!pd_corners_HTM.generated)
		generate_ptable(&pd_corners_HTM);
	/*
	 *if (!pd_ep_HTM.generated)
 	 *	generate_ptable(&pd_ep_HTM);
	 */

	dr1 = PTABLEVAL(pd_drud_HTM.ptable, index_drud(cube));
	dr2 = PTABLEVAL(pd_drud_HTM.ptable, index_drud(apply_trans(rf, cube)));
	dr3 = PTABLEVAL(pd_drud_HTM.ptable, index_drud(apply_trans(fd, cube)));

	drmax = MAX(dr1, MAX(dr2, dr3));
	if (dr1 == dr2 && dr2 == dr3 && dr1 != 0)
		drmax++;

	cor = PTABLEVAL(pd_corners_HTM.ptable, index_corners(cube));
	/* ep = PTABLEVAL(pd_ep_HTM.ptable, index_ep(cube)); */

	/*return MAX(drmax, MAX(ep, cor));*/
	if (drmax == 0 && cor == 0)
		return is_solved(cube, false) ? 0 : 1;
	return MAX(drmax, cor); 
}


/* Index functions implementation ********************************************/

static uint64_t
index_eofb(Cube cube)
{
	return cube.eofb;
}

static uint64_t
index_coud(Cube cube)
{
	return cube.coud;
}

static uint64_t
index_corners(Cube cube)
{
	return cube.coud * FACTORIAL8 + cube.cp;
}

static uint64_t
index_ep(Cube cube)
{
	uint64_t a, b, c;

	a = cube.eposs;
	b = (cube.epose % FACTORIAL4) + epos_dependent_cube(cube)*FACTORIAL4;
	c = cube.eposm % FACTORIAL4;

	b *= FACTORIAL4 * BINOM12ON4;
	c *= FACTORIAL4 * BINOM12ON4 * FACTORIAL4 * BINOM8ON4;

	return a + b + c;
}

static uint64_t
index_drud(Cube cube)
{
	uint64_t a, b, c;

	a = cube.eofb;
	b = cube.coud;
	c = cube.epose / FACTORIAL4;

	b *= POW2TO11;
	c *= POW2TO11 * POW3TO7;

	return a + b + c;
}

/* Movesets ******************************************************************/

bool
moveset_HTM(Move m)
{
	return m >= U && m <= B3;
}

bool
moveset_URF(Move m)
{
	Move b = base_move(m);

	return b == U || b == R || b == F;
}

