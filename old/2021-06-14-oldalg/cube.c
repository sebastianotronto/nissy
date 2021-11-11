#include "cube.h"

/* Constants and macros *****************************************************/

#define POW2TO11            2048ULL
#define POW2TO12            4096ULL
#define POW3TO7             2187ULL
#define POW3TO8             6561ULL
#define FACTORIAL4          24ULL
#define FACTORIAL6          720ULL
#define FACTORIAL8          40320ULL
#define FACTORIAL12         479001600ULL
#define BINOM12ON4          495ULL
#define BINOM8ON4           70ULL
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))

#define BASEALGLEN          50
#define NMOVES              (z3+1)
#define NTRANS              (mirror+1)
#define NROTATIONS          (NTRANS-1)
#define PTABLESIZE(n)       ((n+1) / 2)
#define PTABLEVAL(tab,ind)  (((ind)%2) ? (tab[(ind)/2] / 16) : \
                                         (tab[(ind)/2] % 16))


/* Local functions **********************************************************/

static Cube        admissible_ep(Cube cube, PieceFilter f);
static void        append_alg(AlgList *l, Alg *alg);
static void        append_move(Alg *alg, Move m, bool inverse);
static Cube        apply_alg_generic(Alg *alg, Cube c, PieceFilter f, bool a);
static void        apply_permutation(int *perm, int *set, int n);
static Cube        apply_move_cubearray(Move m, Cube cube, PieceFilter f);
static uint16_t    array_ep_to_epos(int *ep, int *eps_solved);
static Cube        arrays_to_cube(CubeArray *arr, PieceFilter f);
static Move        base_move(Move m);
static int         binomial(int n, int k);
static Cube        compose_filtered(Cube c2, Cube c1, PieceFilter f);
static void        cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f);
static int         digit_array_to_int(int *a, int n, int b);
static int         edge_slice(Edge e); /* E=0, S=1, M=2 */
static int         epos_dependent(int pos1, int pos2);
static uint16_t    epos_from_arrays(int *epos, int *ep);
static void        epos_to_partial_ep(uint16_t epos, int *ep, int *ss);
static int         factorial(int n);
static void        free_alglistnode(AlgListNode *aln);
static void        free_cubearray(CubeArray *arr, PieceFilter f);
static void        generate_ptable(PruneData *pd);
static void        generate_ptable_dfs(Cube c, PruneData *pd, DfsData *dd);
static void        index_to_perm(int p, int n, int *r);
static void        index_to_subset(int s, int n, int k, int *r);
static void        int_to_digit_array(int a, int b, int n, int *r);
static void        int_to_sum_zero_array(int x, int b, int n, int *a);
static int         invert_digits(int a, int b, int n);
static bool        is_perm(int *a, int n);
static bool        is_subset(int *a, int n, int k);
static Cube        move_via_arrays(CubeArray *arr, Cube c, PieceFilter pf);
static void        moveset_to_list(bool (*ms)(Move), int (*f)(Cube), Move *r);
static AlgList *   new_alglist();
static CubeArray * new_cubearray(Cube cube, PieceFilter f);
static int         perm_sign(int *a, int n);
static int         perm_to_index(int *a, int n);
static int         powint(int a, int b);
static void        ptable_set_reached(PruneData *pd, uint64_t ind);
static void        ptable_update(PruneData *pd, uint64_t ind, int m);
static bool        read_ptable_file(PruneData *pd);
static bool        read_ttables_file();
static bool        read_mtables_file();
static Cube        rotate_via_compose(Trans r, Cube c, PieceFilter f);
static void        solve_dfs(Cube c, Step s, SolveOptions *opts, DfsData *dd);
static int         subset_to_index(int *a, int n, int k);
static void        sum_arrays_mod(int *src, int *dst, int n, int m);
static void        swap(int *a, int *b);
static bool        write_ptable_file(PruneData *pd);
static bool        write_ttables_file();
static bool        write_mtables_file();

static void        init_auxtables();
static void        init_environment();
static void        init_moves();
static void        init_moves_aux();
static void        init_steps();
static void        init_strings();
static void        init_trans();
static void        init_trans_aux();

static bool        moveset_HTM(Move m);
static bool        moveset_URF(Move m);

/* Steps and related functions and data **************************************/

Step                    eofb_HTM;
Step                    eorl_HTM;
Step                    eoud_HTM;
Step                    coud_HTM;
Step                    corl_HTM;
Step                    cofb_HTM;
Step                    coud_URF;
Step                    corl_URF;
Step                    cofb_URF;
Step                    corners_HTM;
Step                    corners_URF;
Step                    edges_HTM;
Step                    drud_HTM;
Step                    optimal_HTM;

PruneData               pd_eofb_HTM;
PruneData               pd_coud_HTM;
PruneData               pd_corners_HTM;
PruneData               pd_ep_HTM;
PruneData               pd_drud_HTM;

static uint64_t         index_eofb(Cube cube);
static uint64_t         index_coud(Cube cube);
static uint64_t         index_corners(Cube cube);
static uint64_t         index_ep(Cube cube);
static uint64_t         index_drud(Cube cube);

static int              check_nothing(Cube cube);
static int              check_eofb_HTM(Cube cube);
static int              check_coud_HTM(Cube cube);
static int              check_coud_URF(Cube cube);
static int              check_corners_HTM(Cube cube);
static int              check_corners_URF(Cube cube);
static int              check_edges_HTM(Cube cube);
static int              check_drud_HTM(Cube cube);
static int              check_optimal_HTM(Cube cube);

/* All sorts of useful costants and tables  **********************************/

static char *           tabledir;

static PieceFilter      pf_all;
static PieceFilter      pf_4val;
static PieceFilter      pf_epcp;
static PieceFilter      pf_cpos;
static PieceFilter      pf_cp;
static PieceFilter      pf_ep;
static PieceFilter      pf_e;
static PieceFilter      pf_s;
static PieceFilter      pf_m;
static PieceFilter      pf_eo;
static PieceFilter      pf_co;

static int              epe_solved[4];
static int              eps_solved[4];
static int              epm_solved[4];

static char             move_string[NMOVES][7];
static char             edge_string[12][7];
static char             corner_string[8][7];
static char             center_string[6][7];

static bool             commute[NMOVES][NMOVES];
static bool             possible_next[NMOVES][NMOVES][NMOVES];
static Move             inverse_move_aux[NMOVES];
static Trans            inverse_trans_aux[NTRANS];
static int              epos_dependent_aux[BINOM12ON4][BINOM12ON4];

static uint16_t         epose_ttable[NTRANS][FACTORIAL12/FACTORIAL8];
static uint16_t         eposs_ttable[NTRANS][FACTORIAL12/FACTORIAL8];
static uint16_t         eposm_ttable[NTRANS][FACTORIAL12/FACTORIAL8];
static uint16_t         eo_ttable[NTRANS][POW2TO11];
static uint16_t         cp_ttable[NTRANS][FACTORIAL8];
static uint16_t         co_ttable[NTRANS][POW3TO7];
static uint16_t         cpos_ttable[NTRANS][FACTORIAL6];
static Move             moves_ttable[NTRANS][NMOVES];

static uint16_t         epose_mtable[NMOVES][FACTORIAL12/FACTORIAL8];
static uint16_t         eposs_mtable[NMOVES][FACTORIAL12/FACTORIAL8];
static uint16_t         eposm_mtable[NMOVES][FACTORIAL12/FACTORIAL8];
static uint16_t         eofb_mtable[NMOVES][POW2TO11];
static uint16_t         eorl_mtable[NMOVES][POW2TO11];
static uint16_t         eoud_mtable[NMOVES][POW2TO11];
static uint16_t         cp_mtable[NMOVES][FACTORIAL8];
static uint16_t         coud_mtable[NMOVES][POW3TO7];
static uint16_t         cofb_mtable[NMOVES][POW3TO7];
static uint16_t         corl_mtable[NMOVES][POW3TO7];
static uint16_t         cpos_mtable[NMOVES][FACTORIAL6];

static uint64_t         me[12];

static int              edge_cycle[NMOVES][12];
static int              corner_cycle[NMOVES][8];
static int              center_cycle[NMOVES][6];
static int              eofb_flipped[NMOVES][12];
static int              eorl_flipped[NMOVES][12];
static int              eoud_flipped[NMOVES][12];
static int              coud_flipped[NMOVES][8];
static int              corl_flipped[NMOVES][8];
static int              cofb_flipped[NMOVES][8];
static Alg *            equiv_alg[NMOVES];

static int              epose_source[NTRANS];  /* 0=epose, 1=eposs, 2=eposm */
static int              eposs_source[NTRANS];
static int              eposm_source[NTRANS];
static int              eofb_source[NTRANS];   /* 0=eoud,  1=eorl,  2=eofb  */
static int              eorl_source[NTRANS];
static int              eoud_source[NTRANS];
static int              coud_source[NTRANS];   /* 0=coud,  1=corl,  2=cofb  */
static int              cofb_source[NTRANS];
static int              corl_source[NTRANS];
static int              ep_mirror[12];
static int              cp_mirror[8];
static int              cpos_mirror[6];
static Alg *            trans_algs[NROTATIONS];


/* Local functions implementation ********************************************/

