#define PRUNING_C

#include "pruning.h"

#define ENTRIES_PER_GROUP              (2*sizeof(entry_group_t))

static int         findchunk(PruneData *pd, int nchunks, uint64_t i);
static void        genptable_bfs(PruneData *pd, int d, int nt, int nc);
static void        genptable_fixnasty(PruneData *pd, int d, int nthreads);
static void *      instance_bfs(void *arg);
static void *      instance_fixnasty(void *arg);
static void        ptable_update(PruneData *pd, uint64_t ind, int m);
static bool        read_ptable_file(PruneData *pd);
static bool        write_ptable_file(PruneData *pd);

PDGenData *active_pdg[256];

int
findchunk(PruneData *pd, int nchunks, uint64_t i)
{
	uint64_t chunksize;

	chunksize = pd->coord->max / (uint64_t)nchunks;
	chunksize += ENTRIES_PER_GROUP - (chunksize % ENTRIES_PER_GROUP);

	return MIN(nchunks-1, (int)(i / chunksize));
}

PruneData *
genptable(PDGenData *pdg, int nthreads)
{
	int d, nchunks, i;
	uint64_t oldn;
	PruneData *pd;

	for (i = 0; active_pdg[i] != NULL; i++) {
		pd = active_pdg[i]->pd;
		if (pd->coord == pdg->coord && pd->moveset == pdg->moveset)
			return pd;
	}

	pd = malloc(sizeof(PruneData));
	pdg->pd = pd;
	pd->coord   = pdg->coord;
	pd->moveset = pdg->moveset;
	pd->ptable = malloc(ptablesize(pd) * sizeof(entry_group_t));

	gen_coord(pd->coord);

	if (read_ptable_file(pd))
		goto genptable_done;

	if (nthreads < 4) {
		fprintf(stderr,
			"--- Warning ---\n"
			"You are using only %d threads to generate the pruning"
			"tables. This can take a while.\n"
			"Unless you did this intentionally, you should re-run"
			"this command with `-t 4' or more.\n"
			"---------------\n\n", nthreads
		);
	}

	nchunks = MIN(ptablesize(pd), 100000);
	fprintf(stderr, "Generating pt_%s_%s with %d threads\n",
			pd->coord->name, pd->moveset->name, nthreads); 

	memset(pd->ptable, ~(uint8_t)0, ptablesize(pd)*sizeof(entry_group_t));
	for (i = 0; i < 16; i++)
		pd->count[i] = 0;

	ptable_update(pd, 0, 0);
	pd->n = 1;
	oldn = 0;
	genptable_fixnasty(pd, 0, nthreads);
	fprintf(stderr, "Depth %d done, generated %"
		PRIu64 "\t(%" PRIu64 "/%" PRIu64 ")\n",
		0, pd->n - oldn, pd->n, pd->coord->max);
	oldn = pd->n;
	pd->count[0] = pd->n;
	for (d = 0; d < 15 && pd->n < pd->coord->max; d++) {
		genptable_bfs(pd, d, nthreads, nchunks);
		genptable_fixnasty(pd, d+1, nthreads);
		fprintf(stderr, "Depth %d done, generated %"
			PRIu64 "\t(%" PRIu64 "/%" PRIu64 ")\n",
			d+1, pd->n - oldn, pd->n, pd->coord->max);
		pd->count[d+1] = pd->n - oldn;
		oldn = pd->n;
	}
	fprintf(stderr, "Pruning table generated!\n");

	if (!write_ptable_file(pd))
		fprintf(stderr, "Error writing ptable file\n");

genptable_done:
	for (i = 0; active_pdg[i] != NULL; i++);
	active_pdg[i] = malloc(sizeof(PDGenData));
	active_pdg[i]->coord   = pdg->coord;
	active_pdg[i]->moveset = pdg->moveset;
	active_pdg[i]->pd      = pd;

	return pd;
}

