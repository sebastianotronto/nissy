#include "coord.h"

static Cube        admissible_eos_from_eofbepos(Cube cube);
static Cube        antindex_eofb(uint64_t ind);
static Cube        antindex_eofbepos(uint64_t ind);
static Cube        antindex_epud(uint64_t ind);
static Cube        antindex_coud(uint64_t ind);
static Cube        antindex_corners(uint64_t ind);
static Cube        antindex_cp(uint64_t ind);
static Cube        antindex_cornershtr(uint64_t ind);
static Cube        antindex_drud(uint64_t ind);
static Cube        antindex_coud_sym16(uint64_t ind);
static Cube        antindex_cp_sym16(uint64_t ind);
static Cube        antindex_eofbepos_sym16(uint64_t ind);
static Cube        antindex_drud_sym16(uint64_t ind);
static Cube        antindex_drud_eofb(uint64_t ind);
static Cube        antindex_drudfin_noE_sym16(uint64_t ind);
static Cube        antindex_htrfin(uint64_t ind);
static Cube        antindex_khuge(uint64_t ind);
static int         epos_dependent_pos(int pos1, int pos2);
static void        gensym(SymData *sd);
static uint64_t    index_eofb(Cube cube);
static uint64_t    index_eofbepos(Cube cube);
static uint64_t    index_epud(Cube cube);
static uint64_t    index_coud(Cube cube);
static uint64_t    index_corners(Cube cube);
static uint64_t    index_cp(Cube cube);
static uint64_t    index_cornershtr(Cube cube);
static uint64_t    index_drud(Cube cube);
static uint64_t    index_coud_sym16(Cube cube);
static uint64_t    index_cp_sym16(Cube cube);
static uint64_t    index_eofbepos_sym16(Cube cube);
static uint64_t    index_drud_sym16(Cube cube);
static uint64_t    index_drud_eofb(Cube cube);
static uint64_t    index_drudfin_noE_sym16(Cube cube);
static uint64_t    index_htrfin(Cube cube);
static uint64_t    index_khuge(Cube cube);
static void        init_cphtr_cosets();
static void        init_cphtr_left_cosets_bfs(int i, int c);
static void        init_cphtr_right_cosets_color(int i, int c);
static void        init_symdata();
static bool        read_symdata_file(SymData *sd);
static bool        write_symdata_file(SymData *sd);

/* All sorts of useful costants and tables  **********************************/

/* TODO: Can I move inside functions that use them?
	 Maybe I need to pass them as argument to some
	 secondary function */
static int              cphtr_left_cosets[FACTORIAL8];
static int              cphtr_right_cosets[FACTORIAL8];
static int              cphtr_right_rep[BINOM8ON4*6];


/* Symmetry data for some coordinates ****************************************/

static Trans
trans_group_trivial[1] = { uf };

static Trans
trans_group_udfix[16] = {
	uf, ur, ub, ul,
	df, dr, db, dl,
	uf_mirror, ur_mirror, ub_mirror, ul_mirror,
	df_mirror, dr_mirror, db_mirror, dl_mirror,
};

SymData
sd_coud_16 = {
	.filename  = "sd_coud_16",
	.coord     = &coord_coud,
	.sym_coord = &coord_coud_sym16,
	.ntrans    = 16,
	.trans     = trans_group_udfix
};

SymData
sd_cp_16 = {
	.filename  = "sd_cp_16",
	.coord     = &coord_cp,
	.sym_coord = &coord_cp_sym16,
	.ntrans    = 16,
	.trans     = trans_group_udfix
};

SymData
sd_eofbepos_16 = {
	.filename  = "sd_eofbepos_16",
	.coord     = &coord_eofbepos,
	.sym_coord = &coord_eofbepos_sym16,
	.ntrans    = 16,
	.trans     = trans_group_udfix
};

