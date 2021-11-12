#include "coord.h"

static Cube        antindex_eofb(uint64_t ind);
static Cube        antindex_eofbepos(uint64_t ind);
static Cube        antindex_epud(uint64_t ind);
static Cube        antindex_coud(uint64_t ind);
static Cube        antindex_corners(uint64_t ind);
static Cube        antindex_cp(uint64_t ind);
static Cube        antindex_cphtr(uint64_t);
static Cube        antindex_cornershtr(uint64_t ind);
static Cube        antindex_cornershtrfin(uint64_t ind);
static Cube        antindex_drud(uint64_t ind);
static Cube        antindex_drud_eofb(uint64_t ind);
static Cube        antindex_htr_drud(uint64_t ind);
static Cube        antindex_htrfin(uint64_t ind);

static uint64_t    index_eofb(Cube cube);
static uint64_t    index_eofbepos(Cube cube);
static uint64_t    index_epud(Cube cube);
static uint64_t    index_coud(Cube cube);
static uint64_t    index_corners(Cube cube);
static uint64_t    index_cp(Cube cube);
static uint64_t    index_cphtr(Cube cube);
static uint64_t    index_cornershtr(Cube cube);
static uint64_t    index_cornershtrfin(Cube cube);
static uint64_t    index_drud(Cube cube);
static uint64_t    index_drud_eofb(Cube cube);
static uint64_t    index_htr_drud(Cube cube);
static uint64_t    index_htrfin(Cube cube);

static void        init_cphtr_cosets();
static void        init_cphtr_left_cosets_bfs(int i, int c);
static void        init_cphtr_right_cosets_color(int i, int c);
static void        init_cornershtrfin();


/* All sorts of useful costants and tables  **********************************/

static int              cphtr_left_cosets[FACTORIAL8];
static int              cphtr_right_cosets[FACTORIAL8];
static int              cphtr_right_rep[BINOM8ON4*6];
static int              cornershtrfin_ind[FACTORIAL8];
static int              cornershtrfin_ant[24*24/6];

/* Coordinates and their implementation **************************************/

Coordinate
coord_eofb = {
	.index  = index_eofb,
	.cube   = antindex_eofb,
	.max    = POW2TO11,
	.ntrans = 1,
};

Coordinate
coord_eofbepos = {
	.index  = index_eofbepos,
	.cube   = antindex_eofbepos,
	.max    = POW2TO11 * BINOM12ON4,
	.ntrans = 1,
};

Coordinate
coord_coud = {
	.index  = index_coud,
	.cube   = antindex_coud,
	.max    = POW3TO7,
	.ntrans = 1,
};

Coordinate
coord_corners = {
	.index  = index_corners,
	.cube   = antindex_corners,
	.max    = POW3TO7 * FACTORIAL8,
	.ntrans = 1,
};

Coordinate
coord_cp = {
	.index  = index_cp,
	.cube   = antindex_cp,
	.max    = FACTORIAL8,
	.ntrans = 1,
};

Coordinate
coord_cphtr = {
	.index  = index_cphtr,
	.cube   = antindex_cphtr,
	.max    = BINOM8ON4 * 6,
	.ntrans = 1,
};

Coordinate
coord_cornershtr = {
	.index  = index_cornershtr,
	.cube   = antindex_cornershtr,
	.max    = POW3TO7 * BINOM8ON4 * 6,
	.ntrans = 1,
};

Coordinate
coord_cornershtrfin = {
	.index  = index_cornershtrfin,
	.cube   = antindex_cornershtrfin,
	.max    = 24*24/6,
	.ntrans = 1,
};

Coordinate
coord_epud = {
	.index  = index_epud,
	.cube   = antindex_epud,
	.max    = FACTORIAL8,
	.ntrans = 1,
};

Coordinate
coord_drud = {
	.index  = index_drud,
	.cube   = antindex_drud,
	.max    = POW2TO11 * POW3TO7 * BINOM12ON4,
	.ntrans = 1,
};

Coordinate
coord_htr_drud = {
	.index  = index_htr_drud,
	.cube   = antindex_htr_drud,
	.max    = BINOM8ON4 * 6 * BINOM8ON4,
	.ntrans = 1,
};

Coordinate
coord_htrfin = {
	.index  = index_htrfin,
	.cube   = antindex_htrfin,
	.max    = 24 * 24 * 24 *24 * 24 / 6, /* should be /12 but it's ok */
	.ntrans = 1,
};

Coordinate
coord_drud_eofb = {
	.index  = index_drud_eofb,
	.cube   = antindex_drud_eofb,
	.max    = POW3TO7 * BINOM12ON4,
	.ntrans = 1,
};

