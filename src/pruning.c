#include "pruning.h"

/*
 * The commented functions are a way to generate a pruning table
 * without using anti-indexes. It does not matter too much because
 * we still need anti-indexes in gensym.
 */
/*
static bool        dfs_get_visited(PruneData *pd, DfsData *dd, Cube c);
static bool        dfs_get_visited_index(DfsData *dd, uint64_t ind);
static void        dfs_set_visited(PruneData *pd, DfsData *dd, Cube c, bool b);
static void        dfs_set_visited_index(DfsData *dd, uint64_t ind, bool b);
static void        genptable_dfs(Cube c, PruneData *pd, DfsData *dd);
*/

static void        genptable_bfs(PruneData *pd, int d, Move *ms);
static void        genptable_branch(PruneData *pd,uint64_t ind,int d,Move *ms);
static void        ptable_update(PruneData *pd, Cube cube, int m);
static void        ptable_update_index(PruneData *pd, uint64_t ind, int m);
static int         ptableval_index(PruneData *pd, uint64_t ind);
static bool        read_ptable_file(PruneData *pd);
static bool        write_ptable_file(PruneData *pd);

PruneData
pd_eofb_HTM = {
	.filename = "pt_eofb_HTM",
	.coord    = &coord_eofb,
	.moveset  = moveset_HTM,
};

PruneData
pd_coud_HTM = {
	.filename = "pt_coud_HTM",
	.coord    = &coord_coud,
	.moveset  = moveset_HTM,
};

PruneData
pd_cornershtr_HTM = {
	.filename = "pt_cornershtr_HTM",
	.coord    = &coord_cornershtr,
	.moveset  = moveset_HTM,
};

PruneData
pd_corners_HTM = {
	.filename = "pt_corners_HTM",
	.coord    = &coord_corners,
	.moveset  = moveset_HTM,
};

PruneData
pd_drud_sym16_HTM = {
	.filename = "pt_drud_sym16_HTM",
	.coord    = &coord_drud_sym16,
	.moveset  = moveset_HTM,
};

PruneData
pd_drud_eofb = {
	.filename = "pt_drud_eofb",
	.coord    = &coord_drud_eofb,
	.moveset  = moveset_eofb,
};

PruneData
pd_drudfin_noE_sym16_drud = {
	.filename = "pt_drudfin_noE_sym16_drud",
	.coord    = &coord_drudfin_noE_sym16,
	.moveset  = moveset_drud,
};

PruneData
pd_htr_drud = {
	.filename = "pt_htr_drud",
	.coord    = &coord_htr_drud,
	.moveset  = moveset_drud,
};

PruneData
pd_htrfin_htr = {
	.filename = "pt_htrfin_htr",
	.coord    = &coord_htrfin,
	.moveset  = moveset_htr,
};

PruneData
pd_khuge_HTM = {
	.filename = "pt_khuge_HTM",
	.coord    = &coord_khuge,
	.moveset  = moveset_HTM,
};

/*
void
genptable(PruneData *pd)
{
	Move *ms;
	uint64_t j, oldn;
	DfsData dd;

	if (pd->generated)
		return;

	pd->ptable = malloc(ptablesize(pd) * sizeof(uint8_t));

	if (read_ptable_file(pd)) {
		pd->generated = true;
		return;
	}
	pd->generated = true;

	fprintf(stderr, "Cannot load %s, generating it\n", pd->filename); 

	ms = malloc(NMOVES * sizeof(Move));
	moveset_to_list(pd->moveset, ms);

	for (j = 0; j < pd->coord->max; j++)
		ptable_update_index(pd, j, 15);

	dd = (DfsData) { .m = 0 };
	dd.visited = malloc((ptablesize(pd)/4 + 1) * sizeof(uint8_t));
	dd.sorted_moves = malloc(NMOVES * sizeof(Move));
	moveset_to_list(pd->moveset, dd.sorted_moves);
	oldn = 0;
	pd->n = 0;

	for (dd.d = 0; dd.d < 15 && pd->n < pd->coord->max; dd.d++) {
		for (j = 0; j < pd->coord->max; j++)
			dfs_set_visited_index(&dd, j, false);
		genptable_dfs((Cube){0}, pd, &dd);
		fprintf(stderr, "Depth %d done, generated %"
			PRIu64 "\t(%" PRIu64 "/%" PRIu64 ")\n",
			dd.d+1, pd->n - oldn, pd->n, pd->coord->max);
		oldn = pd->n;
	}

	if (!write_ptable_file(pd))
		fprintf(stderr, "Error writing ptable file\n");

	free(ms);
	free(dd.visited);
	free(dd.sorted_moves);
}
*/