static int        n_all_symdata = 3;
static SymData *  all_sd[3]     = {
	&sd_coud_16,
	&sd_cp_16,
	&sd_eofbepos_16,
};

/* Coordinates and their implementation **************************************/

Coordinate
coord_eofb = {
	.index  = index_eofb,
	.cube   = antindex_eofb,
	.check  = check_eofb,
	.max    = POW2TO11,
	.ntrans = 1,
	.trans  = trans_group_trivial,
};

Coordinate
coord_eofbepos = {
	.index  = index_eofbepos,
	.cube   = antindex_eofbepos,
	.check  = check_eofbepos,
	.max    = POW2TO11 * BINOM12ON4,
	.ntrans = 1,
	.trans  = trans_group_trivial,
};

Coordinate
coord_coud = {
	.index  = index_coud,
	.cube   = antindex_coud,
	.check  = check_coud,
	.max    = POW3TO7,
	.ntrans = 1,
	.trans  = trans_group_trivial,
};

Coordinate
coord_corners = {
	.index  = index_corners,
	.cube   = antindex_corners,
	.check  = check_corners,
	.max    = POW3TO7 * FACTORIAL8,
	.ntrans = 1,
	.trans  = trans_group_trivial,
};

Coordinate
coord_cp = {
	.index  = index_cp,
	.cube   = antindex_cp,
	.check  = check_cp,
	.max    = FACTORIAL8,
	.ntrans = 1,
	.trans  = trans_group_trivial,
};

Coordinate
coord_cornershtr = {
	.index  = index_cornershtr,
	.cube   = antindex_cornershtr,
	.check  = check_cornershtr,
	.max    = POW3TO7 * BINOM8ON4 * 6,
	.ntrans = 1,
	.trans  = trans_group_trivial,
};

Coordinate
coord_drud = {
	.index  = index_drud,
	.cube   = antindex_drud,
	.check  = check_drud,
	.max    = POW2TO11 * POW3TO7 * BINOM12ON4,
	.ntrans = 1,
	.trans  = trans_group_trivial,
};

Coordinate
coord_htrfin = {
	.index  = index_htrfin,
	.cube   = antindex_htrfin,
	.check  = check_htrfin,
	.max    = 24 * 24 * 24 *24 * 24,
	.ntrans = 1,
	.trans  = trans_group_trivial,
};

Coordinate
coord_drud_eofb = {
	.index  = index_drud_eofb,
	.cube   = antindex_drud_eofb,
	.check  = check_drud,
	.max    = POW3TO7 * BINOM12ON4,
	.ntrans = 1,
	.trans  = trans_group_trivial,
};

