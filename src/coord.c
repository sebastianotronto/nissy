#define COORD_C

#include "coord.h"

static void        gen_coord_comp(Coordinate *coord);
static void        gen_coord_sym(Coordinate *coord);
static bool        read_coord_mtable(Coordinate *coord);
static bool        read_coord_sd(Coordinate *coord);
static bool        read_coord_ttable(Coordinate *coord);
static bool        write_coord_mtable(Coordinate *coord);
static bool        write_coord_sd(Coordinate *coord);
static bool        write_coord_ttable(Coordinate *coord);

/* Indexers ******************************************************************/

uint64_t
index_eofb(Cube *cube)
{
	return (uint64_t)digit_array_to_int(cube->eo, 11, 2);
}

uint64_t
index_coud(Cube *cube)
{
	return (uint64_t)digit_array_to_int(cube->co, 7, 3);
}

uint64_t
index_cp(Cube *cube)
{
	return (uint64_t)perm_to_index(cube->cp, 8);
}

uint64_t
index_cpudsep(Cube *cube)
{
	int i, c[8];

	for (i = 0; i < 8; i++)
		c[i] = cube->cp[i] < 4 ? 0 : 1;

	return (uint64_t)subset_to_index(c, 8, 4);
}

uint64_t
index_epe(Cube *cube)
{
	int i, e[4];

	for (i = 0; i < 4; i++)
		e[i] = cube->ep[i+8] - 8;

	return (uint64_t)perm_to_index(e, 4);
}

uint64_t
index_epud(Cube *cube)
{
	return (uint64_t)perm_to_index(cube->ep, 8);
}

uint64_t
index_epos(Cube *cube)
{
	int i, a[12];

	for (i = 0; i < 12; i++)
		a[i] = (cube->ep[i] < 8) ? 0 : 1;

	return (uint64_t)subset_to_index(a, 12, 4);
}

uint64_t
index_eposepe(Cube *cube)
{
	int i, j, e[4];
	uint64_t epos, epe;

	epos = (uint64_t)index_epos(cube);
	for (i = 0, j = 0; i < 12; i++)
		if (cube->ep[i] >= 8)
			e[j++] = cube->ep[i] - 8;
	epe = (uint64_t)perm_to_index(e, 4);

	return epos * FACTORIAL4 + epe;
}

/* Inverse indexers **********************************************************/

void
invindex_eofb(uint64_t ind, Cube *cube)
{
	int_to_sum_zero_array(ind, 2, 12, cube->eo);
}

void
invindex_coud(uint64_t ind, Cube *cube)
{
	int_to_sum_zero_array(ind, 3, 8, cube->co);
}

void
invindex_cp(uint64_t ind, Cube *cube)
{
	index_to_perm(ind, 8, cube->cp);
}

void
invindex_cpudsep(uint64_t ind, Cube *cube)
{
	int i, j, k, c[8];

	index_to_subset(ind, 8, 4, c);
	for (i = 0, j = 0, k = 4; i < 8; i++)
		cube->cp[i] = c[i] == 0 ? j++ : k++;
}


void
invindex_epe(uint64_t ind, Cube *cube)
{
	int i;

	index_to_perm(ind, 4, &cube->ep[8]);
	for (i = 0; i < 4; i++)
		cube->ep[i+8] += 8;
}

void
invindex_epud(uint64_t ind, Cube *cube)
{
	index_to_perm(ind, 8, cube->ep);
}

void
invindex_epos(uint64_t ind, Cube *cube)
{
	int i, j, k;

	index_to_subset(ind, 12, 4, cube->ep);
	for (i = 0, j = 0, k = 8; i < 12; i++)
		if (cube->ep[i] == 0)
			cube->ep[i] = j++;
		else
			cube->ep[i] = k++;
}

void
invindex_eposepe(uint64_t ind, Cube *cube)
{
	int i, j, k, e[4];
	uint64_t epos, epe;

	epos = ind / FACTORIAL4;
	epe = ind % FACTORIAL4;

	index_to_subset(epos, 12, 4, cube->ep);
	index_to_perm(epe, 4, e);

	for (i = 0, j = 0, k = 0; i < 12; i++)
		if (cube->ep[i] == 0)
			cube->ep[i] = j++;
		else
			cube->ep[i] = e[k++] + 8;
}

/* Other local functions *****************************************************/

uint64_t
indexers_getmax(Indexer **is)
{
	int i;
	uint64_t max = 1;

	for (i = 0; is[i] != NULL; i++)
		max *= is[i]->n;

	return max;
}