void
genptable(PruneData *pd)
{
	Move *ms;
	int d;
	uint64_t j, oldn;

	if (pd->generated)
		return;

	/* TODO: check if memory is enough, otherwise maybe exit gracefully? */
	pd->ptable = malloc(ptablesize(pd) * sizeof(uint8_t));

	if (read_ptable_file(pd)) {
		pd->generated = true;
		return;
	}
	pd->generated = true;

	fprintf(stderr, "Cannot load %s, generating it\n", pd->filename); 

	ms = malloc(NMOVES * sizeof(Move));
	moveset_to_list(pd->moveset, ms);

	/* We use 4 bits per value, so any distance >= 15 is set to 15 */
	for (j = 0; j < pd->coord->max; j++)
		ptable_update_index(pd, j, 15);

	ptable_update(pd, (Cube){0}, 0);
	pd->n = 1;
	oldn = 0;
	fprintf(stderr, "Depth %d done, generated %"
		PRIu64 "\t(%" PRIu64 "/%" PRIu64 ")\n",
		0, pd->n - oldn, pd->n, pd->coord->max);
	oldn = 1;
	for (d = 0; d < 15 && pd->n < pd->coord->max; d++) {
		genptable_bfs(pd, d, ms);
		fprintf(stderr, "Depth %d done, generated %"
			PRIu64 "\t(%" PRIu64 "/%" PRIu64 ")\n",
			d+1, pd->n - oldn, pd->n, pd->coord->max);
		oldn = pd->n;
	}
	fprintf(stderr, "Pruning table generated!\n");

	if (!write_ptable_file(pd))
		fprintf(stderr, "Error writing ptable file\n");

	free(ms);
}

/*
static void
genptable_dfs(Cube c, PruneData *pd, DfsData *dd)
{
	int i, j, pv;
	Move mm;
	Cube cc;

	pv = ptableval(pd, c);

	if (pv < dd->m || dd->m > dd->d)
		return;

	if (dfs_get_visited(pd, dd, c))
		return;
	dfs_set_visited(pd, dd, c, true);

	if (pv != dd->m)
		ptable_update(pd, c, dd->m);

	for (i = 0; i < pd->coord->ntrans; i++) {
		cc = i == 0 ? c :
			      apply_trans(pd->coord->trans[i], c);
		for (j = 0; dd->sorted_moves[j] != NULLMOVE; j++) {
			mm = dd->sorted_moves[j];
			dd->m++;
			genptable_dfs(apply_move(mm, cc), pd, dd);
			dd->m--;
		}
	}
}
*/

/*
static bool
dfs_get_visited(PruneData *pd, DfsData *dd, Cube c)
{
	return dfs_get_visited_index(dd, pd->coord->index(c));
}
*/

/*
static bool
dfs_get_visited_index(DfsData *dd, uint64_t ind)
{
	return dd->visited[ind/8] & ((uint8_t)1 << (ind % 8));
}
*/

/*
static void
dfs_set_visited(PruneData *pd, DfsData *dd, Cube c, bool b)
{
	dfs_set_visited_index(dd, pd->coord->index(c), b);
}
*/