/* Functions *****************************************************************/

static Cube
antindex_eofb(uint64_t ind)
{
	Cube ret = {0};

	ret.eofb = ind;
	ret.eorl = ind;
	ret.eoud = ind;

	return ret;
}

static Cube
antindex_eofbepos(uint64_t ind)
{
	Cube ret = {0};

	/* We need eorl for sym16 coordinate */
	static int initialized = false;
	static uint64_t eorl_aux[POW2TO11][BINOM12ON4];
	static int eo_aux[12], ep_aux[12];
	unsigned int i, j, k;

	if (!initialized) {
		for (i = 0; i < POW2TO11; i++) {
			for (j = 0; j < BINOM12ON4; j++) {
				int_to_sum_zero_array(i, 2, 12, eo_aux);
				index_to_subset(j, 12, 4, ep_aux);
				for (k = 0; k < 12; k++)
					if (ep_aux[k])
						eo_aux[k] = 1 - eo_aux[k];
			}
		}

		initialized = true;
	}

	ret.eofb = ind % POW2TO11;
	ret.epose = (ind / POW2TO11) * 24;
	ret.eorl = eorl_aux[ret.eofb][ret.epose/24];

	return ret;
}

static Cube
antindex_epud(uint64_t ind)
{
	static bool initialized = false;
	static Cube epud_aux[FACTORIAL8];
	int a[12];
	uint64_t ui;
	CubeArray arr;

	if (!initialized) {
		a[FR] = FR;
		a[FL] = FL;
		a[BL] = BL;
		a[BR] = BR;
		for (ui = 0; ui < FACTORIAL8; ui++) {
			index_to_perm(ui, 8, a);
			arr.ep = a;
			epud_aux[ui] = arrays_to_cube(&arr, pf_ep);
		}

		initialized = true;
	}

	return epud_aux[ind];
}

static Cube
antindex_coud(uint64_t ind)
{
	return (Cube){ .coud = ind, .corl = ind, .cofb = ind };
}

static Cube
antindex_corners(uint64_t ind)
{
	Cube c = {0};

	c.coud = ind / FACTORIAL8;
	c.cp   = ind % FACTORIAL8;

	return c;
}

static Cube
antindex_cp(uint64_t ind)
{
	Cube c = {0};

	c.cp = ind;

	return c;
}

static Cube
antindex_cphtr(uint64_t ind)
{
	return (Cube) { .cp = cphtr_right_rep[ind] };
}

static Cube
antindex_cornershtr(uint64_t ind)
{
	Cube c = antindex_cphtr(ind % (BINOM8ON4 * 6));

	c.coud = ind / (BINOM8ON4 * 6);

	return c;
}

static Cube
antindex_cornershtrfin(uint64_t ind)
{
	return (Cube){ .cp = cornershtrfin_ant[ind] };
}

static Cube
antindex_drud(uint64_t ind)
{
	uint64_t epos, eofb;
	Cube c;

	eofb = ind % POW2TO11;
	epos = ind / (POW2TO11 * POW3TO7);
	c = antindex_eofbepos(eofb + POW2TO11 * epos);

	c.coud  = (ind / POW2TO11) % POW3TO7;

	return c;
}

static Cube
antindex_drud_eofb(uint64_t ind)
{
	return antindex_drud(ind * POW2TO11);
}

static Cube
antindex_htr_drud(uint64_t ind)
{
	Cube ret;

	ret       = antindex_cphtr(ind / BINOM8ON4);
	ret.eposs = (ind % BINOM8ON4) * FACTORIAL4;

	return ret;
}

static Cube
antindex_htrfin(uint64_t ind)
{
	Cube ret;

	ret = antindex_cornershtrfin(ind/(24*24*24));

	ret.eposm = ind % 24;
	ind /= 24;
	ret.eposs = ind % 24;
	ind /= 24;
	ret.epose = ind % 24;

	return ret;
}

static uint64_t
index_eofb(Cube cube)
{
	return cube.eofb;
}

static uint64_t
index_eofbepos(Cube cube)
{
	return (cube.epose / FACTORIAL4) * POW2TO11 + cube.eofb;
}

