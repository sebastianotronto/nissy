#include "steps.h"

/* Check functions ***********************************************************/

static int              check_nothing(Cube cube, int target);
static int              check_centers(Cube cube, int target);
static int              check_eofb_HTM(Cube cube, int target);
static int              check_coud_HTM(Cube cube, int target);
static int              check_coud_URF(Cube cube, int target);
static int              check_corners_HTM(Cube cube, int target);
static int              check_cornershtr_HTM(Cube cube, int target);
static int              check_corners_URF(Cube cube, int target);
static int              check_cornershtr_URF(Cube cube, int target);
static int              check_ep_HTM(Cube cube, int target);
static int              check_edges_HTM(Cube cube, int target);
static int              check_drud_HTM(Cube cube, int target);
static int              check_optimal_HTM(Cube cube, int target);
static int              check_corners6eo_HTM(Cube cube, int target);
static int              check_tripleeo_HTM(Cube cube, int target);


/* Index functions ***********************************************************/

static uint64_t         index_eofb(Cube cube);
static uint64_t         index_coud(Cube cube);
static uint64_t         index_corners(Cube cube);
static uint64_t         index_cornershtr(Cube cube);
static uint64_t         index_ep(Cube cube);
static uint64_t         index_drud(Cube cube);
static uint64_t         index_corners6eo(Cube cube);
static uint64_t         index_tripleeo(Cube cube);


/* Steps *********************************************************************/

