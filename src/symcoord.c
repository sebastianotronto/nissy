#include "symcoord.h"

/* These constants have been computed generating the respective SymData */
#define CLASSES_CP_16        2768
#define CLASSES_EOFBEPOS_16  64430

static Cube        antindex_cp_sym16(uint64_t ind);
static Cube        antindex_eofbepos_sym16(uint64_t ind);
static Cube        antindex_drud_sym16(uint64_t ind);
static Cube        antindex_drudfin_noE_sym16(uint64_t ind);
static Cube        antindex_khuge(uint64_t ind);
static Cube        antindex_nxopt31(uint64_t ind);

static uint64_t    index_cp_sym16(Cube cube);
static uint64_t    index_eofbepos_sym16(Cube cube);
static uint64_t    index_drud_sym16(Cube cube);
static uint64_t    index_drudfin_noE_sym16(Cube cube);
static uint64_t    index_khuge(Cube cube);
static uint64_t    index_nxopt31(Cube cube);

static int         transfinder_drud_sym16(uint64_t ind, Trans *ret);
static int         transfinder_drudfin_noE_sym16(uint64_t ind, Trans *ret);
static int         transfinder_khuge(uint64_t ind, Trans *ret);
static int         transfinder_nxopt31(uint64_t ind, Trans *ret);

static void        gensym(SymData *sd);
static bool        read_symdata_file(SymData *sd);
static int         selfsims(SymData *sd, uint64_t ind, Trans *ret);
static bool        write_symdata_file(SymData *sd);

/* Transformation groups and symmetry data ***********************************/

static Trans
trans_group_udfix[16] = {
	uf, ur, ub, ul,
	df, dr, db, dl,
	uf_mirror, ur_mirror, ub_mirror, ul_mirror,
	df_mirror, dr_mirror, db_mirror, dl_mirror,
};

static SymData
sd_cp_16 = {
	.filename  = "sd_cp_16",
	.coord     = &coord_cp,
	.sym_coord = &coord_cp_sym16,
	.ntrans    = 16,
	.trans     = trans_group_udfix
};

static SymData
sd_eofbepos_16 = {
	.filename  = "sd_eofbepos_16",
	.coord     = &coord_eofbepos,
	.sym_coord = &coord_eofbepos_sym16,
	.ntrans    = 16,
	.trans     = trans_group_udfix
};

static int nsymdata = 2;
static SymData * all_sd[] = {
	&sd_cp_16,
	&sd_eofbepos_16,
};


/* Coordinates and their implementation **************************************/

Coordinate
coord_eofbepos_sym16 = {
	.index  = index_eofbepos_sym16,
	.cube   = antindex_eofbepos_sym16,
};

Coordinate
coord_cp_sym16 = {
	.index  = index_cp_sym16,
	.cube   = antindex_cp_sym16,
};

Coordinate
coord_drud_sym16 = {
	.index  = index_drud_sym16,
	.cube   = antindex_drud_sym16,
	.max    = POW3TO7 * CLASSES_EOFBEPOS_16,
	.trans  = transfinder_drud_sym16,
};

Coordinate
coord_drudfin_noE_sym16 = {
	.index  = index_drudfin_noE_sym16,
	.cube   = antindex_drudfin_noE_sym16,
	.max    = FACTORIAL8 * CLASSES_CP_16,
	.trans  = transfinder_drudfin_noE_sym16,
};

Coordinate
coord_khuge = {
	.index  = index_khuge,
	.cube   = antindex_khuge,
	.max    = POW3TO7 * FACTORIAL4 * CLASSES_EOFBEPOS_16,
	.trans  = transfinder_khuge,
};

Coordinate
coord_nxopt31 = {
	.index  = index_nxopt31,
	.cube   = antindex_nxopt31,
	.max    = POW3TO7 * BINOM8ON4 * CLASSES_EOFBEPOS_16 ,
	.trans  = transfinder_nxopt31,
};

/* Functions *****************************************************************/

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

	c = antindex_eofbepos_sym16(ind/POW3TO7);
	c.coud = ind % POW3TO7;
	c.cofb = c.coud;
	c.corl = c.coud;

	return c;
}

static Cube
antindex_drudfin_noE_sym16(uint64_t ind)
{
	Cube c1, c2;

	c1 = coord_epud.cube(ind % FACTORIAL8);
	c2 = antindex_cp_sym16(ind/FACTORIAL8);
	c1.cp = c2.cp;

	return c1;
}

static Cube
antindex_khuge(uint64_t ind)
{
	Cube c;

	c = antindex_eofbepos_sym16(ind/(FACTORIAL4*POW3TO7));
	c.epose = ((c.epose / 24) * 24) + ((ind/POW3TO7) % 24);
	c.coud = ind % POW3TO7;

	return c;
}

static Cube
antindex_nxopt31(uint64_t ind)
{
	Cube c;

	c = antindex_eofbepos_sym16(ind/(BINOM8ON4*POW3TO7));
	c.cp = coord_cpud_separate.cube((ind/POW3TO7)%BINOM8ON4).cp;
	c.coud = ind % POW3TO7;

	return c;
}

static uint64_t
index_cp_sym16(Cube cube)
{
	return sd_cp_16.class[coord_cp.index(cube)];
}