static void
genptable_bfs(PruneData *pd, int d, int nthreads, int nchunks)
{
	int i;
	pthread_t t[nthreads];
	ThreadDataGenpt td[nthreads];
	pthread_mutex_t *mtx[nchunks], *upmtx;

	upmtx = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(upmtx, NULL);
	for (i = 0; i < nchunks; i++) {
		mtx[i] = malloc(sizeof(pthread_mutex_t));
		pthread_mutex_init(mtx[i], NULL);
	}

	for (i = 0; i < nthreads; i++) {
		td[i].thid     = i;
		td[i].nthreads = nthreads;
		td[i].pd       = pd;
		td[i].d        = d;
		td[i].nchunks  = nchunks;
		td[i].mutex    = mtx;
		td[i].upmutex  = upmtx;
		pthread_create(&t[i], NULL, instance_bfs, &td[i]);
	}

	for (i = 0; i < nthreads; i++)
		pthread_join(t[i], NULL);

	free(upmtx);
	for (i = 0; i < nchunks; i++)
		free(mtx[i]);
}

static void
genptable_fixnasty(PruneData *pd, int d, int nthreads)
{
	int i;
	pthread_t t[nthreads];
	ThreadDataGenpt td[nthreads];
	pthread_mutex_t *upmtx;

	if (pd->coord->type != SYMCOMP_COORD)
		return;

	upmtx = malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(upmtx, NULL);
	for (i = 0; i < nthreads; i++) {
		td[i].thid     = i;
		td[i].nthreads = nthreads;
		td[i].pd       = pd;
		td[i].d        = d;
		td[i].upmutex  = upmtx;
		pthread_create(&t[i], NULL, instance_fixnasty, &td[i]);
	}

	for (i = 0; i < nthreads; i++)
		pthread_join(t[i], NULL);

	free(upmtx);
}

static void *
instance_bfs(void *arg)
{
	ThreadDataGenpt *td;
	uint64_t i, ii, blocksize, rmin, rmax, updated;
	int j, pval, ichunk;
	Move *ms;

	td = (ThreadDataGenpt *)arg;
	ms = td->pd->moveset->sorted_moves;
	blocksize = td->pd->coord->max / (uint64_t)td->nthreads;
	rmin = ((uint64_t)td->thid) * blocksize;
	rmax = td->thid == td->nthreads - 1 ?
	       td->pd->coord->max :
	       ((uint64_t)td->thid + 1) * blocksize;

	updated = 0;
	for (i = rmin; i < rmax; i++) {
		ichunk = findchunk(td->pd, td->nchunks, i);
		pthread_mutex_lock(td->mutex[ichunk]);
		pval = ptableval(td->pd, i);
		pthread_mutex_unlock(td->mutex[ichunk]);
		if (pval == td->d) {
			for (j = 0; ms[j] != NULLMOVE; j++) {
				/* ii = td->pd->coord->move(ms[j], i); */
				ii = move_coord(td->pd->coord, ms[j], i, NULL);
				ichunk = findchunk(td->pd, td->nchunks, ii);
				pthread_mutex_lock(td->mutex[ichunk]);
				pval = ptableval(td->pd, ii);
				if (pval > td->d+1) {
					ptable_update(td->pd, ii, td->d+1);
					updated++;
				}
				pthread_mutex_unlock(td->mutex[ichunk]);
			}
		}
	}

	pthread_mutex_lock(td->upmutex);
	td->pd->n += updated;
	pthread_mutex_unlock(td->upmutex);

	return NULL;
}