uint64_t
indexers_getind(Indexer **is, Cube *c)
{
	int i;
	uint64_t max = 0;

	for (i = 0; is[i] != NULL; i++) {
		max *= is[i]->n;
		max += is[i]->index(c);
	}

	return max;
}

void
indexers_makecube(Indexer **is, uint64_t ind, Cube *c)
{
	/* Warning: anti-indexers are applied in the same order as indexers. */
	/* We assume order does not matter, but it would make more sense to  */
	/* apply them in reverse.                                            */

	int i;
	uint64_t m;

	make_solved(c);
	m = indexers_getmax(is);
	for (i = 0; is[i] != NULL; i++) {
		m /= is[i]->n;
		is[i]->to_cube(ind / m, c);
		ind %= m;
	}
}

static void
gen_coord_comp(Coordinate *coord)
{
	uint64_t ui;
	Cube c, mvd;
	Move m;
	Trans t;

	coord->max = indexers_getmax(coord->i);

	for (m = 0; m < NMOVES; m++)
		coord->mtable[m] = malloc(coord->max * sizeof(uint64_t));

	for (t = 0; t < NTRANS; t++)
		coord->ttable[t] = malloc(coord->max * sizeof(uint64_t));

	if (!read_coord_mtable(coord)) {
		fprintf(stderr, "%s: generating mtable\n", coord->name);

		for (ui = 0; ui < coord->max; ui++) {
			indexers_makecube(coord->i, ui, &c);
			for (m = 0; m < NMOVES; m++) {
				copy_cube(&c, &mvd);
				apply_move(m, &mvd);
				coord->mtable[m][ui] =
				    indexers_getind(coord->i, &mvd);
			}
		}
		if (!write_coord_mtable(coord))
			fprintf(stderr, "%s: error writing mtable\n",
			    coord->name);
		
		fprintf(stderr, "%s: mtable generated\n", coord->name);
	}

	if (!read_coord_ttable(coord)) {
		fprintf(stderr, "%s: generating ttable\n", coord->name);

		for (ui = 0; ui < coord->max; ui++) {
			indexers_makecube(coord->i, ui, &c);
			for (t = 0; t < NTRANS; t++) {
				copy_cube(&c, &mvd);
				apply_trans(t, &mvd);
				coord->ttable[t][ui] =
				    indexers_getind(coord->i, &mvd);
			}
		}
		if (!write_coord_ttable(coord))
			fprintf(stderr, "%s: error writing ttable\n",
			    coord->name);
	}
}

static void
gen_coord_sym(Coordinate *coord)
{
	uint64_t i, in, ui, uj, uu, M, nr;
	int j;
	Move m;
	Trans t;

	M = coord->base[0]->max;
	coord->selfsim    = malloc(M * sizeof(uint64_t));
	coord->symclass   = malloc(M * sizeof(uint64_t));
	coord->symrep     = malloc(M * sizeof(uint64_t));
	coord->transtorep = malloc(M * sizeof(Trans));

	if (!read_coord_sd(coord)) {
		fprintf(stderr, "%s: generating syms\n", coord->name);

		for (i = 0; i < M; i++)
			coord->symclass[i] = M+1;

		for (i = 0, nr = 0; i < M; i++) {
			if (coord->symclass[i] != M+1)
				continue;

			coord->symrep[nr]    = i;
			coord->transtorep[i] = uf;
			coord->selfsim[nr]   = (uint64_t)0;
			for (j = 0; j < coord->tgrp->n; j++) {
				t = coord->tgrp->t[j];
				in = trans_coord(coord->base[0], t, i);
				coord->symclass[in] = nr;
				if (in == i)
					coord->selfsim[nr] |= ((uint64_t)1<<t);
				else
					coord->transtorep[in] =
					    inverse_trans(t);
			}
			nr++;
		}

		coord->max = nr;

		fprintf(stderr, "%s: found %" PRIu64 " classes\n",
		    coord->name, nr);
		if (!write_coord_sd(coord))
			fprintf(stderr, "%s: error writing symdata\n",
			    coord->name);
	}

	coord->symrep = realloc(coord->symrep, coord->max*sizeof(uint64_t));
	coord->selfsim = realloc(coord->selfsim, coord->max*sizeof(uint64_t));

	for (m = 0; m < NMOVES; m++) {
		coord->mtable[m] = malloc(coord->max*sizeof(uint64_t));
		coord->ttrep_move[m] = malloc(coord->max*sizeof(Trans));
	}

	if (!read_coord_mtable(coord)) {
		for (ui = 0; ui < coord->max; ui++) {
			uu = coord->symrep[ui];
			for (m = 0; m < NMOVES; m++) {
				uj = move_coord(coord->base[0], m, uu, NULL);
				coord->mtable[m][ui] = coord->symclass[uj];
				coord->ttrep_move[m][ui] =
				    coord->transtorep[uj];
			}
		}
		if (!write_coord_mtable(coord))
			fprintf(stderr, "%s: error writing mtable\n",
			    coord->name);
	}
}