static uint64_t
index_epud(Cube cube)
{
	uint64_t ret;
	CubeArray *arr = new_cubearray(cube, pf_ep);

	ret = perm_to_index(arr->ep, 8);
	free_cubearray(arr, pf_ep);

	return ret;
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
index_cp(Cube cube)
{
	return cube.cp;
}

static uint64_t
index_cphtr(Cube cube)
{
	return cphtr_right_cosets[cube.cp];
}

static uint64_t
index_cornershtr(Cube cube)
{
	return cube.coud * BINOM8ON4 * 6 + index_cphtr(cube);
}

static uint64_t
index_cornershtrfin(Cube cube)
{
	return cornershtrfin_ind[cube.cp];
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
index_drud_eofb(Cube cube)
{
	return index_drud(cube) / POW2TO11;
}

static uint64_t
index_htr_drud(Cube cube)
{
	return index_cphtr(cube) * BINOM8ON4 +
	       (cube.eposs / FACTORIAL4) % BINOM8ON4;
}

static uint64_t
index_htrfin(Cube cube)
{
	uint64_t epe, eps, epm, cp, ep;

	epe = cube.epose % 24;
	eps = cube.eposs % 24;
	epm = cube.eposm % 24;
	ep = (epe * 24 + eps) *24 + epm;
	cp = index_cornershtrfin(cube);

	return cp * 24 * 24 * 24 + ep;
}

/* Init functions implementation *********************************************/

/*
 * There is certainly a better way to do this, but for now I just use
 * a "graph coloring" algorithm to compute the left cosets, and I compose
 * with every possible cp to get the right cosets (it is possible that I am
 * mixing up left and right).
 * 
 * For doing it better "Mathematically", we need 3 things:
 *   - Checking that cp separates the orbits (UFR,UBL,DFL,DBR) and the other
 *     This is easy and it is done in the commented function cphtr_cp().
 *   - Check that there is no ep/cp parity
 *   - Check that we are not in the "3c" case; this is the part I don't
 *     know how to do.
 */
static void
init_cphtr_cosets()
{
	unsigned int i; 
	int c = 0, d = 0;

	for (i = 0; i < FACTORIAL8; i++) {
		cphtr_left_cosets[i]  = -1;
		cphtr_right_cosets[i] = -1;
	}

	/* First we compute left cosets with a bfs */
	for (i = 0; i < FACTORIAL8; i++)
		if (cphtr_left_cosets[i] == -1)
			init_cphtr_left_cosets_bfs(i, c++);

	/* Then we compute right cosets using compose() */
	for (i = 0; i < FACTORIAL8; i++)
		if (cphtr_right_cosets[i] == -1)
			init_cphtr_right_cosets_color(i, d++);
}

static void
init_cphtr_left_cosets_bfs(int i, int c)
{
	int j, jj, next[FACTORIAL8], next2[FACTORIAL8], n, n2;

	Move k;

	n = 1;
	next[0] = i;
	cphtr_left_cosets[i] = c;

	while (n != 0) {
		for (j = 0, n2 = 0; j < n; j++) {
			for (k = U2; k < B3; k++) {
				if (!moveset_htr(k))
					continue;
				jj = apply_move(k, (Cube){ .cp = next[j] }).cp;

				if (cphtr_left_cosets[jj] == -1) {
					cphtr_left_cosets[jj] = c;
					next2[n2++] = jj;
				}
			}
		}

		for (j = 0; j < n2; j++)
			next[j] = next2[j];
		n = n2;
	}
}

static void
init_cphtr_right_cosets_color(int i, int d)
{
	int cp;
	unsigned int j;

	cphtr_right_rep[d] = i;
	for (j = 0; j < FACTORIAL8; j++) {
		if (cphtr_left_cosets[j] == 0) {
			cp = compose((Cube){.cp = i}, (Cube){.cp = j}).cp;
			cphtr_right_cosets[cp] = d;
		}
	}
}

static void
init_cornershtrfin()
{
	unsigned int i, j;
	int n, c;
	Move m;

	for (i = 0; i < FACTORIAL8; i++)
		cornershtrfin_ind[i] = -1;
	cornershtrfin_ind[0] = 0;

	/* 10-pass, I think 5 is enough, but just in case */
	n = 1;
	for (i = 0; i < 10; i++) {
		for (j = 0; j < FACTORIAL8; j++) {
			if (cornershtrfin_ind[j] == -1)
				continue;
			for (m = U; m < NMOVES; m++) {
				if (moveset_htr(m)) {
					c = apply_move(m, (Cube){.cp = j}).cp;
					if (cornershtrfin_ind[c] == -1) {
						cornershtrfin_ind[c] = n;
						cornershtrfin_ant[n] = c;
						n++;
					}
				}
			}
		}
	}
}

void
init_coord()
{
	static bool initialized = false;
	if (initialized)
		return;
	initialized = true;

	init_trans();

	init_cphtr_cosets();
	init_cornershtrfin();
}