static void *
instance_fixnasty(void *arg)
{
	ThreadDataGenpt *td;
	uint64_t i, ii, blocksize, rmin, rmax, updated, ss, M;
	int j;
	Trans t;

	td = (ThreadDataGenpt *)arg;

	/* We know type = SYMCOMP_COORD */
	M = td->pd->coord->base[1]->max;
	blocksize = (td->pd->coord->base[0]->max / td->nthreads) * M;
	rmin = ((uint64_t)td->thid) * blocksize;
	rmax = td->thid == td->nthreads - 1 ?
	       td->pd->coord->max :
	       ((uint64_t)td->thid + 1) * blocksize;

	updated = 0;
	for (i = rmin; i < rmax; i++) {
		if (ptableval(td->pd, i) == td->d) {
			ss = td->pd->coord->base[0]->selfsim[i/M];
			for (j = 0; j < td->pd->coord->base[0]->tgrp->n; j++) {
				t = td->pd->coord->base[0]->tgrp->t[j];
				if (t == uf || !(ss & ((uint64_t)1<<t)))
					continue;
				ii = trans_coord(td->pd->coord, t, i);
				if (ptableval(td->pd, ii) > td->d) {
					ptable_update(td->pd, ii, td->d);
					updated++;
				}
			}
		}
	}

	pthread_mutex_lock(td->upmutex);
	td->pd->n += updated;
	pthread_mutex_unlock(td->upmutex);

	return NULL;
}

void
print_ptable(PruneData *pd)
{
	uint64_t i;

	printf("Table %s_%s\n", pd->coord->name, pd->moveset->name);
	for (i = 0; i < 16; i++)
		printf("%2" PRIu64 "\t%10" PRIu64 "\n", i, pd->count[i]);
}

uint64_t
ptablesize(PruneData *pd)
{
	return (pd->coord->max + ENTRIES_PER_GROUP - 1) / ENTRIES_PER_GROUP;
}

static void
ptable_update(PruneData *pd, uint64_t ind, int n)
{
	int sh;
	entry_group_t mask;
	uint64_t i;

	sh = 4 * (ind % ENTRIES_PER_GROUP);
	mask = ((entry_group_t)15) << sh;
	i = ind/ENTRIES_PER_GROUP;

	pd->ptable[i] &= ~mask;
	pd->ptable[i] |= (((entry_group_t)n)&15) << sh;
}

int
ptableval(PruneData *pd, uint64_t ind)
{
	int sh;

	sh = (ind % ENTRIES_PER_GROUP) * 4;

	return (pd->ptable[ind/ENTRIES_PER_GROUP] & (15 << sh)) >> sh;
}

static bool
read_ptable_file(PruneData *pd)
{
	init_env();

	FILE *f;
	char fname[strlen(tabledir)+256];
	int i, phony;
	uint64_t r;

	strcpy(fname, tabledir);
	strcat(fname, "/pt_");
	strcat(fname, pd->coord->name);
	strcat(fname, "_");
	strcat(fname, pd->moveset->name);

	if ((f = fopen(fname, "rb")) == NULL)
		return false;

	r = fread(&phony, sizeof(int), 1, f);
	for (i = 0; i < 16; i++)
		r += fread(&(pd->count[i]), sizeof(uint64_t), 1, f);
	r += fread(pd->ptable, sizeof(entry_group_t), ptablesize(pd), f);

	fclose(f);

	return r == 17 + ptablesize(pd);
}

static bool
write_ptable_file(PruneData *pd)
{
	init_env();

	FILE *f;
	char fname[strlen(tabledir)+256];
	int i, phony = 0;
	uint64_t w;

	strcpy(fname, tabledir);
	strcat(fname, "/pt_");
	strcat(fname, pd->coord->name);
	strcat(fname, "_");
	strcat(fname, pd->moveset->name);

	if ((f = fopen(fname, "wb")) == NULL)
		return false;

	w = fwrite(&phony, sizeof(int), 1, f); /* phony replace base */
	for (i = 0; i < 16; i++)
		w += fwrite(&(pd->count[i]), sizeof(uint64_t), 1, f);
	w += fwrite(pd->ptable, sizeof(entry_group_t), ptablesize(pd), f);
	fclose(f);

	return w == 17 + ptablesize(pd);
}

