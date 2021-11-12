#include "symcoord.h"

static Cube        antindex_coud_sym16(uint64_t ind);
static Cube        antindex_cp_sym16(uint64_t ind);
static Cube        antindex_eofbepos_sym16(uint64_t ind);
static Cube        antindex_drud_sym16(uint64_t ind);
static Cube        antindex_drudfin_noE_sym16(uint64_t ind);
static Cube        antindex_khuge(uint64_t ind);

static uint64_t    index_coud_sym16(Cube cube);
static uint64_t    index_cp_sym16(Cube cube);
static uint64_t    index_eofbepos_sym16(Cube cube);
static uint64_t    index_drud_sym16(Cube cube);
static uint64_t    index_drudfin_noE_sym16(Cube cube);
static uint64_t    index_khuge(Cube cube);

static void        gensym(SymData *sd);
static bool        read_symdata_file(SymData *sd);
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
sd_coud_16 = {
	.filename  = "sd_coud_16",
	.coord     = &coord_coud,
	.sym_coord = &coord_coud_sym16,
	.ntrans    = 16,
	.trans     = trans_group_udfix
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

static int nsymdata = 3;
static SymData * all_sd[] = {
	&sd_coud_16,
	&sd_cp_16,
	&sd_eofbepos_16,
};


/* Coordinates and their implementation **************************************/

Coordinate
coord_eofbepos_sym16 = {
	.index  = index_eofbepos_sym16,
	.cube   = antindex_eofbepos_sym16,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

Coordinate
coord_coud_sym16 = {
	.index  = index_coud_sym16,
	.cube   = antindex_coud_sym16,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

Coordinate
coord_cp_sym16 = {
	.index  = index_cp_sym16,
	.cube   = antindex_cp_sym16,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

Coordinate
coord_drud_sym16 = {
	.index  = index_drud_sym16,
	.cube   = antindex_drud_sym16,
	.max    = POW3TO7 * 64430,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

Coordinate
coord_drudfin_noE_sym16 = {
	.index  = index_drudfin_noE_sym16,
	.cube   = antindex_drudfin_noE_sym16,
	.max    = FACTORIAL8 * 2768,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

Coordinate
coord_khuge = {
	.index  = index_khuge,
	.cube   = antindex_khuge,
	.max    = POW3TO7 * FACTORIAL4 * 64430,
	.ntrans = 16,
	.trans  = trans_group_udfix,
};

/* Functions *****************************************************************/

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

static uint64_t
index_coud_sym16(Cube cube)
{
	return sd_coud_16.class[coord_coud.index(cube)];
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
		
			/*
			 * TODO: this is the only unavoidable use of
			 * antindexes. I also use them in genptable() (see
			 * pruning.c), but there I can do without (see
			 * commented functions in that file.
			 * Removing this would allow for a great simplification
			 */
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

