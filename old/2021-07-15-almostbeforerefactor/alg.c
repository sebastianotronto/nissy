#include "alg.h"

/* Local functions **********************************************************/

static Cube        admissible_ep(Cube cube, PieceFilter f);
static Cube        admissible_eos_from_eofbepos(Cube cube);
static bool        allowed_next(Move move, DfsData *dd);
static void        append_alg(AlgList *l, Alg *alg);
static void        append_move(Alg *alg, Move m, bool inverse);
static Cube        apply_alg_generic(Alg *alg, Cube c, PieceFilter f, bool a);
static void        apply_permutation(int *perm, int *set, int n);
static Cube        apply_move_cubearray(Move m, Cube cube, PieceFilter f);
static int         array_ep_to_epos(int *ep, int *eps_solved);
static Cube        arrays_to_cube(CubeArray *arr, PieceFilter f);
static int         binomial(int n, int k);
static Cube        compose_filtered(Cube c2, Cube c1, PieceFilter f);
static void        cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f);
static void        dfs(Cube c, Step s, SolveOptions *opts, DfsData *dd);
static void        dfs_branch(Cube c, Step s, SolveOptions *opts, DfsData *dd);
static bool        dfs_check_solved(SolveOptions *opts, DfsData *dd);
static void        dfs_niss(Cube c, Step s, SolveOptions *opts, DfsData *dd);
static bool        dfs_stop(Cube c, Step s, SolveOptions *opts, DfsData *dd);
static int         digit_array_to_int(int *a, int n, int b);
static int         edge_slice(Edge e); /* E=0, S=1, M=2 */
static int         epos_dependent_pos(int pos1, int pos2);
static int         epos_from_arrays(int *epos, int *ep);
static void        epos_to_partial_ep(int epos, int *ep, int *ss);
static int         factorial(int n);
static void        free_alglistnode(AlgListNode *aln);
static void        free_cubearray(CubeArray *arr, PieceFilter f);
static void        genptable_bfs(PruneData *pd, int d, Move *ms);
static void        genptable_branch(PruneData *pd, uint64_t i, int d, Move *m);
static void        gensym(SymData *sd);
static void        index_to_perm(int p, int n, int *r);
static void        index_to_subset(int s, int n, int k, int *r);
static void        init_auxtables();
static void        init_cphtr_cosets();
static void        init_cphtr_left_cosets_bfs(int i, int c);
static void        init_cphtr_right_cosets_color(int i, int c);
static void        init_environment();
static void        init_moves();
static void        init_moves_aux();
static void        init_strings();
static void        init_symdata();
static void        init_trans();
static void        init_trans_aux();
static void        int_to_digit_array(int a, int b, int n, int *r);
static void        int_to_sum_zero_array(int x, int b, int n, int *a);
static int         invert_digits(int a, int b, int n);
static bool        is_perm(int *a, int n);
static bool        is_subset(int *a, int n, int k);
static Cube        move_via_arrays(CubeArray *arr, Cube c, PieceFilter pf);
static void        movelist_to_position(Move *movelist, int *position);
static void        moveset_to_list(Moveset ms, Estimator f, Move *r);
static AlgList *   new_alglist();
static CubeArray * new_cubearray(Cube cube, PieceFilter f);
static int         perm_sign(int *a, int n);
static int         perm_to_index(int *a, int n);
static int         powint(int a, int b);
static void        ptable_update(PruneData *pd, Cube cube, int m);
static int         ptableval_index(PruneData *pd, uint64_t ind);
static void        realloc_alg(Alg *alg, int n);
static bool        read_mtables_file();
static bool        read_ptable_file(PruneData *pd);
static bool        read_symdata_file(SymData *sd);
static bool        read_ttables_file();
static Cube        rotate_via_compose(Trans r, Cube c, PieceFilter f);
static int         subset_to_index(int *a, int n, int k);
static void        sum_arrays_mod(int *src, int *dst, int n, int m);
static void        swap(int *a, int *b);
static bool        write_mtables_file();
static bool        write_ptable_file(PruneData *pd);
static bool        write_symdata_file(SymData *sd);
static bool        write_ttables_file();


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

static Cube             admissible_ee_aux[POW2TO11*BINOM12ON4];
static bool             commute[NMOVES][NMOVES];
static bool             possible_next[NMOVES][NMOVES][NMOVES];
static Move             inverse_move_aux[NMOVES];
static Trans            inverse_trans_aux[NTRANS];
static int              epos_dependent_aux[BINOM12ON4][BINOM12ON4];
static int              cphtr_left_cosets[FACTORIAL8];
static int              cphtr_right_cosets[FACTORIAL8];
static int              cphtr_right_rep[BINOM8ON4*6];
static Center           what_center_at_aux[FACTORIAL6][6];
static Corner           what_corner_at_aux[FACTORIAL8][8];
static int              what_orientation_last_corner_aux[POW3TO7];
static int              what_orientation_last_edge_aux[POW2TO11];
static Center           where_is_center_aux[FACTORIAL6][6];
static Corner           where_is_corner_aux[FACTORIAL8][8];
static Edge             where_is_edge_aux[3][FACTORIAL12/FACTORIAL8][12];

