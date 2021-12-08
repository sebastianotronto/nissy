#include "pruning.h"

/* Chunks for multithreading */
#define NCHUNKS    100000

static int         findchunk(PruneData *pd, int nchunks, uint64_t i);
static void        genptable_bfs(PruneData *pd,int d,Move *ms,int nt,int nc);
static void        genptable_fixnasty(PruneData *pd, int d);
static void *      instance_bfs(void *arg);
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

int
findchunk(PruneData *pd, int nchunks, uint64_t i)
{
	uint64_t chunksize;

	chunksize = pd->coord->max / (uint64_t)nchunks;
	if (chunksize % 2 != 0)
		chunksize++;

	return MIN(nchunks-1, (int)(i / chunksize));
}

void
genptable(PruneData *pd, int nthreads)
{
	Move *ms;
	int d, nchunks;
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

	nchunks = MIN(pd->coord->max, NCHUNKS);
	fprintf(stderr, "Cannot load %s, generating it "
			"with %d threads and %d chunks\n",
			pd->filename, nthreads, nchunks); 

	ms = malloc(NMOVES * sizeof(Move));
	moveset_to_list(pd->moveset, ms);

	/* We use 4 bits per value, so any distance >= 15 is set to 15 */
	for (j = 0; j < pd->coord->max; j++)
		ptable_update_index(pd, j, 15);

	ptable_update(pd, (Cube){0}, 0);
	pd->n = 1;
	oldn = 0;
	genptable_fixnasty(pd, 0);
	fprintf(stderr, "Depth %d done, generated %"
		PRIu64 "\t(%" PRIu64 "/%" PRIu64 ")\n",
		0, pd->n - oldn, pd->n, pd->coord->max);
	oldn = pd->n;
	for (d = 0; d < 15 && pd->n < pd->coord->max; d++) {
		genptable_bfs(pd, d, ms, nthreads, nchunks);
		genptable_fixnasty(pd, d+1);
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

static void
genptable_bfs(PruneData *pd, int d, Move *ms, int nthreads, int nchunks)
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
		td[i].ms       = ms;
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
genptable_fixnasty(PruneData *pd, int d)
{
	uint64_t i;
	int j, n;
	Cube c, cc;
	Trans t[NTRANS];

	for (i = 0; i < pd->coord->max; i++) {
		if (ptableval_index(pd, i) == d) {
			n = pd->coord->trans(i, t);
			if (n == 1)
				continue;

			c = pd->coord->cube(i);
			for (j = 0; j < n; j++) {
				cc = apply_trans(t[j], c);
				if (ptableval(pd, cc) > d) {
					ptable_update(pd, cc, d);
					pd->n++;
				}
			}
		}
	}
}

static void *
instance_bfs(void *arg)
{
	ThreadDataGenpt *td;
	uint64_t i, ii, blocksize, rmin, rmax, updated;
	int j, pval, ichunk;
	Cube c, cc;

	td = (ThreadDataGenpt *)arg;
	blocksize = td->pd->coord->max / (uint64_t)td->nthreads;
	rmin = ((uint64_t)td->thid) * blocksize;
	rmax = td->thid == td->nthreads - 1 ?
	       td->pd->coord->max :
	       ((uint64_t)td->thid + 1) * blocksize;

	updated = 0;
	for (i = rmin; i < rmax; i++) {
		ichunk = findchunk(td->pd, td->nchunks, i);
		pthread_mutex_lock(td->mutex[ichunk]);
		pval = ptableval_index(td->pd, i);
		pthread_mutex_unlock(td->mutex[ichunk]);
		if (pval == td->d) {
			c = td->pd->coord->cube(i);
			for (j = 0; td->ms[j] != NULLMOVE; j++) {
				cc = apply_move(td->ms[j], c);
				ii = td->pd->coord->index(cc);
				ichunk = findchunk(td->pd, td->nchunks, ii);
				pthread_mutex_lock(td->mutex[ichunk]);
				pval = ptableval_index(td->pd, ii);
				if (pval > td->d+1) {
					ptable_update(td->pd, cc, td->d+1);
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

void
print_ptable(PruneData *pd)
{
	uint64_t i, a[16];
	
	for (i = 0; i < 16; i++)
		a[i] = 0;

	if (!pd->generated)
		genptable(pd, 1); /* TODO: set default nthreads somewhere */

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
	ptable_update_index(pd, pd->coord->index(cube), n);
}

static void
ptable_update_index(PruneData *pd, uint64_t ind, int n)
{
	uint8_t oldval2;
	int other;

	oldval2 = pd->ptable[ind/2];
	other = (ind % 2) ? oldval2 % 16 : oldval2 / 16;

	pd->ptable[ind/2] = (ind % 2) ? 16*n + other : 16*other + n;
	/*pd->n++;*/
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
		genptable(pd, 1); /* TODO: set default or remove this case */
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