Step
eofb_HTM = {
	.check     = check_eofb_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
coud_HTM = {
	.check     = check_coud_HTM,
	.ready     = check_centers,
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
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
cornershtr_HTM = {
	.check     = check_cornershtr_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
cornershtr_URF = {
	.check     = check_cornershtr_URF,
	.ready     = check_nothing,
	.moveset   = moveset_URF
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
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
drud_HTM = {
	.check     = check_drud_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
optimal_HTM = {
	.check     = check_optimal_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};

Step
tripleeo_HTM = {
	.check     = check_tripleeo_HTM,
	.ready     = check_centers,
	.moveset   = moveset_HTM
};


/* Blocks ********************************************************************/

Block
block_223dl = {
	.edge   = { [DF] = 1, [DL] = 1, [DB] = 1, [FL] = 1, [BL] = 1 },
	.corner = { [DFL] = 1, [DBL] = 1 },
	.center = { [D_center] = 1, [L_center] = 1 }
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
pd_cornershtr_HTM = {
	.filename = "ptable_cornershtr_HTM",
	.size     = POW3TO7 * BINOM8ON4 * 6, /*TODO: check */
	.index    = index_cornershtr,
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

PruneData
pd_corners6eo_HTM = {
	.filename = "ptable_corners6eo_HTM",
	.size     = POW2TO6 * POW3TO7 * FACTORIAL8,
	.index    = index_corners6eo,
	.moveset  = moveset_HTM
};

PruneData
pd_tripleeo_HTM = {
	.filename = "ptable_tripleeo_HTM",
	.size     = BINOM12ON4 * BINOM8ON4 * POW2TO11,
	.index    = index_tripleeo,
	.moveset  = moveset_HTM
};


/* Alg sets ******************************************************************/

/* TODO: remove this (I am only keeping it as an example for future algesets)
AlgSet
as_ephtr_HTM = {
	.filename = "algset_ephtr_HTM",
	.size     = FACTORIAL4 * FACTORIAL4 * FACTORIAL4,
	.index    = index_ephtr,
	.antindex = index_to_cube_ephtr,
	.step     = (Step) {
		.check   = check_optimal_HTM,
		.ready   = check_htr_solved_corners_HTM,
		.moveset = moveset_HTM
	},
	.opts     = (SolveOptions) {
		.min_moves     = 0,
		.max_moves     = 20,
		.max_solutions = 1,
		.optimal_only  = true,
		.can_niss      = false,
		.feedback      = true,
		.pre_trans     = uf
	}
};
*/


/* Check functions implementation ********************************************/

static int
check_nothing(Cube cube, int target)
{
	return 0;
}

static int
check_centers(Cube cube, int target)
{
	return (cube.cpos == 0) ? 0 : 1;
}

static int
check_eofb_HTM(Cube cube, int target)
{
	if (!pd_eofb_HTM.generated)
		genptable(&pd_eofb_HTM);

	return ptableval(&pd_eofb_HTM, cube.eofb);
}

static int
check_coud_HTM(Cube cube, int target)
{
	if (!pd_coud_HTM.generated)
		genptable(&pd_coud_HTM);

	return ptableval(&pd_coud_HTM, cube.coud);
}

static int
check_coud_URF(Cube cube, int target)
{
	/* TODO: I can improve this by checking first the orientation of
	 * the corner in DBL and use that as a reference */

	int ud = check_coud_HTM(cube, target);
	int rl = check_coud_HTM(apply_move(z, cube), target);
	int fb = check_coud_HTM(apply_move(x, cube), target);

	return MIN(ud, MIN(rl, fb));
}

static int
check_corners_HTM(Cube cube, int target)
{
	if (!pd_corners_HTM.generated)
		genptable(&pd_corners_HTM);

	return ptableval(&pd_corners_HTM, index_corners(cube));
}

static int
check_cornershtr_HTM(Cube cube, int target)
{
	if (!pd_cornershtr_HTM.generated)
		genptable(&pd_cornershtr_HTM);

	return ptableval(&pd_cornershtr_HTM, index_cornershtr(cube));
}

static int
check_cornershtr_URF(Cube cube, int target)
{
	/* TODO: I can improve this by checking first the corner in DBL
	 * and use that as a reference */

	int c, ret = 15;
	Trans i;

	for (i = 0; i < NROTATIONS; i++) {
		c = check_cornershtr_HTM(apply_alg(trans_alg(i),cube), target);
		ret = MIN(ret, c);
	}

	return ret;
}

static int
check_corners_URF(Cube cube, int target)
{
	/* TODO: I can improve this by checking first the corner in DBL
	 * and use that as a reference */

	int ret = 15;
	Trans i;

	for (i = 0; i < NROTATIONS; i++)
		ret = MIN(ret, check_corners_HTM(
			  apply_alg(trans_alg(i), cube), target));

	return ret;
}

static int
check_ep_HTM(Cube cube, int target)
{
	if (!pd_ep_HTM.generated)
		genptable(&pd_ep_HTM);

	return ptableval(&pd_ep_HTM, index_ep(cube));
}

static int
check_edges_HTM(Cube cube, int target)
{
	return MAX(check_ep_HTM(cube, target),
	           check_tripleeo_HTM(cube, target));
}

static int
check_drud_HTM(Cube cube, int target)
{
	if (!pd_drud_HTM.generated)
		genptable(&pd_drud_HTM);

	return ptableval(&pd_drud_HTM, index_drud(cube));
}

static int
check_optimal_HTM(Cube cube, int target)
{
	int dr1, dr2, dr3, cor, ret;
	Cube cube2, cube3;

	dr1 = check_drud_HTM(cube, target);
	/*cor = check_corners6eo_HTM(cube, target);*/
	cor = check_corners_HTM(cube, target);
	ret = MAX(dr1, cor);

	if (ret > target)
		return ret;

	cube2 = apply_trans(rf, cube);
	dr2 = check_drud_HTM(cube2, target);
	/*cor = check_corners6eo_HTM(cube2, target);
	ret = MAX(ret, MAX(dr2, cor));*/
	ret = MAX(ret, dr2);

	if (ret > target)
		return ret;

	cube3 = apply_trans(bd, cube);
	dr3 = check_drud_HTM(cube3, target);
	/*cor = check_corners6eo_HTM(cube3, target);*/

	if (dr1 == dr2 && dr2 == dr3 && dr1 != 0)
		dr3++;

	/*ret = MAX(ret, MAX(dr3, cor));*/
	ret = MAX(ret, dr3);

	if (ret == 0)
		return (!cube.epose && !cube.eposs && !cube.eposm) ? 0 : 6;

	return ret; 
}

static int
check_corners6eo_HTM(Cube cube, int target)
{
	if (!pd_corners6eo_HTM.generated)
		genptable(&pd_corners6eo_HTM);

	return ptableval(&pd_corners6eo_HTM, index_corners6eo(cube));
}

static int
check_tripleeo_HTM(Cube cube, int target)
{
	if (!pd_tripleeo_HTM.generated)
		genptable(&pd_tripleeo_HTM);

	return ptableval(&pd_tripleeo_HTM, index_tripleeo(cube));
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
index_cornershtr(Cube cube)
{
	return cube.coud * BINOM8ON4 * 6 + cphtr(cube);
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

	a = cube.epose;
	b = (cube.eposs % FACTORIAL4) + epos_dependent(cube)*FACTORIAL4;
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

static uint64_t
index_corners6eo(Cube cube)
{
	return (cube.coud*FACTORIAL8 + cube.cp)*POW2TO6 + (cube.eofb%POW2TO6);
}

static uint64_t
index_tripleeo(Cube cube)
{
	uint64_t ee, es;

	ee = cube.epose / FACTORIAL4;
	es = epos_dependent(cube);

	return (ee * BINOM8ON4 + es) * POW2TO11 + cube.eofb;
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