static uint64_t
index_drud_sym16(Cube cube)
{
	Trans t;
	Cube c;

	t = sd_eofbepos_16.transtorep[coord_eofbepos.index(cube)];
	c = apply_trans(t, cube);

	return index_eofbepos_sym16(c) * POW3TO7 + c.coud;
}

static uint64_t
index_drudfin_noE_sym16(Cube cube)
{
	Trans t;
	Cube c;

	t = sd_cp_16.transtorep[coord_cp.index(cube)];
	c = apply_trans(t, cube);

	return index_cp_sym16(c) * FACTORIAL8 + coord_epud.index(c);
}

static uint64_t
index_eofbepos_sym16(Cube cube)
{
	return sd_eofbepos_16.class[coord_eofbepos.index(cube)];
}

static uint64_t
index_khuge(Cube cube)
{
	Trans t;
	Cube c;
	uint64_t a;

	t = sd_eofbepos_16.transtorep[coord_eofbepos.index(cube)];
	c = apply_trans(t, cube);
	a = (index_eofbepos_sym16(c) * 24) + (c.epose % 24);

	return a * POW3TO7 + c.coud;
}

static uint64_t
index_nxopt31(Cube cube)
{
	Trans t;
	Cube c;
	uint64_t a;

	t = sd_eofbepos_16.transtorep[coord_eofbepos.index(cube)];
	c = apply_trans(t, cube);
	a = (index_eofbepos_sym16(c)*BINOM8ON4) + coord_cpud_separate.index(c);

	return a * POW3TO7 + c.coud;
}

static int
transfinder_drud_sym16(uint64_t ind, Trans *ret)
{
	uint64_t i, trueind;
	int j;
	static bool initialized = false;
	static int naux[CLASSES_EOFBEPOS_16];
	static Trans retaux[CLASSES_EOFBEPOS_16][NTRANS];

	if (!initialized) {
		for (i = 0; i < CLASSES_EOFBEPOS_16; i++)
			naux[i] = selfsims(&sd_eofbepos_16, i, retaux[i]);

		initialized = true;
	}

	trueind = ind/POW3TO7;
	for (j = 0; j < naux[trueind]; j++)
		ret[j] = retaux[trueind][j];
	return naux[trueind];
}

static int
transfinder_drudfin_noE_sym16(uint64_t ind, Trans *ret)
{
	uint64_t i, trueind;
	int j;
	static bool initialized = false;
	static int naux[CLASSES_CP_16];
	static Trans retaux[CLASSES_CP_16][NTRANS];

	if (!initialized) {
		for (i = 0; i < CLASSES_CP_16; i++)
			naux[i] = selfsims(&sd_cp_16, i, retaux[i]);

		initialized = true;
	}

	trueind = ind/FACTORIAL8;
	for (j = 0; j < naux[trueind]; j++)
		ret[j] = retaux[trueind][j];
	return naux[trueind];
}

static int
transfinder_khuge(uint64_t ind, Trans *ret)
{
	uint64_t i, trueind;
	int j;
	static bool initialized = false;
	static int naux[CLASSES_EOFBEPOS_16];
	static Trans retaux[CLASSES_EOFBEPOS_16][NTRANS];

	if (!initialized) {
		for (i = 0; i < CLASSES_EOFBEPOS_16; i++)
			naux[i] = selfsims(&sd_eofbepos_16, i, retaux[i]);

		initialized = true;
	}

	trueind = ind/(FACTORIAL4*POW3TO7);
	for (j = 0; j < naux[trueind]; j++)
		ret[j] = retaux[trueind][j];
	return naux[trueind];
}

static int
transfinder_nxopt31(uint64_t ind, Trans *ret)
{
	uint64_t i, trueind;
	int j;
	static bool initialized = false;
	static int naux[CLASSES_EOFBEPOS_16];
	static Trans retaux[CLASSES_EOFBEPOS_16][NTRANS];

	if (!initialized) {
		for (i = 0; i < CLASSES_EOFBEPOS_16; i++)
			naux[i] = selfsims(&sd_eofbepos_16, i, retaux[i]);

		initialized = true;
	}

	trueind = ind/(BINOM8ON4*POW3TO7);
	for (j = 0; j < naux[trueind]; j++)
		ret[j] = retaux[trueind][j];
	return naux[trueind];
}

/* Other functions ***********************************************************/

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

	fprintf(stderr, "Found %" PRIu64 " classes\n", nreps);

	if (!write_symdata_file(sd))
		fprintf(stderr, "Error writing SymData file\n");

	return;
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

static int
selfsims(SymData *sd, uint64_t ind, Trans *ret)
{
	Cube cube, tcube;
	int i, n;
	uint64_t indnosym;

	cube = sd->sym_coord->cube(ind);
	indnosym = sd->coord->index(cube);
	n = 0;
	for (i = 0; i < sd->ntrans; i++) {
		tcube = apply_trans(sd->trans[i], cube);
		if (sd->coord->index(tcube) == indnosym)
			ret[n++] = sd->trans[i];
	}

	return n;
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

void
init_symcoord()
{
	int i;

	static bool initialized = false;
	if (initialized)
		return;
	initialized = true;

	init_coord();

	for (i = 0; i < nsymdata; i++)
		gensym(all_sd[i]);
}