/*
static void
dfs_set_visited_index(DfsData *dd, uint64_t ind, bool b)
{
	if (b)
		dd->visited[ind/8] |= ((uint8_t)1 << (ind % 8));
	else
		dd->visited[ind/8] &= ~((uint8_t)1 << (ind % 8));
}
*/

static void
genptable_bfs(PruneData *pd, int d, Move *ms)
{
	uint64_t i;

	for (i = 0; i < pd->coord->max; i++)
		if (ptableval_index(pd, i) == d)
			genptable_branch(pd, i, d, ms);
}

static void
genptable_branch(PruneData *pd, uint64_t ind, int d, Move *ms)
{
	int i, j;
	Cube ci, cc, c;

	ci = pd->coord->cube(ind);

	for (i = 0; i < pd->coord->ntrans; i++) {
		c = i == 0 ? ci :
			     apply_trans(pd->coord->trans[i], ci);
		for (j = 0; ms[j] != NULLMOVE; j++) {
			cc = apply_move(ms[j], c);
			if (ptableval(pd, cc) > d+1)
				ptable_update(pd, cc, d+1);
		}
	}
}

void
print_ptable(PruneData *pd)
{
	uint64_t i, a[16];
	
	for (i = 0; i < 16; i++)
		a[i] = 0;

	if (!pd->generated)
		genptable(pd);

	for (i = 0; i < pd->coord->max; i++)
		a[ptableval_index(pd, i)]++;
		
	fprintf(stderr, "Values for table %s\n", pd->filename);
	for (i = 0; i < 16; i++)
		printf("%2" PRIu64 "\t%10" PRIu64 "\n", i, a[i]);
}

uint64_t
ptablesize(PruneData *pd)
{
	return (pd->coord->max + 1) / 2;
}

static void
ptable_update(PruneData *pd, Cube cube, int n)
{
	uint64_t ind = pd->coord->index(cube);
	ptable_update_index(pd, ind, n);
}

static void
ptable_update_index(PruneData *pd, uint64_t ind, int n)
{
	uint8_t oldval2 = pd->ptable[ind/2];
	int other = (ind % 2) ? oldval2 % 16 : oldval2 / 16;

	pd->ptable[ind/2] = (ind % 2) ? 16*n + other : 16*other + n;
	pd->n++;
}

int
ptableval(PruneData *pd, Cube cube)
{
	return ptableval_index(pd, pd->coord->index(cube));
}

static int
ptableval_index(PruneData *pd, uint64_t ind)
{
	if (!pd->generated) {
		fprintf(stderr, "Warning: request pruning table value"
			" for uninitialized table %s.\n It's fine, but it"
			" should not happen. Please report bug.\n",
			pd->filename);
		genptable(pd);
	}

	return (ind % 2) ? pd->ptable[ind/2] / 16 : pd->ptable[ind/2] % 16;
}

static bool
read_ptable_file(PruneData *pd)
{
	init_env();

	FILE *f;
	char fname[strlen(tabledir)+100];
	uint64_t r;

	strcpy(fname, tabledir);
	strcat(fname, "/");
	strcat(fname, pd->filename);

	if ((f = fopen(fname, "rb")) == NULL)
		return false;

	r = fread(pd->ptable, sizeof(uint8_t), ptablesize(pd), f);
	fclose(f);

	return r == ptablesize(pd);
}

static bool
write_ptable_file(PruneData *pd)
{
	init_env();

	FILE *f;
	char fname[strlen(tabledir)+100];
	uint64_t written;

	strcpy(fname, tabledir);
	strcat(fname, "/");
	strcat(fname, pd->filename);

	if ((f = fopen(fname, "wb")) == NULL)
		return false;

	written = fwrite(pd->ptable, sizeof(uint8_t), ptablesize(pd), f);
	fclose(f);

	return written == ptablesize(pd);
}