static bool
read_coord_mtable(Coordinate *coord)
{
	FILE *f;
	char fname[strlen(tabledir)+256];
	Move m;
	uint64_t M;
	bool r;

	strcpy(fname, tabledir);
	strcat(fname, "/mt_");
	strcat(fname, coord->name);

	if ((f = fopen(fname, "rb")) == NULL)
		return false;

	M = coord->max;
	r = true;
	for (m = 0; m < NMOVES; m++)
		r = r && fread(coord->mtable[m], sizeof(uint64_t), M, f) == M;

	if (coord->type == SYM_COORD)
		for (m = 0; m < NMOVES; m++)
			r = r && fread(coord->ttrep_move[m],
			    sizeof(Trans), M, f) == M;

	fclose(f);
	return r;
}

static bool
read_coord_sd(Coordinate *coord)
{
	FILE *f;
	char fname[strlen(tabledir)+256];
	uint64_t M, N;
	bool r;

	strcpy(fname, tabledir);
	strcat(fname, "/sd_");
	strcat(fname, coord->name);

	if ((f = fopen(fname, "rb")) == NULL)
		return false;

	r = true;
	r = r && fread(&coord->max,       sizeof(uint64_t), 1, f) == 1;
	M = coord->max;
	N = coord->base[0]->max;
	r = r && fread(coord->symrep,     sizeof(uint64_t), M, f) == M;
	r = r && fread(coord->selfsim,    sizeof(uint64_t), M, f) == M;
	r = r && fread(coord->symclass,   sizeof(uint64_t), N, f) == N;
	r = r && fread(coord->transtorep, sizeof(Trans),    N, f) == N;

	fclose(f);
	return r;
}

static bool
read_coord_ttable(Coordinate *coord)
{
	FILE *f;
	char fname[strlen(tabledir)+256];
	Trans t;
	uint64_t M;
	bool r;

	strcpy(fname, tabledir);
	strcat(fname, "/tt_");
	strcat(fname, coord->name);

	if ((f = fopen(fname, "rb")) == NULL)
		return false;

	M = coord->max;
	r = true;
	for (t = 0; t < NTRANS; t++)
		r = r && fread(coord->ttable[t], sizeof(uint64_t), M, f) == M;

	fclose(f);
	return r;
}

static bool
write_coord_mtable(Coordinate *coord)
{
	FILE *f;
	char fname[strlen(tabledir)+256];
	Move m;
	uint64_t M;
	bool r;

	strcpy(fname, tabledir);
	strcat(fname, "/mt_");
	strcat(fname, coord->name);

	if ((f = fopen(fname, "wb")) == NULL)
		return false;

	M = coord->max;
	r = true;
	for (m = 0; m < NMOVES; m++)
		r = r && fwrite(coord->mtable[m], sizeof(uint64_t), M, f) == M;

	if (coord->type == SYM_COORD)
		for (m = 0; m < NMOVES; m++)
			r = r && fwrite(coord->ttrep_move[m],
			    sizeof(Trans), M, f) == M;

	fclose(f);
	return r;
}

static bool
write_coord_sd(Coordinate *coord)
{
	FILE *f;
	char fname[strlen(tabledir)+256];
	uint64_t M, N;
	bool r;

	strcpy(fname, tabledir);
	strcat(fname, "/sd_");
	strcat(fname, coord->name);

	if ((f = fopen(fname, "wb")) == NULL)
		return false;

	r = true;
	M = coord->max;
	N = coord->base[0]->max;
	r = r && fwrite(&coord->max,       sizeof(uint64_t), 1, f) == 1;
	r = r && fwrite(coord->symrep,     sizeof(uint64_t), M, f) == M;
	r = r && fwrite(coord->selfsim,    sizeof(uint64_t), M, f) == M;
	r = r && fwrite(coord->symclass,   sizeof(uint64_t), N, f) == N;
	r = r && fwrite(coord->transtorep, sizeof(Trans),    N, f) == N;

	fclose(f);
	return r;
}