static int              epose_ttable[NTRANS][FACTORIAL12/FACTORIAL8];
static int              eposs_ttable[NTRANS][FACTORIAL12/FACTORIAL8];
static int              eposm_ttable[NTRANS][FACTORIAL12/FACTORIAL8];
static int              eo_ttable[NTRANS][POW2TO11];
static int              cp_ttable[NTRANS][FACTORIAL8];
static int              co_ttable[NTRANS][POW3TO7];
static int              cpos_ttable[NTRANS][FACTORIAL6];
static Move             moves_ttable[NTRANS][NMOVES];

static int              epose_mtable[NMOVES][FACTORIAL12/FACTORIAL8];
static int              eposs_mtable[NMOVES][FACTORIAL12/FACTORIAL8];
static int              eposm_mtable[NMOVES][FACTORIAL12/FACTORIAL8];
static int              eofb_mtable[NMOVES][POW2TO11];
static int              eorl_mtable[NMOVES][POW2TO11];
static int              eoud_mtable[NMOVES][POW2TO11];
static int              cp_mtable[NMOVES][FACTORIAL8];
static int              coud_mtable[NMOVES][POW3TO7];
static int              cofb_mtable[NMOVES][POW3TO7];
static int              corl_mtable[NMOVES][POW3TO7];
static int              cpos_mtable[NMOVES][FACTORIAL6];

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
static Alg *            rotation_algs[NROTATIONS];


/* Symmetry data for some coordinates ****************************************/

Trans
trans_group_trivial[1] = { uf };

Trans
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
sd_eofbepos_16 = {
	.filename  = "sd_eofbepos_16",
	.coord     = &coord_eofbepos,
	.sym_coord = &coord_eofbepos_sym16,
	.ntrans    = 16,
	.trans     = trans_group_udfix
};

static int        n_all_symdata         = 2;
static SymData *  all_sd[2]             = { &sd_coud_16, &sd_eofbepos_16 };

/* Coordinates and their implementation **************************************/

static uint64_t    index_eofb(Cube cube);
static uint64_t    index_eofbepos(Cube cube);
static uint64_t    index_coud(Cube cube);
static uint64_t    index_corners(Cube cube);
static uint64_t    index_cornershtr(Cube cube);
static uint64_t    index_drud(Cube cube);
static uint64_t    index_coud_sym16(Cube cube);
static uint64_t    index_eofbepos_sym16(Cube cube);
static uint64_t    index_drud_sym16(Cube cube);
static uint64_t    index_khuge(Cube cube);

static Cube        antindex_eofb(uint64_t ind);
static Cube        antindex_eofbepos(uint64_t ind);
static Cube        antindex_coud(uint64_t ind);
static Cube        antindex_corners(uint64_t ind);
static Cube        antindex_cornershtr(uint64_t ind);
static Cube        antindex_drud(uint64_t ind);
static Cube        antindex_coud_sym16(uint64_t ind);
static Cube        antindex_eofbepos_sym16(uint64_t ind);
static Cube        antindex_drud_sym16(uint64_t ind);
static Cube        antindex_khuge(uint64_t ind);

Coordinate
coord_eofb = {
	.index = index_eofb,
	.cube  = antindex_eofb,
	.check = check_eofb,
	.max   = POW2TO11
};

Coordinate
coord_eofbepos = {
	.index = index_eofbepos,
	.cube  = antindex_eofbepos,
	.check = check_eofbepos,
	.max   = POW2TO11 * BINOM12ON4
};

Coordinate
coord_coud = {
	.index = index_coud,
	.cube  = antindex_coud,
	.check = check_coud,
	.max   = POW3TO7
};

Coordinate
coord_corners = {
	.index = index_corners,
	.cube  = antindex_corners,
	.check = check_corners,
	.max   = POW3TO7 * FACTORIAL8
};

Coordinate
coord_cornershtr = {
	.index = index_cornershtr,
	.cube  = antindex_cornershtr,
	.check = check_cornershtr,
	.max   = POW3TO7 * BINOM8ON4 * 6
};

Coordinate
coord_drud = {
	.index = index_drud,
	.cube  = antindex_drud,
	.check = check_drud,
	.max   = POW2TO11 * POW3TO7 * BINOM12ON4
};

Coordinate
coord_eofbepos_sym16 = {
	.index = index_eofbepos_sym16,
	.cube  = antindex_eofbepos_sym16,
	.check = check_eofbepos,
};

Coordinate
coord_coud_sym16 = {
	.index = index_coud_sym16,
	.cube  = antindex_coud_sym16,
	.check = check_coud,
};

Coordinate
coord_drud_sym16 = {
	.index = index_drud_sym16,
	.cube  = antindex_drud_sym16,
	.check = check_drud,
	.max   = POW3TO7 * 64430
};