static Cube
admissible_ep(Cube cube, PieceFilter f)
{
	CubeArray *arr = new_cubearray(cube, f);
	Cube ret;
	bool used[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i, j;

	for (i = 0; i < 12; i++)
		if (arr->ep[i] != -1)
			used[arr->ep[i]] = true;

	for (i = 0, j = 0; i < 12; i++) {
		for ( ; j < 11 && used[j]; j++);
		if (arr->ep[i] == -1)
			arr->ep[i] = j++;
	}

	ret = arrays_to_cube(arr, pf_ep);
	free_cubearray(arr, f);

	return ret;
}

static void
append_alg(AlgList *l, Alg *alg)
{
	AlgListNode *node = malloc(sizeof(AlgListNode));
	AlgNode *i;

	node->alg = new_alg("");
	for (i = alg->first; i != NULL; i = i->next)
		append_move(node->alg, i->m, i->inverse);
	node->next = NULL;

	if (++l->len == 1)
		l->first = node;
	else
		l->last->next = node;
	l->last = node;
}

static void
append_move(Alg *alg, Move m, bool inverse)
{
	AlgNode *node = malloc(sizeof(AlgNode));

	node->m = m;
	node->inverse = inverse;
	node->next = NULL;
	node->prev = alg->last;

	if (++alg->len == 1)
		alg->first = node;
	else
		alg->last->next = node;
	alg->last = node;
}

static Cube
apply_alg_generic(Alg *alg, Cube c, PieceFilter f, bool a)
{
	Cube ret = {0};
	AlgNode *i;

	for (i = alg->first; i != NULL; i = i->next)
		if (i->inverse)
			ret = a ? apply_move(i->m, ret) :
			          apply_move_cubearray(i->m, ret, f);

	ret = compose_filtered(c, inverse_cube(ret), f);

	for (i = alg->first; i != NULL; i = i->next)
		if (!i->inverse)
			ret = a ? apply_move(i->m, ret) :
			          apply_move_cubearray(i->m, ret, f);

	return ret;
}

static void
apply_permutation(int *perm, int *set, int n)
{
	int *aux = malloc(n * sizeof(int));
	int i;

	if (!is_perm(perm, n))
		return;
		
	for (i = 0; i < n; i++)
		aux[i] = set[perm[i]];

	memcpy(set, aux, n * sizeof(int));
	free(aux);
}

static Cube
apply_move_cubearray(Move m, Cube cube, PieceFilter f)
{
	CubeArray m_arr = {
		edge_cycle[m],
		eofb_flipped[m],
		eorl_flipped[m],
		eoud_flipped[m],
		corner_cycle[m],
		coud_flipped[m],
		corl_flipped[m],
		cofb_flipped[m],
		center_cycle[m]
	};

	return move_via_arrays(&m_arr, cube, f);
}

static uint16_t
array_ep_to_epos(int *ep, int *ss)
{
	int epos[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	int eps[4];
	int i, j, is;

	for (i = 0, is = 0; i < 12; i++) {
		for (j = 0; j < 4; j++) {
			if (ep[i] == ss[j]) {
				eps[is++] = j;
				epos[i] = 1;
			}
		}
	}

	for (i = 0; i < 4; i++)
		swap(&epos[ss[i]], &epos[i+8]);

	return epos_from_arrays(epos, eps);
}

static Cube
arrays_to_cube(CubeArray *arr, PieceFilter f)
{
	Cube ret = {0};

	if (f.epose)
		ret.epose = array_ep_to_epos(arr->ep, epe_solved);
	if (f.eposs)
		ret.eposs = array_ep_to_epos(arr->ep, eps_solved);
	if (f.eposm)
		ret.eposm = array_ep_to_epos(arr->ep, epm_solved);
	if (f.eofb)
		ret.eofb = digit_array_to_int(arr->eofb, 11, 2);
	if (f.eorl)
		ret.eorl = digit_array_to_int(arr->eorl, 11, 2);
	if (f.eoud)
		ret.eoud = digit_array_to_int(arr->eoud, 11, 2);
	if (f.cp)
		ret.cp = perm_to_index(arr->cp, 8);
	if (f.coud)
		ret.coud = digit_array_to_int(arr->coud, 7, 3);
	if (f.corl)
		ret.corl = digit_array_to_int(arr->corl, 7, 3);
	if (f.cofb)
		ret.cofb = digit_array_to_int(arr->cofb, 7, 3);
	if (f.cpos)
		ret.cpos = perm_to_index(arr->cpos, 6);

	return ret;
}

static Move
base_move(Move m)
{
	if (m == NULLMOVE)
		return NULLMOVE;
	else
		return m - (m-1)%3;
}

static int
binomial(int n, int k)
{
	if (n < 0 || k < 0 || k > n)
		return 0;

	return factorial(n) / (factorial(k) * factorial(n-k));
}

static Cube
compose_filtered(Cube c2, Cube c1, PieceFilter f)
{
	CubeArray *arr = new_cubearray(c2, f);
	Cube ret;

	ret = move_via_arrays(arr, c1, f);
	free_cubearray(arr, f);

	return ret;
}

static void
cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f)
{
	int i;

	if (f.epose || f.eposs || f.eposm)
		for (i = 0; i < 12; i++)
			arr->ep[i] = -1;

	if (f.epose) 
		epos_to_partial_ep(cube.epose, arr->ep, epe_solved); 
	if (f.eposs) 
		epos_to_partial_ep(cube.eposs, arr->ep, eps_solved); 
	if (f.eposm)
		epos_to_partial_ep(cube.eposm, arr->ep, epm_solved); 
	if (f.eofb)
		int_to_sum_zero_array(cube.eofb, 2, 12, arr->eofb);
	if (f.eorl)
		int_to_sum_zero_array(cube.eorl, 2, 12, arr->eorl);
	if (f.eoud)
		int_to_sum_zero_array(cube.eoud, 2, 12, arr->eoud);
	if (f.cp)
		index_to_perm(cube.cp, 8, arr->cp);
	if (f.coud)
		int_to_sum_zero_array(cube.coud, 3, 8, arr->coud);
	if (f.corl)
		int_to_sum_zero_array(cube.corl, 3, 8, arr->corl);
	if (f.cofb)
		int_to_sum_zero_array(cube.cofb, 3, 8, arr->cofb);
	if (f.cpos)
		index_to_perm(cube.cpos, 6, arr->cpos);
}

static int
digit_array_to_int(int *a, int n, int b)
{
    int i, ret = 0, p = 1;

    for (i = 0; i < n; i++, p *= b)
        ret += a[i] * p;

    return ret;
}

static int
edge_slice(Edge e) {
	if (e < 0 || e > 11)
		return -1;

	if (e == FR || e == FL || e == BL || e == BR)
		return 0;
	if (e == UR || e == UL || e == DR || e == DL)
		return 1;

	return 2;
}

static int
epos_dependent(int poss, int pose)
{
	int ep[12] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
	int ep8[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int i, j;

	epos_to_partial_ep(poss*FACTORIAL4, ep, eps_solved);
	epos_to_partial_ep(pose*FACTORIAL4, ep, epe_solved);

	for (i = 0, j = 0; i < 12; i++)
		if (edge_slice(ep[i]) != 1)
			ep8[j++] = (edge_slice(ep[i]) == 0) ? 1 : 0;

	return subset_to_index(ep8, 8, 4);
}

static uint16_t
epos_from_arrays(int *epos, int *ep)
{
	return FACTORIAL4 * subset_to_index(epos,12,4) + perm_to_index(ep,4);
}

static void
epos_to_partial_ep(uint16_t epos, int *ep, int *ss)
{
	int i, is, eposs[12], eps[4];

	index_to_perm(epos % FACTORIAL4, 4, eps);
	index_to_subset(epos / FACTORIAL4, 12, 4, eposs);

	for (i = 0; i < 4; i++)
		swap(&eposs[ss[i]], &eposs[i+8]);

	for (i = 0, is = 0; i < 12; i++)
		if (eposs[i])
			ep[i] = ss[eps[is++]];
}

static int
factorial(int n)
{
	int i, ret = 1;

	if (n < 0)
		return 0;

	for (i = 1; i <= n; i++)
		ret *= i;

	return ret;
}

void
free_alg(Alg *alg)
{
	AlgNode *aux, *i = alg->first;

	while (i != NULL) {
		aux = i->next;
		free(i);
		i = aux;
	}
	free(alg);
}

void
free_alglist(AlgList *l)
{
	AlgListNode *aux, *i = l->first;

	while (i != NULL) {
		aux = i->next;
		free_alglistnode(i);
		i = aux;
	}
	free(l);
}

void
free_alglistnode(AlgListNode *aln)
{
	free_alg(aln->alg);
	free(aln);
}

static void
free_cubearray(CubeArray *arr, PieceFilter f)
{
	if (f.epose || f.eposs || f.eposm)
		free(arr->ep);
	if (f.eofb)
		free(arr->eofb);
	if (f.eorl)
		free(arr->eorl);
	if (f.eoud)
		free(arr->eoud);
	if (f.cp)
		free(arr->cp);
	if (f.coud)
		free(arr->coud);
	if (f.corl)
		free(arr->corl);
	if (f.cofb)
		free(arr->cofb);
	if (f.cpos)
		free(arr->cpos);

	free(arr);
}

static void
generate_ptable(PruneData *pd)
{
	uint64_t j;
	DfsData dd;

	if (pd->generated)
		return;

	pd->ptable = malloc(PTABLESIZE(pd->size) * sizeof(uint8_t));

	if (read_ptable_file(pd)) {
		pd->generated = true;
		return;
	}

	fprintf(stderr, "Cannot load %s, generating it\n", pd->filename); 

	dd.m     = 0;
	dd.last1 = NULLMOVE;
	dd.last2 = NULLMOVE;

	/* We use 4 bits per value, so any distance >= 15 is set to 15 */
	for (j = 0; j < pd->size; j++)
		ptable_update(pd, j, 15);

	moveset_to_list(pd->moveset, NULL, dd.sorted_moves);
	
	pd->reached = malloc(PTABLESIZE(pd->size) * sizeof(uint8_t));
	for (dd.d = 0, pd->n = 0; dd.d < 15 && pd->n < pd->size; dd.d++) {
		memset(pd->reached, 0, PTABLESIZE(pd->size)*sizeof(uint8_t));
		generate_ptable_dfs((Cube){0}, pd, &dd);
		fprintf(stderr, "Depth %d completed, generated %lu/%lu\n",
			dd.d, pd->n, pd->size);
	}

	if (!write_ptable_file(pd))
		fprintf(stderr, "Error writing ptable file\n");

	pd->generated = true;
	free(pd->reached);
}

static void
generate_ptable_dfs(Cube c, PruneData *pd, DfsData *dd)
{
	uint64_t ind = pd->index(c);
	int oldval = PTABLEVAL(pd->ptable, ind);
	Move i, move, l1 = dd->last1, l2 = dd->last2;

	if (oldval < dd->m || PTABLEVAL(pd->reached, ind) || pd->n == pd->size)
		return;

	ptable_set_reached(pd, ind);

	if (dd->m == dd->d) {
		if (dd->m < oldval)
			ptable_update(pd, ind, dd->m);
		return;
	}

	dd->m++;
	dd->last2 = dd->last1;

	for (i = 0; dd->sorted_moves[i] != NULLMOVE; i++) {
		move = dd->sorted_moves[i];
		if (possible_next[l2][l1][move]) {
			dd->last1 = move;
			generate_ptable_dfs(apply_move(move, c), pd, dd);
		}
	}

	dd->m--;
	dd->last1 = l1;
	dd->last2 = l2;
}

static void
index_to_perm(int p, int n, int *r)
{
	int *a = malloc(n * sizeof(int));
	int i, j, c;

	for (i = 0; i < n; i++)
		a[i] = 0;

	if (p < 0 || p >= factorial(n))
		for (i = 0; i < n; i++)
			r[i] = -1;

	for (i = 0; i < n; i++) {
		c = 0;
		j = 0;
		while (c <= p / factorial(n-i-1))
			c += a[j++] ? 0 : 1;
		r[i] = j-1;
		a[j-1] = 1;
		p %= factorial(n-i-1);
	}

	free(a);
}

static void
index_to_subset(int s, int n, int k, int *r)
{
	int i, j, v;

	if (s < 0 || s >= binomial(n, k)) {
		for (i = 0; i < n; i++)
			r[i] = -1;
		return;
	}

	for (i = 0; i < n; i++) {
		if (k == n-i) {
			for (j = i; j < n; j++)
				r[j] = 1;
			return;
		}

		if (k == 0) {
			for (j = i; j < n; j++)
				r[j] = 0;
			return;
		}

		v = binomial(n-i-1, k);
		if (s >= v) {
			r[i] = 1;
			k--;
			s -= v;
		} else {
			r[i] = 0;
		}
	}
}

static void
int_to_digit_array(int a, int b, int n, int *r)
{
	int i;

	if (b <= 1)
		for (i = 0; i < n; i++)
			r[i] = 0;
	else
		for (i = 0; i < n; i++, a /= b)
			r[i] = a % b;
}

static void
int_to_sum_zero_array(int x, int b, int n, int *a)
{
	int i, s = 0;

	if (b <= 1) {
		for (i = 0; i < n; i++)
		    a[i] = 0;
	} else {
		int_to_digit_array(x, b, n-1, a);
		for (i = 0; i < n - 1; i++)
		    s = (s + a[i]) % b;
		a[n-1] = (b - s) % b;
	}
}

static int
invert_digits(int a, int b, int n)
{
	int i, ret, *r = malloc(n * sizeof(int));

	int_to_digit_array(a, b, n, r);
	for (i = 0; i < n; i++)
		r[i] = (b-r[i]) % b;

	ret = digit_array_to_int(r, n, b);
	free(r);
	return ret;
}

static bool
is_perm(int *a, int n)
{
	int *aux = malloc(n * sizeof(int));
	int i;
	
	for (i = 0; i < n; i++)
		if (a[i] < 0 || a[i] >= n)
			return false;
		else
			aux[a[i]] = 1;

	for (i = 0; i < n; i++)
		if (!aux[i])
			return false;

	free(aux);

	return true;
}

static bool
is_subset(int *a, int n, int k)
{
	int i, sum = 0;

	for (i = 0; i < n; i++)
		sum += a[i] ? 1 : 0;

	return sum == k;
}

static Cube
move_via_arrays(CubeArray *arr, Cube c, PieceFilter f)
{
	CubeArray *arrc = new_cubearray(c, f);
	Cube ret;

	if (f.epose || f.eposs || f.eposm)
		apply_permutation(arr->ep, arrc->ep, 12);

	if (f.eofb) {
		apply_permutation(arr->ep, arrc->eofb, 12);
		sum_arrays_mod(arr->eofb, arrc->eofb, 12, 2);
	}

	if (f.eorl) {
		apply_permutation(arr->ep, arrc->eorl, 12);
		sum_arrays_mod(arr->eorl, arrc->eorl, 12, 2);
	}

	if (f.eoud) {
		apply_permutation(arr->ep, arrc->eoud, 12);
		sum_arrays_mod(arr->eoud, arrc->eoud, 12, 2);
	}

	if (f.cp)
		apply_permutation(arr->cp, arrc->cp, 8);

	if (f.coud) {
		apply_permutation(arr->cp, arrc->coud, 8);
		sum_arrays_mod(arr->coud, arrc->coud, 8, 3);
	}

	if (f.corl) {
		apply_permutation(arr->cp, arrc->corl, 8);
		sum_arrays_mod(arr->corl, arrc->corl, 8, 3);
	}

	if (f.cofb) {
		apply_permutation(arr->cp, arrc->cofb, 8);
		sum_arrays_mod(arr->cofb, arrc->cofb, 8, 3);
	}

	if (f.cpos)
		apply_permutation(arr->cpos, arrc->cpos, 6);

	ret = arrays_to_cube(arrc, f);
	free_cubearray(arrc, f);

	return ret;
}

static void
moveset_to_list(bool (*ms)(Move m), int (*f)(Cube), Move *r)
{
	Cube c;
	int b[NMOVES];
	int na = 0, nb = 0;
	Move i;

	if (ms == NULL) {
		fprintf(stderr, "Error: no moveset given\n");
		return;
	}

	for (i = U; i < NMOVES; i++) {
		if (ms(i)) {
			c = apply_move(i, (Cube){0});
			if (f != NULL && f(c))
				r[na++] = i;
			else
				b[nb++] = i;
		}
	}

	memcpy(r + na, b, nb * sizeof(Move));
	r[na+nb] = NULLMOVE;
}

static AlgList *
new_alglist()
{
	AlgList *ret = malloc(sizeof(AlgList));

	ret->len   = 0;
	ret->first = NULL;
	ret->last  = NULL;

	return ret;
}

static CubeArray *
new_cubearray(Cube cube, PieceFilter f)
{
	CubeArray *arr = malloc(sizeof(CubeArray));

	if (f.epose || f.eposs || f.eposm)
		arr->ep   = malloc(12 * sizeof(int));
	if (f.eofb)
		arr->eofb = malloc(12 * sizeof(int));
	if (f.eorl)
		arr->eorl = malloc(12 * sizeof(int));
	if (f.eoud)
		arr->eoud = malloc(12 * sizeof(int));
	if (f.cp)
		arr->cp   = malloc(8  * sizeof(int));
	if (f.coud)
		arr->coud = malloc(8  * sizeof(int));
	if (f.corl)
		arr->corl = malloc(8  * sizeof(int));
	if (f.cofb)
		arr->cofb = malloc(8  * sizeof(int));
	if (f.cpos)
		arr->cpos = malloc(6  * sizeof(int));

	cube_to_arrays(cube, arr, f);

	return arr;
}

static int
perm_sign(int *a, int n)
{
	int i, j, ret = 0;

	if (!is_perm(a,n))
		return -1;

	for (i = 0; i < n; i++)
		for (j = i+1; j < n; j++)
			ret += (a[i] > a[j]) ? 1 : 0;

	return ret % 2;
}

static int
perm_to_index(int *a, int n)
{
	int i, j, c, ret = 0;

	if (!is_perm(a, n))
		return -1;

	for (i = 0; i < n; i++) {
		c = 0;
		for (j = i+1; j < n; j++)
			c += (a[i] > a[j]) ? 1 : 0;
		ret += factorial(n-i-1) * c;
	}

	return ret;
}

static int
powint(int a, int b)
{
	if (b < 0)
		return 0; /* Truncate */
	if (b == 0)
		return 1;

	if (b % 2)
		return a * powint(a, b-1);
	else
		return powint(a*a, b/2);
}

static void
ptable_set_reached(PruneData *pd, uint64_t ind)
{
	uint8_t oldval2 = pd->reached[ind/2];
	int other = ind % 2 ? oldval2 % 16 : oldval2 / 16;

	pd->reached[ind/2] = ind % 2 ? 16 + other : 16*other + 1;
}

static void
ptable_update(PruneData *pd, uint64_t ind, int n)
{
	uint8_t oldval2 = pd->ptable[ind/2];
	int other = ind % 2 ? oldval2 % 16 : oldval2 / 16;

	pd->ptable[ind/2] = ind % 2 ? 16*n + other : 16*other + n;
	pd->n++;
}

static bool
read_mtables_file()
{
	FILE *f;
	char fname[strlen(tabledir)+20];
	int m, b = sizeof(uint16_t);
	bool r = true;

	strcpy(fname, tabledir);
	strcat(fname, "/mtables");

	if ((f = fopen(fname, "rb")) == NULL)
		return false;

	for (m = 0; m < NMOVES; m++) {
		r = r && fread(epose_mtable[m], b, me[0],  f) == me[0];
		r = r && fread(eposs_mtable[m], b, me[1],  f) == me[1];
		r = r && fread(eposm_mtable[m], b, me[2],  f) == me[2];
		r = r && fread(eofb_mtable[m],  b, me[3],  f) == me[3];
		r = r && fread(eorl_mtable[m],  b, me[4],  f) == me[4];
		r = r && fread(eoud_mtable[m],  b, me[5],  f) == me[5];
		r = r && fread(cp_mtable[m],    b, me[6],  f) == me[6];
		r = r && fread(coud_mtable[m],  b, me[7],  f) == me[7];
		r = r && fread(corl_mtable[m],  b, me[8],  f) == me[8];
		r = r && fread(cofb_mtable[m],  b, me[9],  f) == me[9];
		r = r && fread(cpos_mtable[m],  b, me[10], f) == me[10];
	}

	fclose(f);
	return r;
}

static bool
read_ptable_file(PruneData *pd)
{
	FILE *f;
	char fname[strlen(tabledir)+100];
	uint64_t r;

	strcpy(fname, tabledir);
	strcat(fname, "/");
	strcat(fname, pd->filename);

	if ((f = fopen(fname, "rb")) == NULL)
		return false;

	r = fread(pd->ptable, sizeof(uint8_t), PTABLESIZE(pd->size), f);
	fclose(f);

	return r == PTABLESIZE(pd->size);
}

static bool
read_ttables_file()
{
	FILE *f;
	char fname[strlen(tabledir)+20];
	int b = sizeof(uint16_t);
	bool r = true;
	Move m;

	strcpy(fname, tabledir);
	strcat(fname, "/");
	strcat(fname, "ttables");

	if ((f = fopen(fname, "rb")) == NULL)
		return false;

	for (m = 0; m < NTRANS; m++) {
		r = r && fread(epose_ttable[m], b, me[0],  f) == me[0];
		r = r && fread(eposs_ttable[m], b, me[1],  f) == me[1];
		r = r && fread(eposm_ttable[m], b, me[2],  f) == me[2];
		r = r && fread(eo_ttable[m],    b, me[3],  f) == me[3];
		r = r && fread(cp_ttable[m],    b, me[6],  f) == me[6];
		r = r && fread(co_ttable[m],    b, me[7],  f) == me[7];
		r = r && fread(cpos_ttable[m],  b, me[10], f) == me[10];
		r = r && fread(moves_ttable[m], b, me[11], f) == me[11];
	}

	fclose(f);
	return r;
}

static Cube
rotate_via_compose(Trans r, Cube c, PieceFilter f)
{
	Alg *inv;
	static int zero12[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	static int zero8[8]   = { 0, 0, 0, 0, 0, 0, 0, 0 };
	static CubeArray ma = {
		.ep   = ep_mirror,
		.eofb = zero12,
		.eorl = zero12,
		.eoud = zero12,
		.cp   = cp_mirror,
		.coud = zero8,
		.corl = zero8,
		.cofb = zero8,
		.cpos = cpos_mirror
	};

	Cube ret;

	if (r != mirror) {
		ret = apply_alg_generic(trans_algs[r], c, f, true);
		inv = on_inverse(trans_algs[r]);
		ret = apply_alg_generic(inv, ret, f, true);
		free_alg(inv);
	} else {
		ret = move_via_arrays(&ma, (Cube){0}, f);
		ret = compose_filtered(c, ret, f);
		ret = move_via_arrays(&ma, ret, f);
	}

	return ret;
}

static void
solve_dfs(Cube c, Step s, SolveOptions *opts, DfsData *dd)
{
	Move move, l1 = dd->last1, l2 = dd->last2;
	int i, lower_bound = s.check(c);
	bool found_many, prune, niss_make_sense, nissbackup = dd->niss;

	if (opts->can_niss && !dd->niss)
		lower_bound = MIN(1, lower_bound);

	found_many = dd->sols->len >= opts->max_solutions;
	prune = dd->current_alg->len + lower_bound > dd->d;
	if (found_many || prune)
		return;

	if (!lower_bound) {        /* solved */
		if (dd->current_alg->len == dd->d)
			append_alg(dd->sols, dd->current_alg);
		return;
	}

	dd->last2 = dd->last1;

	for (i = 0; dd->sorted_moves[i] != NULLMOVE; i++) {
		move = dd->sorted_moves[i];
		if (possible_next[l2][l1][move]) {
			dd->last1 = move;
			append_move(dd->current_alg, move, dd->niss);
			solve_dfs(apply_move(move, c), s, opts, dd);
			remove_last_move(dd->current_alg);
		}
	}

	niss_make_sense = !dd->current_alg->len ||
	                   (s.check(apply_move(l1,(Cube){0})));
	if (opts->can_niss && !dd->niss && niss_make_sense) {
		dd->niss  = true;
		dd->last1 = NULLMOVE;
		dd->last2 = NULLMOVE;
		solve_dfs(inverse_cube(c), s, opts, dd);
	}

	dd->last1 = l1;
	dd->last2 = l2;
	dd->niss  = nissbackup;
}

static int 
subset_to_index(int *a, int n, int k)
{
	int i, ret = 0;

	if (!is_subset(a, n, k))
		return binomial(n, k);

	for (i = 0; i < n; i++) {
		if (k == n-i)
			return ret;
		if (a[i]) {
			ret += binomial(n-i-1, k);
			k--;
		}
	}

	return ret;
}

static void
sum_arrays_mod(int *src, int *dst, int n, int m)
{
	int i;

	for (i = 0; i < n; i++)
		dst[i] = (m <= 0) ? 0 : (src[i] + dst[i]) % m;
}

static void
swap(int *a, int *b)
{
	int aux;

	aux = *a;
	*a = *b;
	*b = aux;
}

static bool
write_mtables_file()
{
	FILE *f;
	char fname[strlen(tabledir)+20];
	int m, b = sizeof(uint16_t);
	bool r = true;

	strcpy(fname, tabledir);
	strcat(fname, "/mtables");

	if ((f = fopen(fname, "wb")) == NULL)
		return false;

	for (m = 0; m < NMOVES; m++) {
		r = r && fwrite(epose_mtable[m], b, me[0],  f) == me[0];
		r = r && fwrite(eposs_mtable[m], b, me[1],  f) == me[1];
		r = r && fwrite(eposm_mtable[m], b, me[2],  f) == me[2];
		r = r && fwrite(eofb_mtable[m],  b, me[3],  f) == me[3];
		r = r && fwrite(eorl_mtable[m],  b, me[4],  f) == me[4];
		r = r && fwrite(eoud_mtable[m],  b, me[5],  f) == me[5];
		r = r && fwrite(cp_mtable[m],    b, me[6],  f) == me[6];
		r = r && fwrite(coud_mtable[m],  b, me[7],  f) == me[7];
		r = r && fwrite(corl_mtable[m],  b, me[8],  f) == me[8];
		r = r && fwrite(cofb_mtable[m],  b, me[9],  f) == me[9];
		r = r && fwrite(cpos_mtable[m],  b, me[10], f) == me[10];
	}

	fclose(f);
	return r;
}

static bool
write_ptable_file(PruneData *pd)
{
	FILE *f;
	char fname[strlen(tabledir)+100];
	uint64_t written;

	strcpy(fname, tabledir);
	strcat(fname, "/");
	strcat(fname, pd->filename);

	if ((f = fopen(fname, "wb")) == NULL)
		return false;

	written = fwrite(pd->ptable, sizeof(uint8_t), PTABLESIZE(pd->size), f);
	fclose(f);

	return written == PTABLESIZE(pd->size);
}

static bool
write_ttables_file()
{
	FILE *f;
	char fname[strlen(tabledir)+20];
	bool r = true;
	int b = sizeof(uint16_t);
	Move m;

	strcpy(fname, tabledir);
	strcat(fname, "/ttables");

	if ((f = fopen(fname, "wb")) == NULL)
		return false;

	for (m = 0; m < NTRANS; m++) {
		r = r && fwrite(epose_ttable[m], b, me[0],  f) == me[0];
		r = r && fwrite(eposs_ttable[m], b, me[1],  f) == me[1];
		r = r && fwrite(eposm_ttable[m], b, me[2],  f) == me[2];
		r = r && fwrite(eo_ttable[m],    b, me[3],  f) == me[3];
		r = r && fwrite(cp_ttable[m],    b, me[6],  f) == me[6];
		r = r && fwrite(co_ttable[m],    b, me[7],  f) == me[7];
		r = r && fwrite(cpos_ttable[m],  b, me[10], f) == me[10];
		r = r && fwrite(moves_ttable[m], b, me[11], f) == me[11];
	}

	fclose(f);
	return r;
}

/* Init functions implementation *********************************************/

static void
init_auxtables()
{
	Cube c1, c2;
	uint64_t ui, uj;
	int i, j, k;
	bool cij, p1, p2;

	for (ui = 0; ui < BINOM12ON4; ui++)
		for (uj = 0; uj < BINOM12ON4; uj++)
			epos_dependent_aux[ui][uj] = epos_dependent(ui, uj);

	for (i = 0; i < NMOVES; i++) {
		for (j = 0; j < NMOVES; j++) {
			c1 = apply_move(i, apply_move(j, (Cube){0}));
			c2 = apply_move(j, apply_move(i, (Cube){0}));
			commute[i][j] = equal(c1, c2);
		}
	}

	for (i = 0; i < NMOVES; i++) {
		for (j = 0; j < NMOVES; j++) {
			for (k = 0; k < NMOVES; k++) {
				p1 = j && base_move(j) == base_move(k);
				p2 = i && base_move(i) == base_move(k);
				cij = commute[i][j];
				possible_next[i][j][k] = !(p1 || (cij && p2));
			}
		}
	}

	for (i = 0; i < NMOVES; i++)
		inverse_move_aux[i] = i ? i + 2 - 2*((i-1)%3) : NULLMOVE;

	/* Is there a more elegant way? */
	inverse_trans_aux[uf] = uf;
	inverse_trans_aux[ur] = ul;
	inverse_trans_aux[ul] = ur;
	inverse_trans_aux[ub] = ub;

	inverse_trans_aux[df] = df;
	inverse_trans_aux[dr] = dr;
	inverse_trans_aux[dl] = dl;
	inverse_trans_aux[db] = db;

	inverse_trans_aux[rf] = lf;
	inverse_trans_aux[rd] = bl;
	inverse_trans_aux[rb] = rb;
	inverse_trans_aux[ru] = fr;

	inverse_trans_aux[lf] = rf;
	inverse_trans_aux[ld] = br;
	inverse_trans_aux[lb] = lb;
	inverse_trans_aux[lu] = fl;

	inverse_trans_aux[fu] = fu;
	inverse_trans_aux[fr] = ru;
	inverse_trans_aux[fd] = bu;
	inverse_trans_aux[fl] = lu;

	inverse_trans_aux[bu] = fd;
	inverse_trans_aux[br] = ld;
	inverse_trans_aux[bd] = bd;
	inverse_trans_aux[bl] = rd;

	inverse_trans_aux[mirror] = mirror;
}

static void
init_environment()
{
	char *nissydata = getenv("NISSYDATA");
	char *localdata = getenv("XDG_DATA_HOME");
	char *home      = getenv("HOME");
	bool read, write;

	if (nissydata != NULL) {
		tabledir = malloc(strlen(nissydata) * sizeof(char) + 20);
		strcpy(tabledir, nissydata);
	} else if (localdata != NULL) {
		tabledir = malloc(strlen(localdata) * sizeof(char) + 20);
		strcpy(tabledir, localdata);
		strcat(tabledir, "/nissy");
	} else if (home != NULL) {
		tabledir = malloc(strlen(home) * sizeof(char) + 20);
		strcpy(tabledir, home);
		strcat(tabledir, "/.nissy");
	}

	mkdir(tabledir, 0777);
	strcat(tabledir, "/tables");
	mkdir(tabledir, 0777);

	read  = !access(tabledir, R_OK);
	write = !access(tabledir, W_OK);

	if (!read) {
		fprintf(stderr, "Table files cannot be read.\n");
	} else if (!write) {
		fprintf(stderr, "Data directory not writable: ");
		fprintf(stderr, "tables can be loaded, but not saved.\n");
	}
}

static void
init_moves() {
	Cube c;
	CubeArray arrs;
	int i;
	uint16_t ui;
	Move m;

	/* Generate all move cycles and flips; I do this regardless */
	for (i = 0; i < NMOVES; i++) {
		if (i == U || i == x || i == y)
			continue;

		c = apply_alg_generic(equiv_alg[i], (Cube){0}, pf_all, false);

		arrs = (CubeArray){
			edge_cycle[i],
			eofb_flipped[i],
			eorl_flipped[i],
			eoud_flipped[i],
			corner_cycle[i],
			coud_flipped[i],
			corl_flipped[i],
			cofb_flipped[i],
			center_cycle[i]
		};
		cube_to_arrays(c, &arrs, pf_all);
	}

	if (read_mtables_file())
		return;

	fprintf(stderr, "Cannot load %s, generating it\n", "mtables"); 

	/* Initialize transition tables */
	for (m = 0; m < NMOVES; m++) {
		for (ui = 0; ui < FACTORIAL12/FACTORIAL8; ui++) {
			c = (Cube){ .epose = ui };
			c = apply_move_cubearray(m, c, pf_e);
			epose_mtable[m][ui] = c.epose;

			c = (Cube){ .eposs = ui };
			c = apply_move_cubearray(m, c, pf_s);
			eposs_mtable[m][ui] = c.eposs;

			c = (Cube){ .eposm = ui };
			c = apply_move_cubearray(m, c, pf_m);
			eposm_mtable[m][ui] = c.eposm;
		}
		for (ui = 0; ui < POW2TO11; ui++ ) {
			c = (Cube){ .eofb = ui };
			c = apply_move_cubearray(m, c, pf_eo);
			eofb_mtable[m][ui] = c.eofb;

			c = (Cube){ .eorl = ui };
			c = apply_move_cubearray(m, c, pf_eo);
			eorl_mtable[m][ui] = c.eorl;

			c = (Cube){ .eoud = ui };
			c = apply_move_cubearray(m, c, pf_eo);
			eoud_mtable[m][ui] = c.eoud;
		}
		for (ui = 0; ui < POW3TO7; ui++) {
			c = (Cube){ .coud = ui };
			c = apply_move_cubearray(m, c, pf_co);
			coud_mtable[m][ui] = c.coud;

			c = (Cube){ .corl = ui };
			c = apply_move_cubearray(m, c, pf_co);
			corl_mtable[m][ui] = c.corl;

			c = (Cube){ .cofb = ui };
			c = apply_move_cubearray(m, c, pf_co);
			cofb_mtable[m][ui] = c.cofb;
		}
		for (ui = 0; ui < FACTORIAL8; ui++) {
			c = (Cube){ .cp = ui };
			c = apply_move_cubearray(m, c, pf_cp);
			cp_mtable[m][ui] = c.cp;
		}
		for (ui = 0; ui < FACTORIAL6; ui++) {
			c = (Cube){ .cpos = ui };
			c = apply_move_cubearray(m, c, pf_cpos);
			cpos_mtable[m][ui] = c.cpos;
		}
	}

	if (!write_mtables_file())
		fprintf(stderr, "Error writing mtables\n");
}

static void
init_moves_aux()
{
	/* Some standard PieceFilters */
	pf_all.epose  = true;
	pf_all.eposs  = true;
	pf_all.eposm  = true;
	pf_all.eofb   = true;
	pf_all.eorl   = true;
	pf_all.eoud   = true;
	pf_all.cp     = true;
	pf_all.cofb   = true;
	pf_all.corl   = true;
	pf_all.coud   = true;
	pf_all.cpos   = true;

	pf_4val.epose = true;
	pf_4val.eposs = true;
	pf_4val.eposm = true;
	pf_4val.eofb  = true;
	pf_4val.coud  = true;
	pf_4val.cp    = true;

	pf_epcp.epose = true;
	pf_epcp.eposs = true;
	pf_epcp.eposm = true;
	pf_epcp.cp    = true;

	pf_cpos.cpos  = true;

	pf_cp.cp      = true;

	pf_ep.epose   = true;
	pf_ep.eposs   = true;
	pf_ep.eposm   = true;

	pf_e.epose    = true;
	pf_s.eposs    = true;
	pf_m.eposm    = true;

	pf_eo.eofb    = true;
	pf_eo.eorl    = true;
	pf_eo.eoud    = true;

	pf_co.cofb    = true;
	pf_co.corl    = true;
	pf_co.coud    = true;

	/* Used to convert to and from CubeArray */
	epe_solved[0] = FR;
	epe_solved[1] = FL;
	epe_solved[2] = BL;
	epe_solved[3] = BR;

	eps_solved[0] = UL;
	eps_solved[1] = UR;
	eps_solved[2] = DL;
	eps_solved[3] = DR;

	epm_solved[0] = UF;
	epm_solved[1] = UB;
	epm_solved[2] = DF;
	epm_solved[3] = DB;

	/* Table sizes, used for reading and writing files */
	me[0]  = FACTORIAL12/FACTORIAL8;
	me[1]  = FACTORIAL12/FACTORIAL8;
	me[2]  = FACTORIAL12/FACTORIAL8;
	me[3]  = POW2TO11;
	me[4]  = POW2TO11;
	me[5]  = POW2TO11;
	me[6]  = FACTORIAL8;
	me[7]  = POW3TO7;
	me[8]  = POW3TO7;
	me[9]  = POW3TO7;
	me[10] = FACTORIAL6;
	me[11] = NMOVES;

	/* Cycles *********************/
	edge_cycle[U][UF] = UR;
	edge_cycle[U][UL] = UF;
	edge_cycle[U][UB] = UL;
	edge_cycle[U][UR] = UB;
	edge_cycle[U][DF] = DF;
	edge_cycle[U][DL] = DL;
	edge_cycle[U][DB] = DB;
	edge_cycle[U][DR] = DR;
	edge_cycle[U][FR] = FR;
	edge_cycle[U][FL] = FL;
	edge_cycle[U][BL] = BL;
	edge_cycle[U][BR] = BR;

	edge_cycle[x][UF] = DF;
	edge_cycle[x][UL] = FL;
	edge_cycle[x][UB] = UF;
	edge_cycle[x][UR] = FR;
	edge_cycle[x][DF] = DB;
	edge_cycle[x][DL] = BL;
	edge_cycle[x][DB] = UB;
	edge_cycle[x][DR] = BR;
	edge_cycle[x][FR] = DR;
	edge_cycle[x][FL] = DL;
	edge_cycle[x][BL] = UL;
	edge_cycle[x][BR] = UR;

	edge_cycle[y][UF] = UR;
	edge_cycle[y][UL] = UF;
	edge_cycle[y][UB] = UL;
	edge_cycle[y][UR] = UB;
	edge_cycle[y][DF] = DR;
	edge_cycle[y][DL] = DF;
	edge_cycle[y][DB] = DL;
	edge_cycle[y][DR] = DB;
	edge_cycle[y][FR] = BR;
	edge_cycle[y][FL] = FR;
	edge_cycle[y][BL] = FL;
	edge_cycle[y][BR] = BL;

	corner_cycle[U][UFR] = UBR;
	corner_cycle[U][UFL] = UFR;
	corner_cycle[U][UBL] = UFL;
	corner_cycle[U][UBR] = UBL;
	corner_cycle[U][DFR] = DFR;
	corner_cycle[U][DFL] = DFL;
	corner_cycle[U][DBL] = DBL;
	corner_cycle[U][DBR] = DBR;

	corner_cycle[x][UFR] = DFR;
	corner_cycle[x][UFL] = DFL;
	corner_cycle[x][UBL] = UFL;
	corner_cycle[x][UBR] = UFR;
	corner_cycle[x][DFR] = DBR;
	corner_cycle[x][DFL] = DBL;
	corner_cycle[x][DBL] = UBL;
	corner_cycle[x][DBR] = UBR;

	corner_cycle[y][UFR] = UBR;
	corner_cycle[y][UFL] = UFR;
	corner_cycle[y][UBL] = UFL;
	corner_cycle[y][UBR] = UBL;
	corner_cycle[y][DFR] = DBR;
	corner_cycle[y][DFL] = DFR;
	corner_cycle[y][DBL] = DFL;
	corner_cycle[y][DBR] = DBL;

	center_cycle[U][U_center] = U_center;
	center_cycle[U][D_center] = D_center;
	center_cycle[U][R_center] = R_center;
	center_cycle[U][L_center] = L_center;
	center_cycle[U][F_center] = F_center;
	center_cycle[U][B_center] = B_center;

	center_cycle[x][U_center] = F_center;
	center_cycle[x][D_center] = B_center;
	center_cycle[x][R_center] = R_center;
	center_cycle[x][L_center] = L_center;
	center_cycle[x][F_center] = D_center;
	center_cycle[x][B_center] = U_center;

	center_cycle[y][U_center] = U_center;
	center_cycle[y][D_center] = D_center;
	center_cycle[y][R_center] = B_center;
	center_cycle[y][L_center] = F_center;
	center_cycle[y][F_center] = R_center;
	center_cycle[y][B_center] = L_center;

	/* Flipped pieces *************/
	eofb_flipped[x][UF] = 1;
	eofb_flipped[x][UB] = 1;
	eofb_flipped[x][DF] = 1;
	eofb_flipped[x][DB] = 1;

	eofb_flipped[y][FR] = 1;
	eofb_flipped[y][FL] = 1;
	eofb_flipped[y][BL] = 1;
	eofb_flipped[y][BR] = 1;

	eorl_flipped[x][UF] = 1;
	eorl_flipped[x][UL] = 1;
	eorl_flipped[x][UB] = 1;
	eorl_flipped[x][UR] = 1;
	eorl_flipped[x][DF] = 1;
	eorl_flipped[x][DL] = 1;
	eorl_flipped[x][DB] = 1;
	eorl_flipped[x][DR] = 1;
	eorl_flipped[x][FR] = 1;
	eorl_flipped[x][FL] = 1;
	eorl_flipped[x][BL] = 1;
	eorl_flipped[x][BR] = 1;

	eorl_flipped[y][FR] = 1;
	eorl_flipped[y][FL] = 1;
	eorl_flipped[y][BL] = 1;
	eorl_flipped[y][BR] = 1;

	eoud_flipped[U][UF] = 1;
	eoud_flipped[U][UL] = 1;
	eoud_flipped[U][UB] = 1;
	eoud_flipped[U][UR] = 1;

	eoud_flipped[x][UF] = 1;
	eoud_flipped[x][UB] = 1;
	eoud_flipped[x][DF] = 1;
	eoud_flipped[x][DB] = 1;

	eoud_flipped[y][UF] = 1;
	eoud_flipped[y][UL] = 1;
	eoud_flipped[y][UB] = 1;
	eoud_flipped[y][UR] = 1;
	eoud_flipped[y][DF] = 1;
	eoud_flipped[y][DL] = 1;
	eoud_flipped[y][DB] = 1;
	eoud_flipped[y][DR] = 1;
	eoud_flipped[y][FR] = 1;
	eoud_flipped[y][FL] = 1;
	eoud_flipped[y][BL] = 1;
	eoud_flipped[y][BR] = 1;

	coud_flipped[x][UFR] = 2;
	coud_flipped[x][UFL] = 1;
	coud_flipped[x][UBR] = 1;
	coud_flipped[x][UBL] = 2;
	coud_flipped[x][DFR] = 1;
	coud_flipped[x][DFL] = 2;
	coud_flipped[x][DBR] = 2;
	coud_flipped[x][DBL] = 1;

	corl_flipped[U][UFR] = 1;
	corl_flipped[U][UFL] = 2;
	corl_flipped[U][UBL] = 1;
	corl_flipped[U][UBR] = 2;

	corl_flipped[y][UFR] = 1;
	corl_flipped[y][UFL] = 2;
	corl_flipped[y][UBL] = 1;
	corl_flipped[y][UBR] = 2;
	corl_flipped[y][DFR] = 2;
	corl_flipped[y][DFL] = 1;
	corl_flipped[y][DBL] = 2;
	corl_flipped[y][DBR] = 1;

	cofb_flipped[U][UFR] = 2;
	cofb_flipped[U][UFL] = 1;
	cofb_flipped[U][UBL] = 2;
	cofb_flipped[U][UBR] = 1;

	cofb_flipped[x][UFR] = 1;
	cofb_flipped[x][UFL] = 2;
	cofb_flipped[x][UBL] = 1;
	cofb_flipped[x][UBR] = 2;
	cofb_flipped[x][DFR] = 2;
	cofb_flipped[x][DFL] = 1;
	cofb_flipped[x][DBL] = 2;
	cofb_flipped[x][DBR] = 1;

	cofb_flipped[y][UFR] = 2;
	cofb_flipped[y][UFL] = 1;
	cofb_flipped[y][UBL] = 2;
	cofb_flipped[y][UBR] = 1;
	cofb_flipped[y][DFR] = 1;
	cofb_flipped[y][DFL] = 2;
	cofb_flipped[y][DBL] = 1;
	cofb_flipped[y][DBR] = 2;

	/* Equivalent moves ***********/
	equiv_alg[NULLMOVE] = new_alg("");

	equiv_alg[U]   = new_alg("        U           ");
	equiv_alg[U2]  = new_alg("        UU          ");
	equiv_alg[U3]  = new_alg("        UUU         ");
	equiv_alg[D]   = new_alg("  xx    U    xx      ");
	equiv_alg[D2]  = new_alg("  xx    UU   xx      ");
	equiv_alg[D3]  = new_alg("  xx    UUU  xx      ");
	equiv_alg[R]   = new_alg("  yx    U    xxxyyy  ");
	equiv_alg[R2]  = new_alg("  yx    UU   xxxyyy  ");
	equiv_alg[R3]  = new_alg("  yx    UUU  xxxyyy  ");
	equiv_alg[L]   = new_alg("  yyyx  U    xxxy    ");
	equiv_alg[L2]  = new_alg("  yyyx  UU   xxxy    ");
	equiv_alg[L3]  = new_alg("  yyyx  UUU  xxxy    ");
	equiv_alg[F]   = new_alg("  x     U    xxx     ");
	equiv_alg[F2]  = new_alg("  x     UU   xxx     ");
	equiv_alg[F3]  = new_alg("  x     UUU  xxx     ");
	equiv_alg[B]   = new_alg("  xxx   U    x       ");
	equiv_alg[B2]  = new_alg("  xxx   UU   x       ");
	equiv_alg[B3]  = new_alg("  xxx   UUU  x       ");

	equiv_alg[Uw]  = new_alg("  xx    U    xx      y        ");
	equiv_alg[Uw2] = new_alg("  xx    UU   xx      yy       ");
	equiv_alg[Uw3] = new_alg("  xx    UUU  xx      yyy      ");
	equiv_alg[Dw]  = new_alg("        U            yyy      ");
	equiv_alg[Dw2] = new_alg("        UU           yy       ");
	equiv_alg[Dw3] = new_alg("        UUU          y        ");
	equiv_alg[Rw]  = new_alg("  yyyx  U    xxxy    x        ");
	equiv_alg[Rw2] = new_alg("  yyyx  UU   xxxy    xx       ");
	equiv_alg[Rw3] = new_alg("  yyyx  UUU  xxxy    xxx      ");
	equiv_alg[Lw]  = new_alg("  yx    U    xxxyyy  xxx      ");
	equiv_alg[Lw2] = new_alg("  yx    UU   xxxyyy  xx       ");
	equiv_alg[Lw3] = new_alg("  yx    UUU  xxxyyy  x        ");
	equiv_alg[Fw]  = new_alg("  xxx   U    x       yxxxyyy  ");
	equiv_alg[Fw2] = new_alg("  xxx   UU   x       yxxyyy   ");
	equiv_alg[Fw3] = new_alg("  xxx   UUU  x       yxyyy    ");
	equiv_alg[Bw]  = new_alg("  x     U    xxx     yxyyy    ");
	equiv_alg[Bw2] = new_alg("  x     UU   xxx     yxxyyy   ");
	equiv_alg[Bw3] = new_alg("  x     UUU  xxx     yxxxyyy  ");

	equiv_alg[M]   = new_alg("  yx  U    xx  UUU  yxyyy  ");
	equiv_alg[M2]  = new_alg("  yx  UU   xx  UU   xxxy   ");
	equiv_alg[M3]  = new_alg("  yx  UUU  xx  U    yxxxy  ");
	equiv_alg[S]   = new_alg("  x   UUU  xx  U    yyyx   ");
	equiv_alg[S2]  = new_alg("  x   UU   xx  UU   yyx    ");
	equiv_alg[S3]  = new_alg("  x   U    xx  UUU  yx     ");
	equiv_alg[E]   = new_alg("      U    xx  UUU  xxyyy  ");
	equiv_alg[E2]  = new_alg("      UU   xx  UU   xxyy   ");
	equiv_alg[E3]  = new_alg("      UUU  xx  U    xxy    ");

	equiv_alg[x]   = new_alg("       x         ");
	equiv_alg[x2]  = new_alg("       xx        ");
	equiv_alg[x3]  = new_alg("       xxx       ");
	equiv_alg[y]   = new_alg("       y         ");
	equiv_alg[y2]  = new_alg("       yy        ");
	equiv_alg[y3]  = new_alg("       yyy       ");
	equiv_alg[z]   = new_alg("  yyy  x    y    ");
	equiv_alg[z2]  = new_alg("  yy   xx        ");
	equiv_alg[z3]  = new_alg("  y    x    yyy  ");
}

static void
init_steps()
{
	/* PruneData */
	pd_eofb_HTM.filename = "ptable_eofb_HTM";
	pd_eofb_HTM.size     = POW2TO11;
	pd_eofb_HTM.index    = index_eofb;
	pd_eofb_HTM.moveset  = moveset_HTM;

	pd_coud_HTM.filename = "ptable_coud_HTM";
	pd_coud_HTM.size     = POW3TO7;
	pd_coud_HTM.index    = index_coud;
	pd_coud_HTM.moveset  = moveset_HTM;

	pd_corners_HTM.filename = "ptable_corners_HTM";
	pd_corners_HTM.size     = POW3TO7 * FACTORIAL8;
	pd_corners_HTM.index    = index_corners;
	pd_corners_HTM.moveset  = moveset_HTM;

	pd_ep_HTM.filename = "ptable_ep_HTM";
	pd_ep_HTM.size     = FACTORIAL12;
	pd_ep_HTM.index    = index_ep;
	pd_ep_HTM.moveset  = moveset_HTM;

	pd_drud_HTM.filename = "ptable_drud_HTM";
	pd_drud_HTM.size     = POW2TO11 * POW3TO7 * BINOM12ON4;
	pd_drud_HTM.index    = index_drud;
	pd_drud_HTM.moveset  = moveset_HTM;


	/* Actual steps */
	eofb_HTM.check     = check_eofb_HTM;
	eofb_HTM.ready     = check_nothing;
	eofb_HTM.pre_trans = uf;
	eofb_HTM.moveset   = moveset_HTM;

	eorl_HTM.check     = check_eofb_HTM;
	eorl_HTM.ready     = check_nothing;
	eorl_HTM.pre_trans = ur;
	eorl_HTM.moveset   = moveset_HTM;

	eoud_HTM.check     = check_eofb_HTM;
	eoud_HTM.ready     = check_nothing;
	eoud_HTM.pre_trans = bu;
	eoud_HTM.moveset   = moveset_HTM;


	coud_HTM.check     = check_coud_HTM;
	coud_HTM.ready     = check_nothing;
	coud_HTM.pre_trans = uf;
	coud_HTM.moveset   = moveset_HTM;

	corl_HTM.check     = check_coud_HTM;
	corl_HTM.ready     = check_nothing;
	corl_HTM.pre_trans = rf;
	corl_HTM.moveset   = moveset_HTM;

	cofb_HTM.check     = check_coud_HTM;
	cofb_HTM.ready     = check_nothing;
	cofb_HTM.pre_trans = fd;
	cofb_HTM.moveset   = moveset_HTM;

	coud_URF.check     = check_coud_URF;
	coud_URF.ready     = check_nothing;
	coud_URF.pre_trans = uf;
	coud_URF.moveset   = moveset_URF;

	corl_URF.check     = check_coud_URF;
	corl_URF.ready     = check_nothing;
	corl_URF.pre_trans = rf;
	corl_URF.moveset   = moveset_URF;

	cofb_URF.check     = check_coud_URF;
	cofb_URF.ready     = check_nothing;
	cofb_URF.pre_trans = fd;
	cofb_URF.moveset   = moveset_URF;

	corners_HTM.check     = check_corners_HTM;
	corners_HTM.ready     = check_nothing;
	corners_HTM.pre_trans = uf;
	corners_HTM.moveset   = moveset_HTM;

	corners_URF.check     = check_corners_URF;
	corners_URF.ready     = check_nothing;
	corners_URF.pre_trans = uf;
	corners_URF.moveset   = moveset_URF;

	edges_HTM.check     = check_edges_HTM;
	edges_HTM.ready     = check_nothing;
	edges_HTM.pre_trans = uf;
	edges_HTM.moveset   = moveset_HTM;

	drud_HTM.check     = check_drud_HTM;
	drud_HTM.ready     = check_nothing;
	drud_HTM.pre_trans = uf;
	drud_HTM.moveset   = moveset_HTM;

	optimal_HTM.check     = check_optimal_HTM;
	optimal_HTM.ready     = check_nothing;
	optimal_HTM.pre_trans = uf;
	optimal_HTM.moveset   = moveset_HTM;
}

static void
init_strings()
{
	strcpy(move_string    [NULLMOVE],  "-"     );
	strcpy(move_string    [U],         "U"     );
	strcpy(move_string    [U2],        "U2"    );
	strcpy(move_string    [U3],        "U\'"   );
	strcpy(move_string    [D],         "D"     );
	strcpy(move_string    [D2],        "D2"    );
	strcpy(move_string    [D3],        "D\'"   );
	strcpy(move_string    [R],         "R"     );
	strcpy(move_string    [R2],        "R2"    );
	strcpy(move_string    [R3],        "R\'"   );
	strcpy(move_string    [L],         "L"     );
	strcpy(move_string    [L2],        "L2"    );
	strcpy(move_string    [L3],        "L\'"   );
	strcpy(move_string    [F],         "F"     );
	strcpy(move_string    [F2],        "F2"    );
	strcpy(move_string    [F3],        "F\'"   );
	strcpy(move_string    [B],         "B"     );
	strcpy(move_string    [B2],        "B2"    );
	strcpy(move_string    [B3],        "B\'"   );
	strcpy(move_string    [Uw],        "Uw"    );
	strcpy(move_string    [Uw2],       "Uw2"   );
	strcpy(move_string    [Uw3],       "Uw\'"  );
	strcpy(move_string    [Dw],        "Dw"    );
	strcpy(move_string    [Dw2],       "Dw2"   );
	strcpy(move_string    [Dw3],       "Dw\'"  );
	strcpy(move_string    [Rw],        "Rw"    );
	strcpy(move_string    [Rw2],       "Rw2"   );
	strcpy(move_string    [Rw3],       "Rw\'"  );
	strcpy(move_string    [Lw],        "Lw"    );
	strcpy(move_string    [Lw2],       "Lw2"   );
	strcpy(move_string    [Lw3],       "Lw\'"  );
	strcpy(move_string    [Fw],        "Fw"    );
	strcpy(move_string    [Fw2],       "Fw2"   );
	strcpy(move_string    [Fw3],       "Fw\'"  );
	strcpy(move_string    [Bw],        "Bw"    );
	strcpy(move_string    [Bw2],       "Bw2"   );
	strcpy(move_string    [Bw3],       "Bw\'"  );
	strcpy(move_string    [M],         "M"     );
	strcpy(move_string    [M2],        "M2"    );
	strcpy(move_string    [M3],        "M\'"   );
	strcpy(move_string    [S],         "S"     );
	strcpy(move_string    [S2],        "S2"    );
	strcpy(move_string    [S3],        "S\'"   );
	strcpy(move_string    [E],         "E"     );
	strcpy(move_string    [E2],        "E2"    );
	strcpy(move_string    [E3],        "E\'"   );
	strcpy(move_string    [x],         "x"     );
	strcpy(move_string    [x2],        "x2"    );
	strcpy(move_string    [x3],        "x\'"   );
	strcpy(move_string    [y],         "y"     );
	strcpy(move_string    [y2],        "y2"    );
	strcpy(move_string    [y3],        "y\'"   );
	strcpy(move_string    [z],         "z"     );
	strcpy(move_string    [z2],        "z2"    );
	strcpy(move_string    [z3],        "z\'"   );

	strcpy(edge_string    [UF],        "UF"    );
	strcpy(edge_string    [UL],        "UL"    );
	strcpy(edge_string    [UB],        "UB"    );
	strcpy(edge_string    [UR],        "UR"    );
	strcpy(edge_string    [DF],        "DF"    );
	strcpy(edge_string    [DL],        "DL"    );
	strcpy(edge_string    [DB],        "DB"    );
	strcpy(edge_string    [DR],        "DR"    );
	strcpy(edge_string    [FR],        "FR"    );
	strcpy(edge_string    [FL],        "FL"    );
	strcpy(edge_string    [BL],        "BL"    );
	strcpy(edge_string    [BR],        "BR"    );
 
	strcpy(corner_string  [UFR],       "UFR"   );
	strcpy(corner_string  [UFL],       "UFL"   );
	strcpy(corner_string  [UBL],       "UBL"   );
	strcpy(corner_string  [UBR],       "UBR"   );
	strcpy(corner_string  [DFR],       "DFR"   );
	strcpy(corner_string  [DFL],       "DFL"   );
	strcpy(corner_string  [DBL],       "DBL"   );
	strcpy(corner_string  [DBR],       "DBR"   );

	strcpy(center_string  [U_center],  "U"     );
	strcpy(center_string  [D_center],  "D"     );
	strcpy(center_string  [R_center],  "R"     );
	strcpy(center_string  [L_center],  "L"     );
	strcpy(center_string  [F_center],  "F"     );
	strcpy(center_string  [B_center],  "B"     );
}

static void
init_trans() {
	Cube aux, cube, c[3];
	CubeArray epcp;
	int eparr[12], eoarr[12];
	int cparr[8], coarr[8];
	int i;
	bool b1, b2, b3;
	uint16_t ui;
	Move mi, move;
	Trans m;

	/* Compute sources */
	for (i = 0; i < NTRANS; i++) {
		if (i == mirror)
			cube = (Cube){0};
		else
			cube = apply_alg(trans_algs[i], (Cube){0});

		epose_source[i] = edge_slice(edge_at(cube, FR));
		eposs_source[i] = edge_slice(edge_at(cube, UR));
		eposm_source[i] = edge_slice(edge_at(cube, UF));
		eofb_source[i]  = center_at(cube, F_center)/2;
		eorl_source[i]  = center_at(cube, R_center)/2;
		eoud_source[i]  = center_at(cube, U_center)/2;
		coud_source[i]  = center_at(cube, U_center)/2;
		cofb_source[i]  = center_at(cube, F_center)/2;
		corl_source[i]  = center_at(cube, R_center)/2;
	}

	if (read_ttables_file())
		return;

	fprintf(stderr, "Cannot load %s, generating it\n", "ttables"); 
  
	/* Initialize tables */
	for (m = 0; m < NTRANS; m++) {
		if (m == mirror) {
			memcpy(eparr, ep_mirror, 12 * sizeof(int));
			memcpy(cparr, cp_mirror,  8 * sizeof(int));
		} else {
			epcp = (CubeArray){ .ep = eparr, .cp = cparr };
			cube = apply_alg(trans_algs[m], (Cube){0});
			cube_to_arrays(cube, &epcp, pf_epcp);
		}

		for (ui = 0; ui < FACTORIAL12/FACTORIAL8; ui++) {
			c[0] = admissible_ep((Cube){ .epose = ui }, pf_e);
			c[1] = admissible_ep((Cube){ .eposs = ui }, pf_s);
			c[2] = admissible_ep((Cube){ .eposm = ui }, pf_m);

			cube = rotate_via_compose(m,c[epose_source[m]],pf_ep);
			epose_ttable[m][ui] = cube.epose;

			cube = rotate_via_compose(m,c[eposs_source[m]],pf_ep);
			eposs_ttable[m][ui] = cube.eposs;

			cube = rotate_via_compose(m,c[eposm_source[m]],pf_ep);
			eposm_ttable[m][ui] = cube.eposm;
		}
		for (ui = 0; ui < POW2TO11; ui++ ) {
			int_to_sum_zero_array(ui, 2, 12, eoarr);
			apply_permutation(eparr, eoarr, 12);
			eo_ttable[m][ui] = digit_array_to_int(eoarr, 11, 2);
		}
		for (ui = 0; ui < POW3TO7; ui++) {
			int_to_sum_zero_array(ui, 3, 8, coarr);
			apply_permutation(cparr, coarr, 8);
			co_ttable[m][ui] = digit_array_to_int(coarr, 7, 3);
			if (m == mirror)
				co_ttable[m][ui] =
					invert_digits(co_ttable[m][ui], 3, 7);
		}
		for (ui = 0; ui < FACTORIAL8; ui++) {
			cube = (Cube){ .cp = ui };
			cube = rotate_via_compose(m, cube, pf_cp);
			cp_ttable[m][ui] = cube.cp;
		}
		for (ui = 0; ui < FACTORIAL6; ui++) {
			cube = (Cube){ .cpos = ui };
			cube = rotate_via_compose(m, cube, pf_cpos);
			cpos_ttable[m][ui] = cube.cpos;
		}
		for (mi = 0; mi < NMOVES; mi++) {
			if (m == mirror) {
				b1 = (mi >= U && mi <= Bw3);
				b2 = (mi >= S && mi <= E3);
				b3 = (mi >= x && mi <= z3);
				if (b1 || b2 || b3)
					moves_ttable[m][mi] =
						inverse_move_aux[mi];
				else
					moves_ttable[m][mi] = mi;

				if ((mi-1)/3==(R-1)/3 || (mi-1)/3==(Rw-1)/3)
					moves_ttable[m][mi] += 3;
				if ((mi-1)/3==(L-1)/3 || (mi-1)/3==(L2-1)/3)
					moves_ttable[m][mi] -= 3;
			} else {
				aux = apply_trans(m, apply_move(mi,(Cube){0}));
				for (move = 0; move < NMOVES; move++) {
					cube = apply_move(
						inverse_move_aux[move], aux);
					if (is_solved(cube, false))
						moves_ttable[m][mi] = move;
				}
			}
		}
	}

	if (!write_ttables_file())
		fprintf(stderr, "Error writing ttables\n");
}

static void
init_trans_aux()
{
        ep_mirror[UF] = UF;
        ep_mirror[UL] = UR;
        ep_mirror[UB] = UB;
        ep_mirror[UR] = UL;
        ep_mirror[DF] = DF;
        ep_mirror[DL] = DR;
        ep_mirror[DB] = DB;
        ep_mirror[DR] = DL;
	ep_mirror[FR] = FL;
	ep_mirror[FL] = FR;
	ep_mirror[BR] = BL;
	ep_mirror[BL] = BR;

	cp_mirror[UFR] = UFL;
	cp_mirror[UFL] = UFR;
	cp_mirror[UBL] = UBR;
	cp_mirror[UBR] = UBL;
	cp_mirror[DFR] = DFL;
	cp_mirror[DFL] = DFR;
	cp_mirror[DBL] = DBR;
	cp_mirror[DBR] = DBL;

	cpos_mirror[U_center] = U_center;
	cpos_mirror[D_center] = D_center;
	cpos_mirror[R_center] = L_center;
	cpos_mirror[L_center] = R_center;
	cpos_mirror[F_center] = F_center;
	cpos_mirror[B_center] = B_center;

	/* Is there a more elegant way? */
	trans_algs[uf] = new_alg("");
	trans_algs[ur] = new_alg("y");
	trans_algs[ub] = new_alg("y2");
	trans_algs[ul] = new_alg("y3");

	trans_algs[df] = new_alg("z2");
	trans_algs[dr] = new_alg("y z2");
	trans_algs[db] = new_alg("x2");
	trans_algs[dl] = new_alg("y3 z2");

	trans_algs[rf] = new_alg("z3");
	trans_algs[rd] = new_alg("z3 y");
	trans_algs[rb] = new_alg("z3 y2");
	trans_algs[ru] = new_alg("z3 y3");

	trans_algs[lf] = new_alg("z");
	trans_algs[ld] = new_alg("z y3");
	trans_algs[lb] = new_alg("z y2");
	trans_algs[lu] = new_alg("z y");

	trans_algs[fu] = new_alg("x y2");
	trans_algs[fr] = new_alg("x y");
	trans_algs[fd] = new_alg("x");
	trans_algs[fl] = new_alg("x y3");

	trans_algs[bu] = new_alg("x3");
	trans_algs[br] = new_alg("x3 y");
	trans_algs[bd] = new_alg("x3 y2");
	trans_algs[bl] = new_alg("x3 y3");
}

/* Linearization functions implementation ************************************/

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
index_corners(Cube cube)
{
	return cube.coud * FACTORIAL8 + cube.cp;
}

static uint64_t
index_ep(Cube cube)
{
	uint64_t a, b, c;

	/* TODO: remove this check */
	if (epos_dependent_aux[cube.eposs/FACTORIAL4][cube.epose/FACTORIAL4] ==BINOM8ON4)
		fprintf(stderr, "Impossible\n");

	a = cube.eposs;
	b = (cube.epose % FACTORIAL4) +
	    epos_dependent_aux[cube.eposs/FACTORIAL4][cube.epose/FACTORIAL4] *
		FACTORIAL4;
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

/* Step checking functions implementation ************************************/

static int
check_nothing(Cube cube)
{
	return true;
}

static int
check_eofb_HTM(Cube cube)
{
	if (!pd_eofb_HTM.generated)
		generate_ptable(&pd_eofb_HTM);

	return PTABLEVAL(pd_eofb_HTM.ptable, cube.eofb);
}

static int
check_coud_HTM(Cube cube)
{
	if (!pd_coud_HTM.generated)
		generate_ptable(&pd_coud_HTM);

	return PTABLEVAL(pd_coud_HTM.ptable, cube.coud);
}

static int
check_coud_URF(Cube cube)
{
	/* TODO: I can improve this by checking first the orientation of
	 * the corner in DBL and use that as a reference */

	int ud = check_coud_HTM(cube);
	int rl = check_coud_HTM(apply_move(z, cube));
	int fb = check_coud_HTM(apply_move(x, cube));

	return MIN(ud, MIN(rl, fb));
}

static int
check_corners_HTM(Cube cube)
{
	if (!pd_corners_HTM.generated)
		generate_ptable(&pd_corners_HTM);

	return PTABLEVAL(pd_corners_HTM.ptable, index_corners(cube));
}

static int
check_corners_URF(Cube cube)
{
	/* TODO: I can improve this by checking first the corner in DBL
	 * and use that as a reference */

	int ret = 15;
	Cube c;
	Trans i;

	for (i = 0; i < NROTATIONS; i++) {
		c = apply_alg(trans_algs[i], cube);
		ret = MIN(ret, check_corners_HTM(c));
	}

	return ret;
}

static int
check_edges_HTM(Cube cube)
{
	int ret = 0;

	if (!pd_ep_HTM.generated)
		generate_ptable(&pd_ep_HTM);

	ret = MAX(ret, PTABLEVAL(pd_ep_HTM.ptable, index_ep(cube)));
	ret = MAX(ret, check_eofb_HTM(cube));
	ret = MAX(ret, check_eofb_HTM(apply_trans(ur, cube)));
	ret = MAX(ret, check_eofb_HTM(apply_trans(fd, cube)));

	return ret;
}

static int
check_drud_HTM(Cube cube)
{
	if (!pd_drud_HTM.generated)
		generate_ptable(&pd_drud_HTM);

	return PTABLEVAL(pd_drud_HTM.ptable, index_drud(cube));
}

static int
check_optimal_HTM(Cube cube)
{
	int dr1, dr2, dr3, drmax, cor; /*ep;*/

	if (!pd_drud_HTM.generated)
		generate_ptable(&pd_drud_HTM);
	if (!pd_corners_HTM.generated)
		generate_ptable(&pd_corners_HTM);
	/*
	 *if (!pd_ep_HTM.generated)
 	 *	generate_ptable(&pd_ep_HTM);
	 */

	dr1 = PTABLEVAL(pd_drud_HTM.ptable, index_drud(cube));
	dr2 = PTABLEVAL(pd_drud_HTM.ptable, index_drud(apply_trans(rf, cube)));
	dr3 = PTABLEVAL(pd_drud_HTM.ptable, index_drud(apply_trans(fd, cube)));

	drmax = MAX(dr1, MAX(dr2, dr3));
	if (dr1 == dr2 && dr2 == dr3 && dr1 != 0)
		drmax++;

	cor = PTABLEVAL(pd_corners_HTM.ptable, index_corners(cube));
	/* ep = PTABLEVAL(pd_ep_HTM.ptable, index_ep(cube)); */

	/*return MAX(drmax, MAX(ep, cor));*/
	if (drmax == 0 && cor == 0)
		return is_solved(cube, false) ? 0 : 1;
	return MAX(drmax, cor); 
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


/* Public functions implementation *******************************************/

Cube
apply_alg(Alg *alg, Cube cube)
{
	return apply_alg_generic(alg, cube, pf_all, true);
}

Cube
apply_move(Move m, Cube cube)
{
	Cube moved = {0};

	moved.epose = epose_mtable[m][cube.epose];
	moved.eposs = eposs_mtable[m][cube.eposs];
	moved.eposm = eposm_mtable[m][cube.eposm];
	moved.eofb  = eofb_mtable[m][cube.eofb];
	moved.eorl  = eorl_mtable[m][cube.eorl];
	moved.eoud  = eoud_mtable[m][cube.eoud];
	moved.coud  = coud_mtable[m][cube.coud];
	moved.cofb  = cofb_mtable[m][cube.cofb];
	moved.corl  = corl_mtable[m][cube.corl];
	moved.cp    = cp_mtable[m][cube.cp];
	moved.cpos  = cpos_mtable[m][cube.cpos];

	return moved;
}

Cube
apply_trans(Trans t, Cube cube)
{
	Cube transformed = {0};
	uint16_t aux_epos[3] = { cube.epose, cube.eposs, cube.eposm };
	uint16_t aux_eo[3]   = { cube.eoud,  cube.eorl,  cube.eofb  };
	uint16_t aux_co[3]   = { cube.coud,  cube.corl,  cube.cofb  };

	transformed.epose = epose_ttable[t][aux_epos[epose_source[t]]];
	transformed.eposs = eposs_ttable[t][aux_epos[eposs_source[t]]];
	transformed.eposm = eposm_ttable[t][aux_epos[eposm_source[t]]];
	transformed.eofb  = eo_ttable[t][aux_eo[eofb_source[t]]];
	transformed.eorl  = eo_ttable[t][aux_eo[eorl_source[t]]];
	transformed.eoud  = eo_ttable[t][aux_eo[eoud_source[t]]];
	transformed.coud  = co_ttable[t][aux_co[coud_source[t]]];
	transformed.corl  = co_ttable[t][aux_co[corl_source[t]]];
	transformed.cofb  = co_ttable[t][aux_co[cofb_source[t]]];
	transformed.cp    = cp_ttable[t][cube.cp];
	transformed.cpos  = cpos_ttable[t][cube.cpos];

	return transformed;
}

/* TODO: this has to be changed using pre-computations */
bool
block_solved(Cube cube, Block block)
{
	CubeArray *arr = new_cubearray(cube, pf_all);
	int i;
	bool ret = true;

	for (i = 0; i < 12; i++)
		ret = ret && !(block.edge[i] && (arr->ep[i] != i || arr->eofb[i]));
	for (i = 0; i < 8; i++)
		ret = ret && !(block.corner[i] && (arr->cp[i] != i || arr->coud[i]));
	for (i = 0; i < 6; i++)
		ret = ret && !(block.center[i] && arr->cpos[i] != i);

	free_cubearray(arr, pf_all);

	return ret;
}

Center
center_at(Cube cube, Center c)
{
	int ret;
	CubeArray *arr = new_cubearray(cube, pf_cpos);
	
	ret = arr->cpos[c];
	free_cubearray(arr, pf_cpos);

	return ret;
}

Cube
compose(Cube c2, Cube c1)
{
	return compose_filtered(c2, c1, pf_all);
}

Corner
corner_at(Cube cube, Corner c)
{
	int ret;
	CubeArray *arr = new_cubearray(cube, pf_cp);
	
	ret = arr->cp[c];
	free_cubearray(arr, pf_cp);

	return ret;
}

Edge
edge_at(Cube cube, Edge e)
{
	int ret;
	CubeArray *arr = new_cubearray(cube, pf_ep);
	
	ret = arr->ep[e];
	free_cubearray(arr, pf_ep);

	return ret;
}

bool
equal(Cube c1, Cube c2)
{
	return c1.eofb  == c2.eofb  &&
	       c1.epose == c2.epose &&
	       c1.eposs == c2.eposs &&
	       c1.eposm == c2.eposm &&
	       c1.coud  == c2.coud  &&
	       c1.cp    == c2.cp    &&
	       c1.cpos  == c2.cpos;
}

Cube
inverse_cube(Cube cube)
{
	CubeArray *arr = new_cubearray(cube, pf_all);
	CubeArray *inv = new_cubearray((Cube){0}, pf_all);
	Cube ret;
	int i;

	for (i = 0; i < 12; i++) {
		inv->ep[arr->ep[i]]   = i;
		inv->eofb[arr->ep[i]] = arr->eofb[i];
		inv->eorl[arr->ep[i]] = arr->eorl[i];
		inv->eoud[arr->ep[i]] = arr->eoud[i];
	}

	for (i = 0; i < 8; i++) {
		inv->cp[arr->cp[i]]   = i;
		inv->coud[arr->cp[i]] = (3 - arr->coud[i]) % 3;
		inv->corl[arr->cp[i]] = (3 - arr->corl[i]) % 3;
		inv->cofb[arr->cp[i]] = (3 - arr->cofb[i]) % 3;
	}

	for (int i = 0; i < 6; i++)
		inv->cpos[arr->cpos[i]] = i;

	ret = arrays_to_cube(inv, pf_all);
	free_cubearray(arr, pf_all);
	free_cubearray(inv, pf_all);

	return ret;
}

Move
inverse_move(Move m)
{
	return inverse_move_aux[m];
}

Trans
inverse_trans(Trans t)
{
	return inverse_trans_aux[t];
}

bool
is_admissible(Cube cube)
{
	/* TODO: this should check consistency of different orientations */
	/* TODO: check that centers are opposite and admissible */

	CubeArray *a = new_cubearray(cube, pf_all);
	int parity;
	bool perm;

	perm   = is_perm(a->ep,  12)  &&
	         is_perm(a->cp,   8)  &&
	         is_perm(a->cpos, 6);
	parity = perm_sign(a->ep,  12) +
	         perm_sign(a->cp,   8) +
	         perm_sign(a->cpos, 6);

	return perm && parity % 2 == 0;
}

bool
is_solved(Cube cube, bool reorient)
{
	Trans i;

	if (reorient)
		for (i = 0; i < NROTATIONS; i++)
			if (is_solved(apply_alg(trans_algs[i],cube), false))
				return true;

	return equal(cube, (Cube){0});
}

int
piece_orientation(Cube cube, int piece, char *orientation)
{
	int arr[12], n, b;
	uint16_t x;

	if (!strcmp(orientation, "eofb")) {
		x = cube.eofb;
		n = 12;
		b = 2;
	} else if (!strcmp(orientation, "eorl")) {
		x = cube.eorl;
		n = 12;
		b = 2;
	} else if (!strcmp(orientation, "eoud")) {
		x = cube.eoud;
		n = 12;
		b = 2;
	} else if (!strcmp(orientation, "coud")) {
		x = cube.coud;
		n = 8;
		b = 3;
	} else if (!strcmp(orientation, "corl")) {
		x = cube.corl;
		n = 8;
		b = 3;
	} else if (!strcmp(orientation, "cofb")) {
		x = cube.cofb;
		n = 8;
		b = 3;
	} else {
		return -1;
	}

	int_to_sum_zero_array(x, b, n, arr);
	if (piece < n)
		return arr[piece];

	return -1;
}

void
print_cube(Cube cube)
{
	CubeArray *arr = new_cubearray(cube, pf_all);

	for (int i = 0; i < 12; i++)
		printf(" %s ", edge_string[arr->ep[i]]);
	printf("\n");

	for (int i = 0; i < 12; i++)
		printf("  %c ", arr->eofb[i] + '0');
	printf("\n");

	for (int i = 0; i < 8; i++)
		printf("%s ", corner_string[arr->cp[i]]);
	printf("\n");

	for (int i = 0; i < 8; i++)
		printf("  %c ", arr->coud[i] + '0');
	printf("\n");

	for (int i = 0; i < 6; i++)
		printf("  %s ", center_string[arr->cpos[i]]);
	printf("\n");

	free_cubearray(arr, pf_all);
}

Cube
random_cube()
{
	CubeArray *arr = new_cubearray((Cube){0}, pf_4val);
	Cube ret;
	int ep, cp, eo, co;

	ep = rand() % FACTORIAL12;
	cp = rand() % FACTORIAL8;
	eo = rand() % POW2TO11;
	co = rand() % POW3TO7;

	index_to_perm(ep, 12, arr->ep);
	index_to_perm(cp,  8, arr->cp);
	int_to_sum_zero_array(eo, 2, 12, arr->eofb);
	int_to_sum_zero_array(co, 3,  8, arr->coud);

	if (perm_sign(arr->ep, 12) != perm_sign(arr->cp, 8))
		swap(&(arr->ep[0]), &(arr->ep[1]));

	ret = arrays_to_cube(arr, pf_4val);
	free_cubearray(arr, pf_4val);

	return ret;
}

AlgList *
solve(Cube cube, Step step, SolveOptions *opts)
{
	AlgListNode *node;
	AlgList *sols = new_alglist();
	Cube c = apply_trans(step.pre_trans, cube);
	DfsData dd = {
		.m           = 0,
		.niss        = false,
		.last1       = NULLMOVE,
		.last2       = NULLMOVE,
		.sols        = sols,
		.current_alg = new_alg("")
	};

	if (step.ready != NULL && !step.ready(c))
		return sols;

	moveset_to_list(step.moveset, step.check, dd.sorted_moves);

	for (dd.d = MAX(opts->min_moves, step.check(c));
	     dd.d <= opts->max_moves && !(sols->len && opts->optimal_only);
	     dd.d++) {
		solve_dfs(c, step, opts, &dd);
		if (opts->feedback)
			fprintf(stderr,
				"Depth %d completed, found %d solutions\n",
				dd.d, sols->len);
	}

	for (node = sols->first; node != NULL; node = node->next)
		transform_alg(inverse_trans_aux[step.pre_trans], node->alg);

	return sols;
}

Alg *
inverse_alg(Alg *alg)
{
	Alg *ret = new_alg("");
	AlgNode *i;

	for (i = alg->last; i != NULL; i = i->prev)
		append_move(ret, i->m, i->inverse);

	return ret;
}

Alg *
new_alg(char *str)
{
	Alg *alg = malloc(sizeof(Alg));
	int i;
	bool niss = false;
	Move j, m;

	alg->first = NULL;
	alg->last = NULL;
	alg->len = 0;

	for (i = 0; str[i]; i++) {
		if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n')
			continue;

		if (str[i] == '(' && niss) {
			fprintf(stderr, "Error reading moves: nested ( )\n");
			return alg;
		}

		if (str[i] == ')' && !niss) {
			fprintf(stderr, "Error reading moves: unmatched )\n");
			return alg;
		}

		if (str[i] == '(' || str[i] == ')') {
			niss = !niss;
			continue;
		}

		for (j = 0; j < NMOVES; j++) {
			if (str[i] == move_string[j][0]) {
				m = j;
				if (m <= B && str[i+1]=='w') {
					m += Uw - U;
					i++;
				}
				if (str[i+1]=='2') {
					m += 1;
					i++;
				} else if (str[i+1]=='\'' || str[i+1]=='3') {
					m += 2;
					i++;
				}
				append_move(alg, m, niss);
				break;
			}
		}
	}

	return alg;
}

Alg *
on_inverse(Alg *alg)
{
	Alg *ret = new_alg("");
	AlgNode *i;

	for (i = alg->first; i != NULL; i = i->next)
		append_move(ret, i->m, !i->inverse);

	return ret;
}

void
print_alg(Alg *alg, bool l)
{
	char fill[4];
	AlgNode *i;
	bool niss = false;

	for (i = alg->first; i != NULL; i = i->next) {
		if (!niss && i->inverse)
			strcpy(fill, i == alg->first ? "(" : " (");
		if (niss && !i->inverse)
			strcpy(fill, ") ");
		if (niss == i->inverse)
			strcpy(fill, i == alg->first ? "" : " ");

		printf("%s%s", fill, move_string[i->m]);
		niss = i->inverse;
	}

	if (niss)
		printf(")");
	if (l)
		printf(" (%d)", alg->len);

	printf("\n");
}

void
print_ptable(PruneData *pd)
{
	uint64_t i, a[16];
	
	memset(a, 0, 16 * sizeof(uint64_t));

	if (!pd->generated)
		generate_ptable(pd);

	for (i = 0; i < pd->size; i++)
		a[PTABLEVAL(pd->ptable, i)]++;
		
	fprintf(stderr, "Values for table %s\n", pd->filename);
	for (i = 0; i < 16; i++)
		printf("%2lu\t%10lu\n", i, a[i]);
}

void
print_alglist(AlgList *al, bool l)
{
	AlgListNode *i;

	for (i = al->first; i != NULL; i = i->next)
		print_alg(i->alg, l);
}

void
remove_last_move(Alg *alg)
{
	AlgNode *newlast = alg->last->prev;
	free(alg->last);
	if (newlast != NULL)
		newlast->next = NULL;
	alg->last = newlast;
	alg->len--;
}

void
transform_alg(Trans t, Alg *alg)
{
	AlgNode *i;
	
	for (i = alg->first; i != NULL; i = i->next)
		i->m = moves_ttable[t][i->m];
}


void
init()
{
	/* Order is important! */
	init_environment();
	init_strings();
	init_moves_aux();
	init_moves();
	init_auxtables();
	init_trans_aux();
	init_trans();
	init_steps();
}


