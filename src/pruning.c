#define PRUNING_C

#include "pruning.h"

#define ENTRIES_PER_GROUP              (2*sizeof(entry_group_t))
#define ENTRIES_PER_GROUP_COMPACT      (4*sizeof(entry_group_t))

static int         findchunk(PruneData *pd, int nchunks, uint64_t i);
static void        genptable_bfs(PruneData *pd, int d, int nt, int nc);
static void        genptable_fixnasty(PruneData *pd, int d, int nthreads);
static void *      instance_bfs(void *arg);
static void *      instance_fixnasty(void *arg);
static void        ptable_update(PruneData *pd, uint64_t ind, int m);
static bool        read_ptable_file(PruneData *pd);
static bool        write_ptable_file(PruneData *pd);

PruneData *active_pd[256];

int
findchunk(PruneData *pd, int nchunks, uint64_t i)
{
	uint64_t chunksize;

	chunksize = pd->coord->max / (uint64_t)nchunks;
	chunksize += ENTRIES_PER_GROUP - (chunksize % ENTRIES_PER_GROUP);

	return MIN(nchunks-1, (int)(i / chunksize));
}

PruneData *
genptable(PruneData *pd, int nthreads)
{
	int d, nchunks, i, maxv;
	uint64_t oldn;

	for (i = 0; active_pd[i] != NULL; i++) {
		if (active_pd[i]->coord == pd->coord &&
		    active_pd[i]->moveset == pd->moveset &&
		    active_pd[i]->compact == pd->compact)
			return active_pd[i];
	}

	init_moveset(pd->moveset);
	gen_coord(pd->coord);

	pd->ptable = malloc(ptablesize(pd) * sizeof(entry_group_t));

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

	maxv = pd->compact ? MIN(15, pd->base + 4) : 15;
	for (d = 0; d < maxv && pd->n < pd->coord->max; d++) {
		genptable_bfs(pd, d, nthreads, nchunks);
		genptable_fixnasty(pd, d+1, nthreads);
		fprintf(stderr, "Depth %d done, generated %"
			PRIu64 "\t(%" PRIu64 "/%" PRIu64 ")\n",
			d+1, pd->n - oldn, pd->n, pd->coord->max);
		pd->count[d+1] = pd->n - oldn;
		oldn = pd->n;
	}
	if (pd->compact)
		fprintf(stderr, "Compact table, values above "
				"%d are inaccurate.\n", maxv-1);
	fprintf(stderr, "Pruning table generated!\n");

	if (!write_ptable_file(pd))
		fprintf(stderr, "Error writing ptable file\n");

genptable_done:
	for (i = 0; active_pd[i] != NULL; i++);
	return active_pd[i] = pd;
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
	int j, pval, ichunk, oldc, newc;
	Move *ms;

	td = (ThreadDataGenpt *)arg;
	ms = td->pd->moveset->sorted_moves;
	blocksize = td->pd->coord->max / (uint64_t)td->nthreads;
	rmin = ((uint64_t)td->thid) * blocksize;
	rmax = td->thid == td->nthreads - 1 ?
	       td->pd->coord->max :
	       ((uint64_t)td->thid + 1) * blocksize;

	if (td->pd->compact) {
		if (td->d <= td->pd->base) {
			oldc = 1;
			newc = 1;
		} else {
			oldc = td->d - td->pd->base;
			newc = td->d - td->pd->base;
		}
	} else {
		oldc = td->d;
		newc = td->d + 1;
	}

	updated = 0;
	for (i = rmin; i < rmax; i++) {
		ichunk = findchunk(td->pd, td->nchunks, i);
		pthread_mutex_lock(td->mutex[ichunk]);
		pval = ptableval(td->pd, i);
		pthread_mutex_unlock(td->mutex[ichunk]);
		if (pval == oldc) {
			for (j = 0; ms[j] != NULLMOVE; j++) {
				ii = move_coord(td->pd->coord, ms[j], i, NULL);
				ichunk = findchunk(td->pd, td->nchunks, ii);
				pthread_mutex_lock(td->mutex[ichunk]);
				pval = ptableval(td->pd, ii);
				if (pval > newc) {
					ptable_update(td->pd, ii, newc);
					updated++;
				}
				pthread_mutex_unlock(td->mutex[ichunk]);
			}
			if (td->pd->compact && td->d <= td->pd->base) {
				ichunk = findchunk(td->pd, td->nchunks, i);
				pthread_mutex_lock(td->mutex[ichunk]);
				ptable_update(td->pd, i, 0);
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
	int j, oldc;
	Trans t;

	td = (ThreadDataGenpt *)arg;

	/* We know type = SYMCOMP_COORD */
	M = td->pd->coord->base[1]->max;
	blocksize = (td->pd->coord->base[0]->max / td->nthreads) * M;
	rmin = ((uint64_t)td->thid) * blocksize;
	rmax = td->thid == td->nthreads - 1 ?
	       td->pd->coord->max :
	       ((uint64_t)td->thid + 1) * blocksize;

	if (td->pd->compact) {
		if (td->d <= td->pd->base)
			oldc = 1;
		else
			oldc = td->d - td->pd->base;
	} else {
		oldc = td->d;
	}

	updated = 0;
	for (i = rmin; i < rmax; i++) {
		if (ptableval(td->pd, i) == oldc) {
			ss = td->pd->coord->base[0]->selfsim[i/M];
			for (j = 0; j < td->pd->coord->base[0]->tgrp->n; j++) {
				t = td->pd->coord->base[0]->tgrp->t[j];
				if (t == uf || !(ss & ((uint64_t)1<<t)))
					continue;
				ii = trans_coord(td->pd->coord, t, i);
				if (ptableval(td->pd, ii) > oldc) {
					ptable_update(td->pd, ii, oldc);
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

	if (pd->compact) {
		printf("Compract table with base value: %d\n", pd->base);
		printf("Values above %d are inaccurate.\n", pd->base + 3);
	}

	for (i = 0; i < 16; i++)
		printf("%2" PRIu64 "\t%10" PRIu64 "\n", i, pd->count[i]);
}

uint64_t
ptablesize(PruneData *pd)
{
	uint64_t e;

	e = pd->compact ? ENTRIES_PER_GROUP_COMPACT : ENTRIES_PER_GROUP;

	return (pd->coord->max + e - 1) / e;
}

static void
ptable_update(PruneData *pd, uint64_t ind, int n)
{
	int sh;
	entry_group_t f, mask;
	uint64_t i, e, b;

	e = pd->compact ? ENTRIES_PER_GROUP_COMPACT : ENTRIES_PER_GROUP;
	b = pd->compact ? 2 : 4;
	f = pd->compact ? 3 : 15;

	sh = b * (ind % e);
	mask = f << sh;
	i = ind / e;

	pd->ptable[i] &= ~mask;
	pd->ptable[i] |= (((entry_group_t)n) & f) << sh;
}

int
ptableval(PruneData *pd, uint64_t ind)
{
	int sh;
	uint64_t e;
	entry_group_t m;

	if (pd->compact) {
		e  = ENTRIES_PER_GROUP_COMPACT;
		m  = 3;
		sh = (ind % e) * 2;
	} else {
		e  = ENTRIES_PER_GROUP;
		m  = 15;
		sh = (ind % e) * 4;
	}

	return (pd->ptable[ind/e] & (m << sh)) >> sh;
}

static bool
read_ptable_file(PruneData *pd)
{
	init_env();

	FILE *f;
	char fname[strlen(tabledir)+256];
	int i;
	uint64_t r;

	strcpy(fname, tabledir);
	strcat(fname, "/pt_");
	strcat(fname, pd->coord->name);
	strcat(fname, "_");
	strcat(fname, pd->moveset->name);

	if ((f = fopen(fname, "rb")) == NULL)
		return false;

	r = fread(&(pd->base), sizeof(int), 1, f);
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
	int i;
	uint64_t w;

	strcpy(fname, tabledir);
	strcat(fname, "/pt_");
	strcat(fname, pd->coord->name);
	strcat(fname, "_");
	strcat(fname, pd->moveset->name);

	if ((f = fopen(fname, "wb")) == NULL)
		return false;

	w = fwrite(&(pd->base), sizeof(int), 1, f);
	for (i = 0; i < 16; i++)
		w += fwrite(&(pd->count[i]), sizeof(uint64_t), 1, f);
	w += fwrite(pd->ptable, sizeof(entry_group_t), ptablesize(pd), f);
	fclose(f);

	return w == 17 + ptablesize(pd);
}