static bool
write_coord_ttable(Coordinate *coord)
{
	FILE *f;
	char fname[strlen(tabledir)+256];
	Trans t;
	uint64_t M;
	bool r;

	strcpy(fname, tabledir);
	strcat(fname, "/tt_");
	strcat(fname, coord->name);

	if ((f = fopen(fname, "wb")) == NULL)
		return false;

	M = coord->max;
	r = true;
	for (t = 0; t < NTRANS; t++)
		r = r && fwrite(coord->ttable[t], sizeof(uint64_t), M, f) == M;

	fclose(f);
	return r;
}

/* Public functions **********************************************************/

void
gen_coord(Coordinate *coord)
{
	int i;

	if (coord == NULL || coord->generated)
		return;

	for (i = 0; i < 2; i++)
		gen_coord(coord->base[i]);

	switch (coord->type) {
	case COMP_COORD:
		if (coord->i[0] == NULL)
			goto error_gc;
		gen_coord_comp(coord);
		break;
	case SYM_COORD:
		if (coord->base[0] == NULL || coord->tgrp == NULL)
			goto error_gc;
		gen_coord_sym(coord);
		break;
	case SYMCOMP_COORD:
		if (coord->base[0] == NULL || coord->base[1] == NULL)
			goto error_gc;
		coord->max = coord->base[0]->max * coord->base[1]->max;
		break;
	default:
		break;
	}

	coord->generated = true;
	return;

error_gc:
	fprintf(stderr, "Error generating coordinates.\n"
			"This is a bug, pleae report.\n");
	exit(1);
}

uint64_t
index_coord(Coordinate *coord, Cube *cube, Trans *offtrans)
{
	uint64_t c[2], cnosym;
	Trans ttr;

	switch (coord->type) {
	case COMP_COORD:
		if (offtrans != NULL)
			*offtrans = uf;

		return indexers_getind(coord->i, cube);
	case SYM_COORD:
		cnosym = index_coord(coord->base[0], cube, NULL);
		ttr = coord->transtorep[cnosym];

		if (offtrans != NULL)
			*offtrans = ttr;

		return coord->symclass[cnosym];
	case SYMCOMP_COORD:
		c[0] = index_coord(coord->base[0], cube, NULL);
		cnosym = index_coord(coord->base[0]->base[0], cube, NULL);
		ttr = coord->base[0]->transtorep[cnosym];
		c[1] = index_coord(coord->base[1], cube, NULL);
		c[1] = trans_coord(coord->base[1], ttr, c[1]);

		if (offtrans != NULL)
			*offtrans = ttr;

		return c[0] * coord->base[1]->max + c[1];
	default:
		break;
	}

	return coord->max; /* Only reached in case of error */
}

uint64_t
move_coord(Coordinate *coord, Move m, uint64_t ind, Trans *offtrans)
{
	uint64_t i[2], M;
	Trans ttr;

	/* Some safety checks should be done here, but for performance   *
	 * reasons we'd rather do them before calling this function.     *
	 * We should check if coord is generated.                        */

	switch (coord->type) {
	case COMP_COORD:
		if (offtrans != NULL)
			*offtrans = uf;

		return coord->mtable[m][ind];
	case SYM_COORD:
		ttr = coord->ttrep_move[m][ind];

		if (offtrans != NULL)
			*offtrans = ttr;

		return coord->mtable[m][ind];
	case SYMCOMP_COORD:
		M = coord->base[1]->max;
		i[0] = ind / M;
		i[1] = ind % M;
		ttr = coord->base[0]->ttrep_move[m][i[0]];
		i[0] = coord->base[0]->mtable[m][i[0]];
		i[1] = coord->base[1]->mtable[m][i[1]];
		i[1] = coord->base[1]->ttable[ttr][i[1]];

		if (offtrans != NULL)
			*offtrans = ttr;

		return i[0] * M + i[1];
	default:
		break;
	}

	return coord->max; /* Only reached in case of error */
}

uint64_t
trans_coord(Coordinate *coord, Trans t, uint64_t ind)
{
	uint64_t i[2], M;

	/* Some safety checks should be done here, but for performance   *
	 * reasons we'd rather do them before calling this function.     *
	 * We should check if coord is generated.                        */

	switch (coord->type) {
	case COMP_COORD:
		return coord->ttable[t][ind];
	case SYM_COORD:
		return ind;
	case SYMCOMP_COORD:
		M = coord->base[1]->max;
		i[0] = ind / M; /* Always fixed */
		i[1] = ind % M;
		i[1] = coord->base[1]->ttable[t][i[1]];
		return i[0] * M + i[1];
	default:
		break;
	}

	return coord->max; /* Only reached in case of error */
}