Coordinate
coord_khuge = {
	.index = index_khuge,
	.cube  = antindex_khuge,
	.check = check_khuge,
	.max   = POW3TO7 * FACTORIAL4 * 64430
};


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
index_coud_sym16(Cube cube)
{
	return sd_coud_16.class[index_coud(cube)];
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


/* TODO: rename */
static Cube
antindex_eofb(uint64_t ind)
{
	return (Cube){ .eofb = ind, .eorl = ind, .eoud = ind };
}

static Cube
antindex_eofbepos(uint64_t ind)
{
	return admissible_ee_aux[ind];
}

/* TODO: rename */
static Cube
antindex_coud(uint64_t ind)
{
	return (Cube){ .coud = ind, .corl = ind, .cofb = ind };
}

/* TODO: admissible co for other orientations */
static Cube
antindex_corners(uint64_t ind)
{
	Cube c = {0};

	c.coud = ind / FACTORIAL8;
	c.cp   = ind % FACTORIAL8;

	return c;
}

/* TODO: admissible co for other orientations */
static Cube
antindex_cornershtr(uint64_t ind)
{
	Cube c = anti_cphtr(ind % (BINOM8ON4 * 6));

	c.coud = ind / (BINOM8ON4 * 6);

	return c;
}

/* TODO: admissible eos and cos */
/* DONE: temporary fix, make it better */
static Cube
antindex_drud(uint64_t ind)
{
	uint64_t epos, eofb;
	Cube c;

	eofb = ind % POW2TO11;
	epos = ind / (POW2TO11 * POW3TO7);
	c = admissible_ee_aux[eofb + POW2TO11 * epos];

	c.coud  = (ind / POW2TO11) % POW3TO7;
	c.corl  = c.coud;
	c.cofb  = c.coud;

	return c;
}

static Cube
antindex_coud_sym16(uint64_t ind)
{
	return sd_coud_16.rep[ind];
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
antindex_khuge(uint64_t ind)
{
	Cube c;

	c = sd_eofbepos_16.rep[ind/(FACTORIAL4*POW3TO7)];
	c.epose = ((c.epose / 24) * 24) + ((ind/POW3TO7) % 24);
	c.coud = ind % POW3TO7;

	return c;
}


/* Checkers ******************************************************************/

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


/* Local functions implementation ********************************************/

/* TODO: this should be an anti index (maybe?) */
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

static bool
allowed_next(Move move, DfsData *dd)
{
	if (!possible_next[dd->last2][dd->last1][move])
		return false;

	if (commute[dd->last1][move])
		return dd->move_position[dd->last1] < dd->move_position[move];

	return true;
}

static void
append_alg(AlgList *l, Alg *alg)
{
	AlgListNode *node = malloc(sizeof(AlgListNode));
	int i;

	node->alg = new_alg("");
	for (i = 0; i < alg->len; i++)
		append_move(node->alg, alg->move[i], alg->inv[i]);
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
	if (alg->len == alg->allocated)
		realloc_alg(alg, 2*alg->len);

	alg->move[alg->len] = m;
	alg->inv [alg->len] = inverse;
	alg->len++;
}

static Cube
apply_alg_generic(Alg *alg, Cube c, PieceFilter f, bool a)
{
	Cube ret = {0};
	int i;

	for (i = 0; i < alg->len; i++)
		if (alg->inv[i])
			ret = a ? apply_move(alg->move[i], ret) :
			          apply_move_cubearray(alg->move[i], ret, f);

	ret = compose_filtered(c, inverse_cube(ret), f);

	for (i = 0; i < alg->len; i++)
		if (!alg->inv[i])
			ret = a ? apply_move(alg->move[i], ret) :
			          apply_move_cubearray(alg->move[i], ret, f);

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

static int
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

/*
static int
cphtr_cp(int cp)
{
	int i, a[8];

	index_to_perm(cp, 8, a);

	for (i = 0; i < 8; i++)
		if (a[i] == UFR || a[i] == UBL || a[i] == DFL || a[i] == DBR)
			a[i] = 0;
		else
			a[i] = 1;

	swap(&a[1], &a[5]);
	swap(&a[3], &a[7]);

	return subset_to_index(a, 8, 4);
}
*/

static void
dfs(Cube c, Step s, SolveOptions *opts, DfsData *dd)
{
	if (dfs_stop(c, s, opts, dd))
		return;

	if (dfs_check_solved(opts, dd))
		return;

	dfs_branch(c, s, opts, dd);

	if (opts->can_niss && !dd->niss)
		dfs_niss(c, s, opts, dd);
}

static void
dfs_branch(Cube c, Step s, SolveOptions *opts, DfsData *dd)
{
	Move m, l1 = dd->last1, l2 = dd->last2, *moves = dd->sorted_moves;

	int i, maxnsol = opts->max_solutions;

	for (i = 0; moves[i] != NULLMOVE && dd->sols->len < maxnsol; i++) {
		m = moves[i];
		if (allowed_next(m, dd)) {
			dd->last2 = dd->last1;
			dd->last1 = m;
			append_move(dd->current_alg, m, dd->niss);

			dfs(apply_move(m, c), s, opts, dd);

			dd->current_alg->len--;
			dd->last2 = l2;
			dd->last1 = l1;
		}
	}
}

static bool
dfs_check_solved(SolveOptions *opts, DfsData *dd)
{
	if (dd->lb != 0)
		return false;

	if (dd->current_alg->len == dd->d) {
		append_alg(dd->sols, dd->current_alg);

		if (opts->feedback)
			print_alg(dd->current_alg, false);
	}

	return true;
}

static void
dfs_niss(Cube c, Step s, SolveOptions *opts, DfsData *dd)
{
	Move l1 = dd->last1, l2 = dd->last2;
	CubeTarget ct;

	ct.cube = apply_move(inverse_move(l1), (Cube){0});
	ct.target = 1;

	if (dd->current_alg->len == 0 || s.estimate(ct)) {
		dd->niss  = true;
		dd->last1 = NULLMOVE;
		dd->last2 = NULLMOVE;

		dfs(inverse_cube(c), s, opts, dd);

		dd->last1 = l1;
		dd->last2 = l2;
		dd->niss  = false;
	}
}

static bool
dfs_stop(Cube c, Step s, SolveOptions *opts, DfsData *dd)
{
	CubeTarget ct = {
		.cube   = c,
		.target = dd->d - dd->current_alg->len
	};

	if (dd->sols->len >= opts->max_solutions)
		return true;

	dd->lb = s.estimate(ct);
	if (opts->can_niss && !dd->niss)
		dd->lb = MIN(1, dd->lb);

	if (dd->current_alg->len + dd->lb > dd->d)
		return true;

	return false;
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
epos_dependent_pos(int poss, int pose)
{
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

static int
epos_from_arrays(int *epos, int *ep)
{
	return FACTORIAL4 * subset_to_index(epos,12,4) + perm_to_index(ep,4);
}

static void
epos_to_partial_ep(int epos, int *ep, int *ss)
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
	free(alg->move);
	free(alg->inv);
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
	Cube cc, c;


	for (i = 0; i < pd->ntrans; i++) {
		c = apply_trans(pd->trans[i], pd->coord->cube(ind));
		for (j = 0; ms[j] != NULLMOVE; j++) {
			cc = apply_move(ms[j], c);
			if (ptableval(pd, cc) > d+1)
				ptable_update(pd, cc, d+1);
		}
	}
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
movelist_to_position(Move *movelist, int *position)
{
	Move m;

	for (m = 0; m < NMOVES && movelist[m] != NULLMOVE; m++)
		position[movelist[m]] = m;
}

static void
moveset_to_list(Moveset ms, Estimator f, Move *r)
{
	CubeTarget ct = { .target = 1 };
	int b[NMOVES];
	int na = 0, nb = 0;
	Move i;

	if (ms == NULL) {
		fprintf(stderr, "Error: no moveset given\n");
		return;
	}

	for (i = U; i < NMOVES; i++) {
		if (ms(i)) {
			ct.cube = apply_move(i, (Cube){0});
			if (f != NULL && f(ct))
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
		return 0;
	if (b == 0)
		return 1;

	if (b % 2)
		return a * powint(a, b-1);
	else
		return powint(a*a, b/2);
}

static void
ptable_update(PruneData *pd, Cube cube, int n)
{
	uint64_t ind = pd->coord->index(cube);
	uint8_t oldval2 = pd->ptable[ind/2];
	int other = (ind % 2) ? oldval2 % 16 : oldval2 / 16;

	pd->ptable[ind/2] = (ind % 2) ? 16*n + other : 16*other + n;
	pd->n++;
}

static int
ptableval_index(PruneData *pd, uint64_t ind)
{
	return (ind % 2) ? pd->ptable[ind/2] / 16 : pd->ptable[ind/2] % 16;
}

static void
realloc_alg(Alg *alg, int n)
{
	if (alg == NULL) {
		fprintf(stderr, "Error: trying to reallocate NULL alg.\n");
		return;
	}

	if (n < alg->len) {
		fprintf(stderr, "Error: alg too long for reallocation ");
		fprintf(stderr, "(%d vs %d)\n", alg->len, n);
		return;
	}

	if (n > 1000000) {
		fprintf(stderr, "Warning: very long alg,");
		fprintf(stderr, "something might go wrong.\n");
	}

	alg->move = realloc(alg->move, n * sizeof(int));
	alg->inv  = realloc(alg->inv,  n * sizeof(int));
	alg->allocated = n;
}

static bool
read_mtables_file()
{
	FILE *f;
	char fname[strlen(tabledir)+20];
	int m, b = sizeof(int);
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

	r = fread(pd->ptable, sizeof(uint8_t), ptablesize(pd), f);
	fclose(f);

	return r == ptablesize(pd);
}

static bool
read_symdata_file(SymData *sd)
{
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
read_ttables_file()
{
	FILE *f;
	char fname[strlen(tabledir)+20];
	int b = sizeof(int);
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

	Alg *inv = inverse_alg(rotation_algs[r % NROTATIONS]);
	Cube ret = {0};

	if (r >= NROTATIONS)
		ret = move_via_arrays(&ma, ret, f);
	ret = apply_alg_generic(inv, ret, f, true);

	ret = compose_filtered(c, ret, f);

	ret = apply_alg_generic(rotation_algs[r % NROTATIONS], ret, f, true);
	if (r >= NROTATIONS)
		ret = move_via_arrays(&ma, ret, f);

	free_alg(inv);
	return ret;
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
	int m, b = sizeof(int);
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

	written = fwrite(pd->ptable, sizeof(uint8_t), ptablesize(pd), f);
	fclose(f);

	return written == ptablesize(pd);
}

static bool
write_symdata_file(SymData *sd)
{
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

static bool
write_ttables_file()
{
	FILE *f;
	char fname[strlen(tabledir)+20];
	bool r = true;
	int b = sizeof(int);
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
	CubeArray *arr;
	uint64_t ui, uj;
	int i, j, k, auxarr[12];
	bool cij, p1, p2;

	for (ui = 0; ui < POW2TO11*BINOM12ON4; ui++) {
		k = (ui / POW2TO11) * 24;
		c1 = admissible_ep((Cube){ .epose = k }, pf_e);
		c1.eofb  = ui % POW2TO11;
		c1 = admissible_eos_from_eofbepos(c1);
		admissible_ee_aux[ui] = c1;
	}

	for (ui = 0; ui < FACTORIAL6; ui++) {
		arr = new_cubearray((Cube){.cpos = ui}, pf_cpos);
		for (i = 0; i < 6; i++) {
			what_center_at_aux[ui][i] = arr->cpos[i];
			where_is_center_aux[ui][arr->cpos[i]] = i;
		}
		free_cubearray(arr, pf_cpos);
	}

	for (ui = 0; ui < FACTORIAL8; ui++) {
		arr = new_cubearray((Cube){.cp = ui}, pf_cp);
		for (i = 0; i < 8; i++) {
			what_corner_at_aux[ui][i] = arr->cp[i];
			where_is_corner_aux[ui][arr->cp[i]] = i;
		}
		free_cubearray(arr, pf_cp);
	}

	for (ui = 0; ui < FACTORIAL12/FACTORIAL8; ui++) {
		arr = new_cubearray((Cube){.epose = ui}, pf_e);
		for (i = 0; i < 12; i++)
			if (edge_slice(arr->ep[i]) == 0)
				where_is_edge_aux[0][ui][arr->ep[i]] = i;
		free_cubearray(arr, pf_e);

		arr = new_cubearray((Cube){.eposs = ui}, pf_s);
		for (i = 0; i < 12; i++)
			if (edge_slice(arr->ep[i]) == 1)
				where_is_edge_aux[1][ui][arr->ep[i]] = i;
		free_cubearray(arr, pf_s);

		arr = new_cubearray((Cube){.eposm = ui}, pf_m);
		for (i = 0; i < 12; i++)
			if (edge_slice(arr->ep[i]) == 2)
				where_is_edge_aux[2][ui][arr->ep[i]] = i;
		free_cubearray(arr, pf_m);
	}

	for (ui = 0; ui < POW3TO7; ui++) {
		int_to_sum_zero_array(ui, 3, 8, auxarr);
		what_orientation_last_corner_aux[ui] = auxarr[7];
	}

	for (ui = 0; ui < POW2TO11; ui++) {
		int_to_sum_zero_array(ui, 2, 12, auxarr);
		what_orientation_last_edge_aux[ui] = auxarr[11];
	}

	for (ui = 0; ui < BINOM12ON4; ui++)
		for (uj = 0; uj < BINOM12ON4; uj++)
			epos_dependent_aux[ui][uj]=epos_dependent_pos(ui, uj);

	for (i = 0; i < NMOVES; i++) {
		for (j = 0; j < NMOVES; j++) {
			c1 = apply_move(i, apply_move(j, (Cube){0}));
			c2 = apply_move(j, apply_move(i, (Cube){0}));
			commute[i][j] = equal(c1, c2) && i && j;
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

	inverse_trans_aux[uf_mirror] = uf_mirror;
	inverse_trans_aux[ur_mirror] = ur_mirror;
	inverse_trans_aux[ul_mirror] = ul_mirror;
	inverse_trans_aux[ub_mirror] = ub_mirror;

	inverse_trans_aux[df_mirror] = df_mirror;
	inverse_trans_aux[dr_mirror] = dl_mirror;
	inverse_trans_aux[dl_mirror] = dr_mirror;
	inverse_trans_aux[db_mirror] = db_mirror;

	inverse_trans_aux[rf_mirror] = rf_mirror;
	inverse_trans_aux[rd_mirror] = br_mirror;
	inverse_trans_aux[rb_mirror] = lb_mirror;
	inverse_trans_aux[ru_mirror] = fl_mirror;

	inverse_trans_aux[lf_mirror] = lf_mirror;
	inverse_trans_aux[ld_mirror] = bl_mirror;
	inverse_trans_aux[lb_mirror] = rb_mirror;
	inverse_trans_aux[lu_mirror] = fr_mirror;

	inverse_trans_aux[fu_mirror] = fu_mirror;
	inverse_trans_aux[fr_mirror] = lu_mirror;
	inverse_trans_aux[fd_mirror] = bu_mirror;
	inverse_trans_aux[fl_mirror] = ru_mirror;

	inverse_trans_aux[bu_mirror] = fd_mirror;
	inverse_trans_aux[br_mirror] = rd_mirror;
	inverse_trans_aux[bd_mirror] = bd_mirror;
	inverse_trans_aux[bl_mirror] = ld_mirror;
}

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
				jj = cp_mtable[moves[k]][next[j]];
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
	unsigned int ui;
	Move m;

	/* Generate all move cycles and flips; I do this regardless */
	for (i = 0; i < NMOVES; i++) {
		if (i == U || i == x || i == y)
			continue;

		c = apply_alg_generic(equiv_alg[i], (Cube){0}, pf_all, false);

		arrs = (CubeArray) {
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
init_symdata()
{
	int i;

	for (i = 0; i < n_all_symdata; i++)
		gensym(all_sd[i]);
}

static void
init_trans() {
	Cube aux, cube, mirr, c[3];
	CubeArray epcp;
	int i, eparr[12], eoarr[12], cparr[8], coarr[8];
	unsigned int ui;
	Move mi, move;
	Trans m;

	/* Compute sources */
	for (i = 0; i < NTRANS; i++) {
		cube = apply_alg(rotation_algs[i % NROTATIONS], (Cube){0});

		epose_source[i] = edge_slice(what_edge_at(cube, FR));
		eposs_source[i] = edge_slice(what_edge_at(cube, UR));
		eposm_source[i] = edge_slice(what_edge_at(cube, UF));
		eofb_source[i]  = what_center_at(cube, F_center)/2;
		eorl_source[i]  = what_center_at(cube, R_center)/2;
		eoud_source[i]  = what_center_at(cube, U_center)/2;
		coud_source[i]  = what_center_at(cube, U_center)/2;
		cofb_source[i]  = what_center_at(cube, F_center)/2;
		corl_source[i]  = what_center_at(cube, R_center)/2;
	}

	if (read_ttables_file())
		return;

	fprintf(stderr, "Cannot load %s, generating it\n", "ttables"); 

	/* Initialize tables */
	for (m = 0; m < NTRANS; m++) {
		epcp = (CubeArray){ .ep = eparr, .cp = cparr };
		cube = apply_alg(rotation_algs[m % NROTATIONS], (Cube){0});
		cube_to_arrays(cube, &epcp, pf_epcp);
		if (m >= NROTATIONS) {
			apply_permutation(ep_mirror, eparr, 12);
			apply_permutation(cp_mirror, cparr,  8);
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
			if (m >= NROTATIONS)
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
			aux = apply_trans(m, apply_move(mi, (Cube){0}));
			for (move = 0; move < NMOVES; move++) {
				cube = apply_move(inverse_move_aux[move], aux);
				mirr = apply_trans(uf_mirror, cube);
				if (is_solved(cube, false) ||
				    is_solved(mirr, false))
					moves_ttable[m][mi] = move;
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
	rotation_algs[uf] = new_alg("");
	rotation_algs[ur] = new_alg("y");
	rotation_algs[ub] = new_alg("y2");
	rotation_algs[ul] = new_alg("y3");

	rotation_algs[df] = new_alg("z2");
	rotation_algs[dr] = new_alg("y z2");
	rotation_algs[db] = new_alg("x2");
	rotation_algs[dl] = new_alg("y3 z2");

	rotation_algs[rf] = new_alg("z3");
	rotation_algs[rd] = new_alg("z3 y");
	rotation_algs[rb] = new_alg("z3 y2");
	rotation_algs[ru] = new_alg("z3 y3");

	rotation_algs[lf] = new_alg("z");
	rotation_algs[ld] = new_alg("z y3");
	rotation_algs[lb] = new_alg("z y2");
	rotation_algs[lu] = new_alg("z y");

	rotation_algs[fu] = new_alg("x y2");
	rotation_algs[fr] = new_alg("x y");
	rotation_algs[fd] = new_alg("x");
	rotation_algs[fl] = new_alg("x y3");

	rotation_algs[bu] = new_alg("x3");
	rotation_algs[br] = new_alg("x3 y");
	rotation_algs[bd] = new_alg("x3 y2");
	rotation_algs[bl] = new_alg("x3 y3");
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
	return (Cube) {
		.epose = epose_mtable[m][cube.epose],
		.eposs = eposs_mtable[m][cube.eposs],
		.eposm = eposm_mtable[m][cube.eposm],
		.eofb  = eofb_mtable[m][cube.eofb],
		.eorl  = eorl_mtable[m][cube.eorl],
		.eoud  = eoud_mtable[m][cube.eoud],
		.coud  = coud_mtable[m][cube.coud],
		.cofb  = cofb_mtable[m][cube.cofb],
		.corl  = corl_mtable[m][cube.corl],
		.cp    = cp_mtable[m][cube.cp],
		.cpos  = cpos_mtable[m][cube.cpos]
	};
}


Cube
apply_trans(Trans t, Cube cube)
{
	int aux_epos[3] = { cube.epose, cube.eposs, cube.eposm };
	int aux_eo[3]   = { cube.eoud,  cube.eorl,  cube.eofb  };
	int aux_co[3]   = { cube.coud,  cube.corl,  cube.cofb  };

	return (Cube) {
		.epose = epose_ttable[t][aux_epos[epose_source[t]]],
		.eposs = eposs_ttable[t][aux_epos[eposs_source[t]]],
		.eposm = eposm_ttable[t][aux_epos[eposm_source[t]]],
		.eofb  = eo_ttable[t][aux_eo[eofb_source[t]]],
		.eorl  = eo_ttable[t][aux_eo[eorl_source[t]]],
		.eoud  = eo_ttable[t][aux_eo[eoud_source[t]]],
		.coud  = co_ttable[t][aux_co[coud_source[t]]],
		.corl  = co_ttable[t][aux_co[corl_source[t]]],
		.cofb  = co_ttable[t][aux_co[cofb_source[t]]],
		.cp    = cp_ttable[t][cube.cp],
		.cpos  = cpos_ttable[t][cube.cpos]
	};
}

Move
base_move(Move m)
{
	if (m == NULLMOVE)
		return NULLMOVE;
	else
		return m - (m-1)%3;
}

Cube
compose(Cube c2, Cube c1)
{
	return compose_filtered(c2, c1, pf_all);
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
	return epos_dependent_aux[c.eposs/FACTORIAL4][c.epose/FACTORIAL4];
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

void
genptable(PruneData *pd)
{
	Move ms[NMOVES];
	int d;
	uint64_t j, oldn = 0;

	if (pd->generated)
		return;

	/* TODO: check if memory is enough, otherwise maybe crash gracefully? */
	pd->ptable = malloc(ptablesize(pd) * sizeof(uint8_t));

	if (read_ptable_file(pd)) {
		pd->generated = true;
		return;
	}

	fprintf(stderr, "Cannot load %s, generating it\n", pd->filename); 

	moveset_to_list(pd->moveset, NULL, ms);

	/* We use 4 bits per value, so any distance >= 15 is set to 15 */
	for (j = 0; j < pd->coord->max; j++)
		ptable_update(pd, pd->coord->cube(j), 15);

	/*TODO: change, set to 0 for every solved state (might be more than 1)*/
	ptable_update(pd, (Cube){0}, 0);
	pd->n = 1;

	for (d = 0; d < 15 && pd->n < pd->coord->max; d++) {
		genptable_bfs(pd, d, ms);
		fprintf(stderr, "Depth %d done, generated %lu\t(%lu/%lu)\n",
			d, pd->n - oldn, pd->n, pd->coord->max);
		oldn = pd->n;
	}

	if (!write_ptable_file(pd))
		fprintf(stderr, "Error writing ptable file\n");

	pd->generated = true;
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
	int i;

	if (reorient) {
		for (i = 0; i < NROTATIONS; i++)
			if (is_solved(apply_alg(rotation_algs[i], cube),false))
				return true;
		return false;
	} else {
		return equal(cube, (Cube){0});
	}
}

bool
is_solved_block(Cube cube, Block block)
{
	int i;

	for (i = 0; i < 12; i++)
		if (block.edge[i] && !is_solved_edge(cube, i))
			return false;
	for (i = 0; i < 8; i++)
		if (block.corner[i] && !is_solved_corner(cube, i))
			return false;
	for (i = 0; i < 6; i++)
		if (block.center[i] && !is_solved_center(cube, i))
			return false;

	return true;
}

bool
is_solved_center(Cube cube, Center c)
{
	return what_center_at(cube, c) == c;
}

bool
is_solved_corner(Cube cube, Corner c)
{
	return what_corner_at(cube, c) == c &&
	       what_orientation_corner(cube.coud, c);
}

bool
is_solved_edge(Cube cube, Edge e)
{
	return what_edge_at(cube, e) == e &&
	       what_orientation_edge(cube.eofb, e);
}

int
piece_orientation(Cube cube, int piece, char *orientation)
{
	int arr[12], n, b, x;

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
/*
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
*/

	for (int i = 0; i < 12; i++)
		printf(" %s ", edge_string[what_edge_at(cube, i)]);
	printf("\n");

	for (int i = 0; i < 12; i++)
		printf("  %d ", what_orientation_edge(cube.eofb, i));
	printf("\n");

	for (int i = 0; i < 8; i++)
		printf("%s ", corner_string[what_corner_at(cube, i)]);
	printf("\n");

	for (int i = 0; i < 8; i++)
		printf("  %d ", what_orientation_corner(cube.coud, i));
	printf("\n");

	for (int i = 0; i < 6; i++)
		printf("  %s ", center_string[what_center_at(cube, i)]);
	printf("\n");

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

/* TODO: clean pre_trans or put it back */
AlgList *
solve(Cube cube, Step step, SolveOptions *opts)
{
	/*AlgListNode *node;*/
	AlgList *sols = new_alglist();
	/*Cube c = apply_trans(opts->pre_trans, cube);*/
	DfsData dd = {
		.m           = 0,
		.niss        = false,
		.lb          = -1,
		.last1       = NULLMOVE,
		.last2       = NULLMOVE,
		.sols        = sols,
		.current_alg = new_alg("")
	};

	if (step.ready != NULL && !step.ready(cube)) {
		fprintf(stderr, "Cube not ready for solving step\n");
		return sols;
	}

	moveset_to_list(step.moveset, step.estimate, dd.sorted_moves);
	movelist_to_position(dd.sorted_moves, dd.move_position);

	for (dd.d = opts->min_moves;
	     dd.d <= opts->max_moves && !(sols->len && opts->optimal_only);
	     dd.d++) {
		if (opts->feedback)
			fprintf(stderr,
				"Found %d solutions, searching depth %d...\n",
				sols->len, dd.d);
		dfs(cube, step, opts, &dd);
	}

/*
	for (node = sols->first; node != NULL; node = node->next)
		transform_alg(inverse_trans(opts->pre_trans), node->alg);
*/

	free_alg(dd.current_alg);
	return sols;
}

Alg *
inverse_alg(Alg *alg)
{
	Alg *ret = new_alg("");
	int i;

	for (i = alg->len-1; i >= 0; i--)
		append_move(ret, inverse_move(alg->move[i]), alg->inv[i]);

	return ret;
}

Alg *
new_alg(char *str)
{
	Alg *alg = malloc(sizeof(Alg));
	int i;
	bool niss = false;
	Move j, m;

	alg->move      = malloc(30 * sizeof(Move));
	alg->inv       = malloc(30 * sizeof(bool));
	alg->allocated = 30;
	alg->len       = 0;

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
	int i;

	for (i = 0; i < alg->len; i++)
		append_move(ret, alg->move[i], !alg->inv[i]);

	return ret;
}

void
print_alg(Alg *alg, bool l)
{
	/* TODO: make it possible to print to stdout or to string */
	/* Maybe just return a string */
	char fill[4];
	int i;
	bool niss = false;

	for (i = 0; i < alg->len; i++) {
		if (!niss && alg->inv[i])
			strcpy(fill, i == 0 ? "(" : " (");
		if (niss && !alg->inv[i])
			strcpy(fill, ") ");
		if (niss == alg->inv[i])
			strcpy(fill, i == 0 ? "" : " ");

		printf("%s%s", fill, move_string[alg->move[i]]);
		niss = alg->inv[i];
	}

	if (niss)
		printf(")");
	if (l)
		printf(" (%d)", alg->len);

	printf("\n");
}

void
print_alglist(AlgList *al, bool l)
{
	AlgListNode *i;

	for (i = al->first; i != NULL; i = i->next)
		print_alg(i->alg, l);
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
		a[ptableval(pd, pd->coord->cube(i))]++;
		
	fprintf(stderr, "Values for table %s\n", pd->filename);
	for (i = 0; i < 16; i++)
		printf("%2lu\t%10lu\n", i, a[i]);
}

uint64_t
ptablesize(PruneData *pd)
{
	return (pd->coord->max + 1) / 2;
}

int
ptableval(PruneData *pd, Cube cube)
{
	return ptableval_index(pd, pd->coord->index(cube));
}

Alg *
rotation_alg(Trans i)
{
	return rotation_algs[i % NROTATIONS];
}

void
transform_alg(Trans t, Alg *alg)
{
	int i;
	
	for (i = 0; i < alg->len; i++)
		alg->move[i] = moves_ttable[t][alg->move[i]];
}

Center
what_center_at(Cube cube, Center c)
{
	return what_center_at_aux[cube.cpos][c];
}

Corner
what_corner_at(Cube cube, Corner c)
{
	return what_corner_at_aux[cube.cp][c];
}

Edge
what_edge_at(Cube cube, Edge e)
{
	Edge ret;
	CubeArray *arr = new_cubearray(cube, pf_ep);

	ret = arr->ep[e];

	free_cubearray(arr, pf_ep);
	return ret;
}

int
what_orientation_corner(int co, Corner c)
{
	if (c < 7)
		return (co / powint(3, c)) % 3;
	else
		return what_orientation_last_corner_aux[co];
}

int
what_orientation_edge(int eo, Edge e)
{
	if (e < 11)
		return (eo & (1 << e)) ? 1 : 0;
	else
		return what_orientation_last_edge_aux[eo];
}

Center
where_is_center(Cube cube, Center c)
{
	return where_is_center_aux[cube.cpos][c];
}

Corner
where_is_corner(Cube cube, Corner c)
{
	return where_is_corner_aux[cube.cp][c];
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
	init_cphtr_cosets();
	init_trans_aux();
	init_trans();
	init_symdata();
}