Coordinate
coord_eofbepos_sym16 = {
	.index  = index_eofbepos_sym16,
	.cube   = antindex_eofbepos_sym16,
	.check  = check_eofbepos,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

Coordinate
coord_coud_sym16 = {
	.index  = index_coud_sym16,
	.cube   = antindex_coud_sym16,
	.check  = check_coud,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

Coordinate
coord_cp_sym16 = {
	.index  = index_cp_sym16,
	.cube   = antindex_cp_sym16,
	.check  = check_cp,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

Coordinate
coord_drud_sym16 = {
	.index  = index_drud_sym16,
	.cube   = antindex_drud_sym16,
	.check  = check_drud,
	.max    = POW3TO7 * 64430,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

Coordinate
coord_drudfin_noE_sym16 = {
	.index  = index_drudfin_noE_sym16,
	.cube   = antindex_drudfin_noE_sym16,
	.check  = check_drudfin_noE,
	.max    = FACTORIAL8 * 2768,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

Coordinate
coord_khuge = {
	.index  = index_khuge,
	.cube   = antindex_khuge,
	.check  = check_khuge,
	.max    = POW3TO7 * FACTORIAL4 * 64430,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

/* Functions *****************************************************************/

static Cube
admissible_eos_from_eofbepos(Cube cube)
{
	Edge e;
	Cube ret;
	CubeArray *arr = new_cubearray(cube, pf_all);

	memcpy(arr->eorl, arr->eofb, 12 * sizeof(int));
	memcpy(arr->eoud, arr->eofb, 12 * sizeof(int));

	for (e = 0; e < 12; e++) {
		if ((edge_slice(e) != 0 && edge_slice(arr->ep[e]) == 0) ||
		    (edge_slice(e) == 0 && edge_slice(arr->ep[e]) != 0))
			arr->eorl[e] = 1 - arr->eorl[e];
		if ((edge_slice(e) != 2 && edge_slice(arr->ep[e]) == 2) ||
		    (edge_slice(e) == 2 && edge_slice(arr->ep[e]) != 2))
			arr->eoud[e] = 1 - arr->eoud[e];
	}

	ret = arrays_to_cube(arr, pf_all);
	free_cubearray(arr, pf_all);

	return ret;
}


static Cube
antindex_eofb(uint64_t ind)
{
	return (Cube){ .eofb = ind, .eorl = ind, .eoud = ind };
}

static Cube
antindex_eofbepos(uint64_t ind)
{
	static bool initialized = false;
	static Cube admissible_ee_aux[POW2TO11*BINOM12ON4];
	static Cube c1;
	static int k;
	static uint64_t ui;

	if (!initialized) {
		for (ui = 0; ui < POW2TO11*BINOM12ON4; ui++) {
			k = (ui / POW2TO11) * 24;
			c1 = admissible_ep((Cube){ .epose = k }, pf_e);
			c1.eofb  = ui % POW2TO11;
			c1 = admissible_eos_from_eofbepos(c1);
			admissible_ee_aux[ui] = c1;
		}

		initialized = true;
	}

	return admissible_ee_aux[ind];
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
antindex_cornershtr(uint64_t ind)
{
	Cube c = anti_cphtr(ind % (BINOM8ON4 * 6));

	c.coud = ind / (BINOM8ON4 * 6);

	return c;
}

/* TODO: admissible eos and cos */
/* DONE: temporary fix, make it better */
/* Or maybe it's ok like this? */
static Cube
antindex_drud(uint64_t ind)
{
	uint64_t epos, eofb;
	Cube c;

	eofb = ind % POW2TO11;
	epos = ind / (POW2TO11 * POW3TO7);
	c = antindex_eofbepos(eofb + POW2TO11 * epos);

	c.coud  = (ind / POW2TO11) % POW3TO7;
	c.corl  = c.coud;
	c.cofb  = c.coud;

	return c;
}

static Cube
antindex_drud_eofb(uint64_t ind)
{
	return antindex_drud(ind * POW2TO11);
}

static Cube
antindex_coud_sym16(uint64_t ind)
{
	return sd_coud_16.rep[ind];
}

static Cube
antindex_cp_sym16(uint64_t ind)
{
	return sd_cp_16.rep[ind];
}

static Cube
antindex_eofbepos_sym16(uint64_t ind)
{
	return sd_eofbepos_16.rep[ind];
}

static Cube
antindex_drud_sym16(uint64_t ind)
{
	Cube c;

	c = sd_eofbepos_16.rep[ind/POW3TO7];
	c.coud = ind % POW3TO7;
	c.cofb = c.coud;
	c.corl = c.coud;

	return c;
}

static Cube
antindex_drudfin_noE_sym16(uint64_t ind)
{
	Cube c1, c2;

	c1 = antindex_epud(ind % FACTORIAL8);
	c2 = sd_cp_16.rep[ind/FACTORIAL8];
	c1.cp = c2.cp;

	return c1;
}

static Cube
antindex_htrfin(uint64_t ind)
{
	Cube ret = {0};
	uint64_t cp1, cp2;

	static bool initialized = false;
	static int i, j, k, c[8], c1[4], c2[4], cp[24][24];
	static int c1solved[4] = {UFR, UBL, DFL, DBR};
	static int c2solved[4] = {UFL, UBR, DFR, DBL};

	if (!initialized) {
		for (i = 0; i < 24; i++) {
			for (j = 0; j < 24; j++) {
				index_to_perm(i, 4, c1);
				index_to_perm(j, 4, c2);
				for (k = 0; k < 8; k++)
					if (k == UFR || k == UBL ||
					    k == DFL || k == DBR)
						c[k] = c1[c1solved[k/2]];
					else
						c[k] = c2[c2solved[k/2]];

				cp[i][j] = perm_to_index(c, 8);
			}
		}

		initialized = true;
	}

	cp2 = ind % 24;
	ind /= 24;
	cp1 = ind % 24;
	ret.cp = cp[cp1][cp2];

	ind /= 24;
	ret.eposm = ind % 24;
	ind /= 24;
	ret.eposs = ind % 24;
	ret.epose = ind / 24;

	return ret;
}

static Cube
antindex_khuge(uint64_t ind)
{
	Cube c;

	c = sd_eofbepos_16.rep[ind/(FACTORIAL4*POW3TO7)];
	c.epose = ((c.epose / 24) * 24) + ((ind/POW3TO7) % 24);
	c.coud = ind % POW3TO7;

	return c;
}

bool
check_centers(Cube cube)
{
	return cube.cpos == 0;
}

bool
check_corners(Cube cube)
{
	return cube.cp == 0 && cube.coud == 0;
}

bool
check_cp(Cube cube)
{
	return cube.cp == 0;
}

bool
check_cornershtr(Cube cube)
{
	return cube.coud == 0 && cphtr(cube) == 0; /* TODO: use array cphtrcosets*/
}

bool
check_coud(Cube cube)
{
	return cube.coud == 0;
}

bool
check_drud(Cube cube)
{
	return cube.eofb == 0 && cube.eorl == 0 && cube.coud == 0;
}

bool
check_htr(Cube cube)
{
	return check_cornershtr(cube) &&
	       cube.eofb == 0 && cube.eorl == 0 && cube.eoud == 0;
}

bool
check_htrfin(Cube cube)
{
	return cube.cp == 0 &&
	       cube.epose == 0 && cube.eposs == 0 && cube.eposm == 0;
}

bool
check_drudfin_noE(Cube cube)
{
	return cube.eposs == 0 && cube.eposm == 0 && cube.cp == 0;
}

bool
check_eofb(Cube cube)
{
	return cube.eofb == 0;
}

bool
check_eofbepos(Cube cube)
{
	return cube.eofb == 0 && cube.epose / 24 == 0;
}

bool
check_epose(Cube cube)
{
	return cube.epose == 0;
}

bool
check_ep(Cube cube)
{
	return cube.epose == 0 && cube.eposs == 0 && cube.eposm == 0;
}

bool
check_khuge(Cube cube)
{
	return check_drud(cube) && cube.epose % 24 == 0;
}

bool
check_nothing(Cube cube)
{
	return is_admissible(cube); /*TODO: maybe change?*/
}

static int
epos_dependent_pos(int poss, int pose)
{
	static int epe_solved[4] = {FR, FL, BL, BR};
	static int eps_solved[4] = {UL, UR, DL, DR};
	int ep[12] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	int ep8[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int i, j;

	epos_to_partial_ep(poss*FACTORIAL4, ep, eps_solved);
	epos_to_partial_ep(pose*FACTORIAL4, ep, epe_solved);

	for (i = 0, j = 0; i < 12; i++)
		if (edge_slice(ep[i]) != 0)
			ep8[j++] = (edge_slice(ep[i]) == 1) ? 1 : 0;

	swap(&ep8[1], &ep8[4]);
	swap(&ep8[3], &ep8[6]);

	return subset_to_index(ep8, 8, 4);
}

static void
gensym(SymData *sd)
{
	uint64_t i, in, nreps = 0;
	int j;
	Cube c, d;

	if (sd->generated)
		return;

	sd->class      = malloc(sd->coord->max * sizeof(uint64_t));
	sd->rep        = malloc(sd->coord->max * sizeof(Cube));
	sd->transtorep = malloc(sd->coord->max * sizeof(Trans));

	if (read_symdata_file(sd)) {
		sd->generated = true;
		return;
	}

	fprintf(stderr, "Cannot load %s, generating it\n", sd->filename);

	for (i = 0; i < sd->coord->max; i++)
		sd->class[i] = sd->coord->max + 1;

	for (i = 0; i < sd->coord->max; i++) {
		if (sd->class[i] == sd->coord->max + 1) {
			c = sd->coord->cube(i);
			sd->rep[nreps] = c;
			for (j = 0; j < sd->ntrans; j++) {
				d = apply_trans(sd->trans[j], c);
				in = sd->coord->index(d);

				if (sd->class[in] == sd->coord->max + 1) {
					sd->class[in] = nreps;
					sd->transtorep[in] =
						inverse_trans(sd->trans[j]);
				}
			}
			nreps++;
		}
	}

	sd->sym_coord->max = nreps;
	sd->rep            = realloc(sd->rep, nreps * sizeof(Cube));
	sd->generated      = true;

	fprintf(stderr, "Found %lu classes\n", nreps);

	if (!write_symdata_file(sd))
		fprintf(stderr, "Error writing SymData file\n");

	return;
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
index_cornershtr(Cube cube)
{
	return cube.coud * BINOM8ON4 * 6 + cphtr(cube);
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
index_coud_sym16(Cube cube)
{
	return sd_coud_16.class[index_coud(cube)];
}

static uint64_t
index_cp_sym16(Cube cube)
{
	return sd_cp_16.class[index_cp(cube)];
}

static uint64_t
index_drud_sym16(Cube cube)
{
	Trans t;
	Cube c;

	t = sd_eofbepos_16.transtorep[index_eofbepos(cube)];
	c = apply_trans(t, cube);

	return index_eofbepos_sym16(c) * POW3TO7 + c.coud;
}

static uint64_t
index_drudfin_noE_sym16(Cube cube)
{
	Trans t;
	Cube c;

	t = sd_cp_16.transtorep[index_cp(cube)];
	c = apply_trans(t, cube);

	return index_cp_sym16(c) * FACTORIAL8 + index_epud(c);
}

static uint64_t
index_htrfin(Cube cube)
{
	uint64_t epe, eps, epm, cp, ep;

	static bool initialized = false;
	static uint64_t cp1[FACTORIAL8], cp2[FACTORIAL8];
	static unsigned int i;
	static int j, n1, n2, c[8], c1[4], c2[4];

	if (!initialized) {
		for (i = 0; i < FACTORIAL8; i++) {
			index_to_perm(i, 8, c);
			n1 = 0;
			n2 = 0;
			for (j = 0; j < 8; j++)
				if (c[j] == UFR || c[j] == UBL ||
				    c[j] == DFL || c[j] == DBR)
					c1[n1++] = c[j] / 2;
				else
					c2[n2++] = c[j] / 2;

			cp1[i] = perm_to_index(c1, 4);
			cp2[i] = perm_to_index(c2, 4);
		}

		initialized = true;
	}

	epe = cube.epose % 24;
	eps = cube.eposs % 24;
	epm = cube.eposm % 24;

	cp = cp1[cube.cp] * 24 + cp2[cube.cp];
	ep = (epe * 24 + eps) *24 + epm;

	return ep * 24 * 24 + cp;
}

static uint64_t
index_eofbepos_sym16(Cube cube)
{
	return sd_eofbepos_16.class[index_eofbepos(cube)];
}

static uint64_t
index_khuge(Cube cube)
{
	Trans t;
	Cube c;
	uint64_t a;

	t = sd_eofbepos_16.transtorep[index_eofbepos(cube)];
	c = apply_trans(t, cube);
	a = (index_eofbepos_sym16(c) * 24) + (c.epose % 24);

	return a * POW3TO7 + c.coud;
}

static bool
read_symdata_file(SymData *sd)
{
	init_env();

	FILE *f;
	char fname[strlen(tabledir)+100];
	uint64_t n = sd->coord->max, *sn = &sd->sym_coord->max;
	bool r = true;

	strcpy(fname, tabledir);
	strcat(fname, "/");
	strcat(fname, sd->filename);

	if ((f = fopen(fname, "rb")) == NULL)
		return false;

	r = r && fread(&sd->sym_coord->max, sizeof(uint64_t), 1,   f) == 1;
	r = r && fread(sd->rep,             sizeof(Cube),     *sn, f) == *sn;
	r = r && fread(sd->class,           sizeof(uint64_t), n,   f) == n;
	r = r && fread(sd->transtorep,      sizeof(Trans),    n,   f) == n;

	fclose(f);
	return r;
}

static bool
write_symdata_file(SymData *sd)
{
	init_env();

	FILE *f;
	char fname[strlen(tabledir)+100];
	uint64_t n = sd->coord->max, *sn = &sd->sym_coord->max;
	bool r = true;

	strcpy(fname, tabledir);
	strcat(fname, "/");
	strcat(fname, sd->filename);

	if ((f = fopen(fname, "wb")) == NULL)
		return false;

	r = r && fwrite(&sd->sym_coord->max, sizeof(uint64_t), 1,   f) == 1;
	r = r && fwrite(sd->rep,             sizeof(Cube),     *sn, f) == *sn;
	r = r && fwrite(sd->class,           sizeof(uint64_t), n,   f) == n;
	r = r && fwrite(sd->transtorep,      sizeof(Trans),    n,   f) == n;

	fclose(f);
	return r;
}

/* Init functions implementation *********************************************/

/*
 * There is certainly a bette way to do this, but for now I just use
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
	int j, jj, k, next[FACTORIAL8], next2[FACTORIAL8], n, n2;
	Move moves[6] = {U2, D2, R2, L2, F2, B2};

	n = 1;
	next[0] = i;
	cphtr_left_cosets[i] = c;

	while (n != 0) {
		for (j = 0, n2 = 0; j < n; j++) {
			for (k = 0; k < 6; k++) {
				/*jj = cp_mtable[moves[k]][next[j]];*/
				/* TODO fix formatting */
				jj = apply_move(moves[k], (Cube){.cp=next[j]}).cp;
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
			/* TODO: use antindexer, it's nicer */
			cp = compose((Cube){.cp = i}, (Cube){.cp = j}).cp;
			cphtr_right_cosets[cp] = d;
		}
	}
}

static void
init_symdata()
{
	int i;

	for (i = 0; i < n_all_symdata; i++)
		gensym(all_sd[i]);
}

/*TODO maybe move the next two */
uint64_t
cphtr(Cube cube)
{
	return cphtr_right_cosets[cube.cp];
}

Cube
anti_cphtr(uint64_t ind)
{
	return (Cube) { .cp = cphtr_right_rep[ind] };
}

uint64_t
epos_dependent(Cube c)
{
	static int initialized = false;
	static int aux[BINOM12ON4][BINOM12ON4];
	static uint64_t ui, uj;

	if (!initialized) {
		for (ui = 0; ui < BINOM12ON4; ui++)
			for (uj = 0; uj < BINOM12ON4; uj++)
				aux[ui][uj] = epos_dependent_pos(ui, uj);

		initialized = true;
	}

	return aux[c.eposs/FACTORIAL4][c.epose/FACTORIAL4];
}

void
init_coord()
{
	static bool initialized = false;
	if (initialized)
		return;
	initialized = true;

	init_cphtr_cosets();
	init_symdata();
}

