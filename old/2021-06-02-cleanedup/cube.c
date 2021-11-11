#include "cube.h"

/* Constants and macros *****************************************************/

#define pow2to11    2048
#define pow2to12    4096
#define pow3to7     2187
#define pow3to8     6561
#define pow12to4    20736
#define factorial4  24
#define factorial6  720
#define factorial8  40320
#define factorial12 479001600
#define binom12on4  495
#define binom8on4   70

#define BASEALGLEN  50
#define NMOVES      (z3+1)
#define NTRANS      (mirror+1)
#define NROTATIONS  (NTRANS-1)

#define min(a,b)    (((a) < (b)) ? (a) : (b))
#define max(a,b)    (((a) > (b)) ? (a) : (b))

/* Local types ***************************************************************/

typedef struct cubearray   CubeArray;
typedef struct dfsdata     DfsData;
typedef struct piecefilter PieceFilter;

struct
cubearray
{
	int *ep;
	int *eofb;
	int *eorl;
	int *eoud;
	int *cp;
	int *coud;
	int *corl;
	int *cofb;
	int *cpos;
};

struct
dfsdata
{
	int      d;
	int      m;
	bool     niss;
	Move     last1;
	Move     last2;
	AlgList *sols;
	Alg      current_alg;
};

struct
piecefilter
{
	bool epose;
	bool eposs;
	bool eposm;
	bool eofb;
	bool eorl;
	bool eoud;
	bool cp;
	bool coud;
	bool cofb;
	bool corl;
	bool cpos;
};

/* Local functions **********************************************************/

static Cube        admissible_ep(Cube cube, PieceFilter f);
static CubeArray * alloc_cubearray(CubeArray *arr, PieceFilter f);
static void        append_alg(AlgList *l, Alg alg);
static void        append_move(Alg alg, NissMove m);
static Cube        apply_alg_filtered(Alg alg, Cube cube, PieceFilter f);
static void        apply_permutation(int *perm, int *set, int n);
static Cube        apply_move_cubearray(Move m, Cube cube, PieceFilter f);
static Cube        arrays_to_cube(CubeArray arr, PieceFilter f);
static int         binomial(int n, int k);
static Cube        compose_filtered(Cube c2, Cube c1, PieceFilter f);
static void        cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f);
static int         digit_array_to_int(int *a, int n, int b);
static int         edge_slice(Edge e); /* E=0, S=1, M=2 */
static uint16_t    epos_from_arrays(int *epos, int *ep);
static int         factorial(int n);
static void        free_alglist(AlgList *l);
static void        free_cubearray(CubeArray *arr, PieceFilter f);
static void        index_to_perm(int p, int n, int *r);
static void        index_to_subset(int s, int n, int k, int *r);
static void        init_auxtables();
static void        init_moves();
static void        init_trans();
static void        int_to_digit_array(int a, int b, int n, int *r);
static void        int_to_sum_zero_array(int x, int b, int n, int *a);
static void        intarrcopy(int *src, int *dst, int n);
static int         invert_digits(int a, int b, int n);
static bool        is_perm(int *a, int n);
static bool        is_subset(int *a, int n, int k);
static Cube        move_via_arrays(CubeArray arr, Cube c, PieceFilter pf);
static Move *      moveset_to_movelist(bool *moveset, Step step);
static AlgList *   new_alglist();
static int         perm_sign(int a[], int n);
static int         perm_to_index(int *a, int n);
static int         powint(int a, int b);
static bool        read_rtables_file();
static bool        read_ttables_file();
static bool        realloc_alg(Alg alg, int n);
static Cube        rotate_via_compose(Trans r, Cube c, PieceFilter f);
static void        solve_dfs(Cube c, Step s, SolveOptions opts, DfsData dd);
static int         subset_to_index(int *a, int n, int k);
static void        sum_arrays_mod(int *src, int *dst, int n, int m);
static void        swap(int *a, int *b);
static bool        write_rtables_file();
static bool        write_ttables_file();

/* Aux tables ****************************************************************/

bool  commute[NMOVES][NMOVES];
bool  possible_next[NMOVES][NMOVES][NMOVES];
Move  inverse_move_aux[NMOVES];
Trans inverse_trans_aux[NTRANS];

/* Some utility constants ****************************************************/

static int zero12[12]     = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int zero8[8]       = { 0, 0, 0, 0, 0, 0, 0, 0 };

static int epe_solved[4]  = { FR, FL, BL, BR };
static int eps_solved[4]  = { UL, UR, DL, DR };
static int epm_solved[4]  = { UF, UB, DF, DB };

static int ep_mirror[12]  = { UF, UR, UB, UL, DF, DR, DB, DL, FL, FR, BR, BL };
static int cp_mirror[8]   = { UFL, UFR, UBR, UBL, DFL, DFR, DBR, DBL };
static int cpos_mirror[6] = {
	U_center, D_center,
	L_center, R_center,
	F_center, B_center
};

static long unsigned int me[12] = {
	factorial12/factorial8,
	factorial12/factorial8,
	factorial12/factorial8,
	pow2to11,
	pow2to11,
	pow2to11,
	factorial8,
	pow3to7,
	pow3to7,
	pow3to7,
	factorial6,
	NMOVES
};

static PieceFilter pf_all  = {
	true, true, true, true, true, true, true, true, true, true, true
};
static PieceFilter pf_epcp = {
	.epose = true, .eposs = true, .eposm = true, .cp = true
};
static PieceFilter pf_cpos = { .cpos = true };
static PieceFilter pf_cp   = { .cp = true };
static PieceFilter pf_ep   = { .epose = true, .eposs = true, .eposm = true };
static PieceFilter pf_e    = { .epose = true };
static PieceFilter pf_s    = { .eposs = true };
static PieceFilter pf_m    = { .eposm = true };
static PieceFilter pf_eo   = { .eofb = true, .eorl = true, .eoud = true };
static PieceFilter pf_co   = { .coud = true, .cofb = true, .corl = true };

/* Strings for moves and pieces **********************************************/

char move_string[NMOVES][5] = {
	"-",
	"U", "U2", "U\'", "D", "D2", "D\'",
	"R", "R2", "R\'", "L", "L2", "L\'",
	"F", "F2", "F\'", "B", "B2", "B\'",
	"Uw", "Uw2", "Uw\'", "Dw", "Dw2", "Dw\'",
	"Rw", "Rw2", "Rw\'", "Lw", "Lw2", "Lw\'",
	"Fw", "Fw2", "Fw\'", "Bw", "Bw2", "Bw\'",
	"M", "M2", "M\'",
	"S", "S2", "S\'",
	"E", "E2", "E\'",
	"x", "x2", "x\'",
	"y", "y2", "y\'",
	"z", "z2", "z\'"
};
char edge_string[12][5] = {
	"UF", "UL", "UB", "UR",
	"DF", "DL", "DB", "DR",
	"FR", "FL", "BL", "BR"
};
char corner_string[8][5] = {
	"UFR", "UFL", "UBL", "UBR",
	"DFR", "DFL", "DBL", "DBR"
};
char center_string[6][5] = {
	"U", "D",
	"R", "L",
	"F", "B"
};

/* Transition tables for transformations and moves ***************************/

uint16_t epose_rtable[NTRANS][factorial12/factorial8];
uint16_t eposs_rtable[NTRANS][factorial12/factorial8];
uint16_t eposm_rtable[NTRANS][factorial12/factorial8];
uint16_t eo_rtable[NTRANS][pow2to11];
uint16_t cp_rtable[NTRANS][factorial8];
uint16_t co_rtable[NTRANS][pow3to7];
uint16_t cpos_rtable[NTRANS][factorial6];
Move     moves_rtable[NTRANS][NMOVES];

uint16_t epose_ttable[NMOVES][factorial12/factorial8];
uint16_t eposs_ttable[NMOVES][factorial12/factorial8];
uint16_t eposm_ttable[NMOVES][factorial12/factorial8];
uint16_t eofb_ttable[NMOVES][pow2to11];
uint16_t eorl_ttable[NMOVES][pow2to11];
uint16_t eoud_ttable[NMOVES][pow2to11];
uint16_t cp_ttable[NMOVES][factorial8];
uint16_t coud_ttable[NMOVES][pow3to7];
uint16_t cofb_ttable[NMOVES][pow3to7];
uint16_t corl_ttable[NMOVES][pow3to7];
uint16_t cpos_ttable[NMOVES][factorial6];

/* Used to compute the full transition tables for moves and transformations **/

int epose_source[NTRANS];  /* 0 = epose, 1 = eposs, 2 = eposm */
int eposs_source[NTRANS];
int eposm_source[NTRANS];
int eofb_source[NTRANS];   /* 0 = eoud,  1 = eorl,  2 = eofb  */
int eorl_source[NTRANS];
int eoud_source[NTRANS];
int coud_source[NTRANS];   /* 0 = coud,  1 = corl,  2 = cofb  */
int cofb_source[NTRANS];
int corl_source[NTRANS];

NissMove rotation_niss[NTRANS][6];
NissMove rotation_algs[NROTATIONS][3] = { 
	{ { .m = NULLMOVE }, { .m = NULLMOVE }, { .m = NULLMOVE } },
	{ { .m = y        }, { .m = NULLMOVE }, { .m = NULLMOVE } },
	{ { .m = y2       }, { .m = NULLMOVE }, { .m = NULLMOVE } },
	{ { .m = y3       }, { .m = NULLMOVE }, { .m = NULLMOVE } },
	{ { .m = z2       }, { .m = NULLMOVE }, { .m = NULLMOVE } },
	{ { .m = y        }, { .m = z2       }, { .m = NULLMOVE } },
	{ { .m = x2       }, { .m = NULLMOVE }, { .m = NULLMOVE } },
	{ { .m = y3       }, { .m = z2       }, { .m = NULLMOVE } },
	{ { .m = z3       }, { .m = NULLMOVE }, { .m = NULLMOVE } },
	{ { .m = z3       }, { .m = y        }, { .m = NULLMOVE } },
	{ { .m = z3       }, { .m = y2       }, { .m = NULLMOVE } },
	{ { .m = z3       }, { .m = y3       }, { .m = NULLMOVE } },
	{ { .m = z        }, { .m = NULLMOVE }, { .m = NULLMOVE } },
	{ { .m = z        }, { .m = y3       }, { .m = NULLMOVE } },
	{ { .m = z        }, { .m = y2       }, { .m = NULLMOVE } },
	{ { .m = z        }, { .m = y        }, { .m = NULLMOVE } },
	{ { .m = x        }, { .m = y2       }, { .m = NULLMOVE } },
	{ { .m = x        }, { .m = y        }, { .m = NULLMOVE } },
	{ { .m = x        }, { .m = NULLMOVE }, { .m = NULLMOVE } },
	{ { .m = x        }, { .m = y3       }, { .m = NULLMOVE } },
	{ { .m = x3       }, { .m = NULLMOVE }, { .m = NULLMOVE } },
	{ { .m = x3       }, { .m = y        }, { .m = NULLMOVE } },
	{ { .m = x3       }, { .m = y2       }, { .m = NULLMOVE } },
	{ { .m = x3       }, { .m = y3       }, { .m = NULLMOVE } },
};

/* For each type of pieces only the effects of U, x and y are described */  
int edge_cycle[NMOVES][12] = {
	[U] = { UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR },
	[x] = { DF, FL, UF, FR, DB, BL, UB, BR, DR, DL, UL, UR },
	[y] = { UR, UF, UL, UB, DR, DF, DL, DB, BR, FR, FL, BL }
};
int eofb_flipped[NMOVES][12] = {
	[x] = { [UF] = 1, [UB] = 1, [DF] = 1, [DB] = 1 },
	[y] = { [FR] = 1, [FL] = 1, [BL] = 1, [BR] = 1 }
};
int eorl_flipped[NMOVES][12] = {
	[x] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	[y] = { [FR] = 1, [FL] = 1, [BL] = 1, [BR] = 1 }
};
int eoud_flipped[NMOVES][12] = {
	[U] = { [UF] = 1, [UL] = 1, [UB] = 1, [UR] = 1 },
	[x] = { [UF] = 1, [UB] = 1, [DF] = 1, [DB] = 1 },
	[y] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  }
};
int corner_cycle[NMOVES][8] = {
	[U] = {UBR, UFR, UFL, UBL, DFR, DFL, DBL, DBR},
	[x] = {DFR, DFL, UFL, UFR, DBR, DBL, UBL, UBR},
	[y] = {UBR, UFR, UFL, UBL, DBR, DFR, DFL, DBL}
};
int coud_flipped[NMOVES][8] = {
	[x] = {
		[UFR] = 2, [UBR] = 1, [UFL] = 1, [UBL] = 2,
		[DBR] = 2, [DFR] = 1, [DBL] = 1, [DFL] = 2
	}
};
int corl_flipped[NMOVES][8] = {
	[U] = { [UFR] = 1, [UBR] = 2, [UBL] = 1, [UFL] = 2 },
	[y] = {
		[UFR] = 1, [UBR] = 2, [UBL] = 1, [UFL] = 2,
		[DFR] = 2, [DBR] = 1, [DBL] = 2, [DFL] = 1
	}
};
int cofb_flipped[NMOVES][8] = {
	[U] = { [UFR] = 2, [UBR] = 1, [UBL] = 2, [UFL] = 1 },
	[x] = {
		[UFR] = 1, [UBR] = 2, [UBL] = 1, [UFL] = 2,
		[DFR] = 2, [DBR] = 1, [DBL] = 2, [DFL] = 1
	},
	[y] = {
		[UFR] = 2, [UBR] = 1, [UBL] = 2, [UFL] = 1,
		[DFR] = 1, [DBR] = 2, [DBL] = 1, [DFL] = 2
	}
};
int center_cycle[NMOVES][6] = {
	[x] = { F_center, B_center, R_center, L_center, D_center, U_center },
	[y] = { U_center, D_center, B_center, F_center, R_center, L_center }
};

/* Each move is reduced to a combination of U, x and y using this table */
Move equiv_moves[NMOVES][14] = {
	[U]   = { U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[U2]  = { U, U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[U3]  = { U, U, U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[D]   = { x, x, U, x, x, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[D2]  = { x, x, U, U, x, x, 0, 0, 0, 0, 0, 0, 0, 0 },
	[D3]  = { x, x, U, U, U, x, x, 0, 0, 0, 0, 0, 0, 0 },
	[R]   = { y, x, U, x, x, x, y, y, y, 0, 0, 0, 0, 0 },
	[R2]  = { y, x, U, U, x, x, x, y, y, y, 0, 0, 0, 0 },
	[R3]  = { y, x, U, U, U, x, x, x, y, y, y, 0, 0, 0 },
	[L]   = { y, y, y, x, U, x, x, x, y, 0, 0, 0, 0, 0 },
	[L2]  = { y, y, y, x, U, U, x, x, x, y, 0, 0, 0, 0 },
	[L3]  = { y, y, y, x, U, U, U, x, x, x, y, 0, 0, 0 },
	[F]   = { x, U, x, x, x, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[F2]  = { x, U, U, x, x, x, 0, 0, 0, 0, 0, 0, 0, 0 },
	[F3]  = { x, U, U, U, x, x, x, 0, 0, 0, 0, 0, 0, 0 },
	[B]   = { x, x, x, U, x, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[B2]  = { x, x, x, U, U, x, 0, 0, 0, 0, 0, 0, 0, 0 },
	[B3]  = { x, x, x, U, U, U, x, 0, 0, 0, 0, 0, 0, 0 },

	[Uw]  = { x, x, U, x, x, y, 0, 0, 0, 0, 0, 0, 0, 0 },
	[Uw2] = { x, x, U, U, x, x, y, y, 0, 0, 0, 0, 0, 0 },
	[Uw3] = { x, x, U, U, U, x, x, y, y, y, 0, 0, 0, 0 },
	[Dw]  = { U, y, y, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[Dw2] = { U, U, y, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[Dw3] = { U, U, U, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[Rw]  = { y, y, y, x, U, x, x, x, y, x, 0, 0, 0, 0 },
	[Rw2] = { y, y, y, x, U, U, x, x, x, y, x, x, 0, 0 },
	[Rw3] = { y, y, y, x, U, U, U, y, x, x, x, y, 0, 0 },
	[Lw]  = { y, x, U, x, x, x, y, y, y, x, x, x, 0, 0 },
	[Lw2] = { y, x, U, U, x, x, x, y, y, y, x, x, 0, 0 },
	[Lw3] = { y, x, U, U, U, x, x, x, y, y, y, x, 0, 0 },
	[Fw]  = { x, x, x, U, y, y, y, x, 0, 0, 0, 0, 0, 0 },
	[Fw2] = { x, x, x, U, U, y, y, x, 0, 0, 0, 0, 0, 0 },
	[Fw3] = { x, x, x, U, U, U, y, x, 0, 0, 0, 0, 0, 0 },
	[Bw]  = { x, U, y, y, y, x, x, x, 0, 0, 0, 0, 0, 0 },
	[Bw2] = { x, U, U, y, y, x, x, x, 0, 0, 0, 0, 0, 0 },
	[Bw3] = { x, U, U, U, y, x, x, x, 0, 0, 0, 0, 0, 0 },

	[M]   = { y, x, U, x, x, U, U, U, y, x, y, y, y, 0 },
	[M2]  = { y, x, U, U, x, x, U, U, x, x, x, y, 0, 0 },
	[M3]  = { y, x, U, U, U, x, x, U, y, x, x, x, y, 0 },
	[S]   = { x, U, U, U, x, x, U, y, y, y, x, 0, 0, 0 },
	[S2]  = { x, U, U, x, x, U, U, y, y, x, 0, 0, 0, 0 },
	[S3]  = { x, U, x, x, U, U, U, y, x, 0, 0, 0, 0, 0 },
	[E]   = { U, x, x, U, U, U, x, x, y, y, y, 0, 0, 0 },
	[E2]  = { U, U, x, x, U, U, x, x, y, y, 0, 0, 0, 0 },
	[E3]  = { U, U, U, x, x, U, x, x, y, 0, 0, 0, 0, 0 },

	[x]   = { x, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[x2]  = { x, x, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[x3]  = { x, x, x, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[y]   = { y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[y2]  = { y, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[y3]  = { y, y, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[z]   = { y, y, y, x, y, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[z2]  = { y, y, x, x, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
	[z3]  = { y, x, y, y, y, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

/* Local functions implementation ********************************************/

static Cube
admissible_ep(Cube cube, PieceFilter f)
{
	CubeArray arr;
	Cube ret;
	bool used[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i, j;

	alloc_cubearray(&arr, f);
	cube_to_arrays(cube, &arr, f);

	for (i = 0; i < 12; i++)
		if (arr.ep[i] != -1)
			used[arr.ep[i]] = true;

	for (i = 0, j = 0; i < 12; i++) {
		for ( ; j < 11 && used[j]; j++);
		if (arr.ep[i] == -1)
			arr.ep[i] = j++;
	}

	ret = arrays_to_cube(arr, pf_ep);
	free_cubearray(&arr, f);

	return ret;
}

static CubeArray *
alloc_cubearray(CubeArray *arr, PieceFilter f)
{
	if (f.epose || f.eposs || f.eposm)
		arr->ep   = malloc(12 * sizeof(int));
	if (f.eofb)
		arr->eofb = malloc(12 * sizeof(int));
	if (f.eorl)
		arr->eorl = malloc(12 * sizeof(int));
	if (f.eoud)
		arr->eoud = malloc(12 * sizeof(int));
	if (f.cp)
		arr->cp   = malloc(8 * sizeof(int));
	if (f.coud)
		arr->coud = malloc(8 * sizeof(int));
	if (f.corl)
		arr->corl = malloc(8 * sizeof(int));
	if (f.cofb)
		arr->cofb = malloc(8 * sizeof(int));
	if (f.cpos)
		arr->cpos = malloc(6 * sizeof(int));

	return arr;
}

static void
append_alg(AlgList *l, Alg alg)
{
	AlgListNode *node = malloc(sizeof(AlgListNode));

	node->alg = new_alg("");
	copy_alg(alg, node->alg);
	node->next = NULL;

	if (++l->len == 1)
		l->first = node;
	else
		l->last->next = node;
	l->last = node;
}

static void
append_move(Alg alg, NissMove m)
{
	int n = len(alg);

	if (!realloc_alg(alg, n))
		return;

	alg[n]   = m;
	alg[n+1] = (NissMove) { .m = NULLMOVE };
}

static Cube
apply_alg_filtered(Alg alg, Cube cube, PieceFilter f)
{
	Cube ret = {0};
	int i;

	for (i = 0; alg[i].m != NULLMOVE; i++)
		if (alg[i].inverse)
			ret = apply_move(alg[i].m, ret);

	ret = compose_filtered(cube, inverse_cube(ret), f);

	for (i = 0; alg[i].m != NULLMOVE; i++)
		if (!alg[i].inverse)
			ret = apply_move(alg[i].m, ret);

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

	intarrcopy(aux, set, n);
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

	return move_via_arrays(m_arr, cube, f);
}

static Cube
arrays_to_cube(CubeArray arr, PieceFilter f)
{
	Cube ret = {0};
	int epose[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int eposs[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int eposm[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int epe[4], eps[4], epm[4];
	int i, ie, is, im, j;

	/* Again, ep is the hardest part */
	if (f.epose) {
		for (i = 0, ie = 0; i < 12; i++) {
			for (j = 0; j < 4; j++) {
				if (arr.ep[i] == epe_solved[j]) {
					epe[ie++] = j;
					epose[i] = 1;
				}
			}
		}
		ret.epose = epos_from_arrays(epose, epe);
	}

	if (f.eposs) {
		for (i = 0, is = 0; i < 12; i++) {
			for (j = 0; j < 4; j++) {
				if (arr.ep[i] == eps_solved[j]) {
					eps[is++] = j;
					eposs[i] = 1;
				}
			}
		}
		for (i = 0; i < 4; i++)
			swap(&eposs[eps_solved[i]], &eposs[i+8]);
		ret.eposs = epos_from_arrays(eposs, eps);
	}

	if (f.eposm) {
		for (i = 0, im = 0; i < 12; i++) {
			for (j = 0; j < 4; j++) {
				if (arr.ep[i] == epm_solved[j]) {
					epm[im++] = j;
					eposm[i] = 1;
				}
			}
		}
		for (i = 0; i < 4; i++)
			swap(&eposm[epm_solved[i]], &eposm[i+8]);
		ret.eposm = epos_from_arrays(eposm, epm);
	}

	if (f.eofb)
		ret.eofb = digit_array_to_int(arr.eofb, 11, 2);
	if (f.eorl)
		ret.eorl = digit_array_to_int(arr.eorl, 11, 2);
	if (f.eoud)
		ret.eoud = digit_array_to_int(arr.eoud, 11, 2);
	if (f.cp)
		ret.cp = perm_to_index(arr.cp, 8);
	if (f.coud)
		ret.coud = digit_array_to_int(arr.coud, 7, 3);
	if (f.corl)
		ret.corl = digit_array_to_int(arr.corl, 7, 3);
	if (f.cofb)
		ret.cofb = digit_array_to_int(arr.cofb, 7, 3);
	if (f.cpos)
		ret.cpos = perm_to_index(arr.cpos, 6);

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
	CubeArray arr;
	Cube ret;

	alloc_cubearray(&arr, f);
	cube_to_arrays(c2, &arr, f);
	ret = move_via_arrays(arr, c1, f);
	free_cubearray(&arr, f);

	return ret;
}

static void
cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f)
{
	int epose[12], eposs[12], eposm[12];
	int epe[4], eps[4], epm[4];
	int i, ie, is, im;

	/* ep is the hardest */
	if (f.epose || f.eposs || f.eposm)
		for (i = 0; i < 12; i++)
			arr->ep[i] = -1;

	if (f.epose) {
		index_to_perm(cube.epose % factorial(4), 4, epe);
		index_to_subset(cube.epose / factorial(4), 12, 4, epose);
		for (i = 0, ie = 0; i < 12; i++)
			if (epose[i])
				arr->ep[i] = epe_solved[epe[ie++]];
	}

	if (f.eposs) {
		index_to_perm(cube.eposs % factorial(4), 4, eps);
		index_to_subset(cube.eposs / factorial(4), 12, 4, eposs);
		for (i = 0; i < 4; i++)
			swap(&eposs[eps_solved[i]], &eposs[i+8]);
		for (i = 0, is = 0; i < 12; i++)
			if (eposs[i])
				arr->ep[i] = eps_solved[eps[is++]]; 
	}

	if (f.eposm) {
		index_to_perm(cube.eposm % factorial(4), 4, epm);
		index_to_subset(cube.eposm / factorial(4), 12, 4, eposm);
		for (i = 0; i < 4; i++)
			swap(&eposm[epm_solved[i]], &eposm[i+8]);
		for (i = 0, im = 0; i < 12; i++)
			if (eposm[i])
				arr->ep[i] = epm_solved[epm[im++]]; 
	}

	/* All the others */
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
	if (e == FR || e == FL || e == BL || e == BR)
		return 0;
	if (e == UR || e == UL || e == DR || e == DL)
		return 1;

	return 2;
}

static uint16_t
epos_from_arrays(int *epos, int *ep)
{
	return factorial4 * subset_to_index(epos,12,4) + perm_to_index(ep,4);
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

static void
free_alglist(AlgList *l)
{
	AlgListNode *aux, *i = l->first;

	while (i != NULL) {
		aux = i->next;
		free(i);
		i = aux;
	}
	free(l);

	return;
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
init_auxtables()
{
	Cube c1, c2;
	int i, j, k;
	bool cij, p1, p2;

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
				p1 = j && (j-(j-1)%3) == (k-(k-1)%3);
				p2 = i && (i-(i-1)%3) == (k-(k-1)%3);
				cij = commute[i][j];
				possible_next[i][j][k] = !(p1 || (cij && p2));
			}
		}
	}

	for (i = 0; i < NMOVES; i++)
		inverse_move_aux[i] =
			i == NULLMOVE ? NULLMOVE : i + 2 - 2*((i-1)%3);

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
init_moves() {
	Cube c;
	CubeArray arrs;
	int i, j;
	uint16_t ui;
	Move m;

	/* Generate all move cycles and flips; I do this regardless */
	for (i = 0; i < NMOVES; i++) {
		if (i == U || i == x || i == y)
			continue;

		c = (Cube){0};
		for (j = 0; equiv_moves[i][j]; j++)
			c = apply_move_cubearray(equiv_moves[i][j], c, pf_all);

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

	if (read_ttables_file())
		return;

	fprintf(stderr, "ttables not found, generating them.\n");

	/* Initialize transition tables */
	for (m = 0; m < NMOVES; m++) {
		for (ui = 0; ui < factorial12/factorial8; ui++) {
			c = (Cube){ .epose = ui };
			c = apply_move_cubearray(m, c, pf_e);
			epose_ttable[m][ui] = c.epose;

			c = (Cube){ .eposs = ui };
			c = apply_move_cubearray(m, c, pf_s);
			eposs_ttable[m][ui] = c.eposs;

			c = (Cube){ .eposm = ui };
			c = apply_move_cubearray(m, c, pf_m);
			eposm_ttable[m][ui] = c.eposm;
		}
		for (ui = 0; ui < pow2to11; ui++ ) {
			c = (Cube){ .eofb = ui };
			c = apply_move_cubearray(m, c, pf_eo);
			eofb_ttable[m][ui] = c.eofb;

			c = (Cube){ .eorl = ui };
			c = apply_move_cubearray(m, c, pf_eo);
			eorl_ttable[m][ui] = c.eorl;

			c = (Cube){ .eoud = ui };
			c = apply_move_cubearray(m, c, pf_eo);
			eoud_ttable[m][ui] = c.eoud;
		}
		for (ui = 0; ui < pow3to7; ui++) {
			c = (Cube){ .coud = ui };
			c = apply_move_cubearray(m, c, pf_co);
			coud_ttable[m][ui] = c.coud;

			c = (Cube){ .corl = ui };
			c = apply_move_cubearray(m, c, pf_co);
			corl_ttable[m][ui] = c.corl;

			c = (Cube){ .cofb = ui };
			c = apply_move_cubearray(m, c, pf_co);
			cofb_ttable[m][ui] = c.cofb;
		}
		for (ui = 0; ui < factorial8; ui++) {
			c = (Cube){ .cp = ui };
			c = apply_move_cubearray(m, c, pf_cp);
			cp_ttable[m][ui] = c.cp;
		}
		for (ui = 0; ui < factorial6; ui++) {
			c = (Cube){ .cpos = ui };
			c = apply_move_cubearray(m, c, pf_cpos);
			cpos_ttable[m][ui] = c.cpos;
		}
	}

	if (!write_ttables_file())
		fprintf(stderr, "Error writing ttables\n");
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
	Trans r, m;

	/* Compute sources */
	for (i = 0; i < NTRANS; i++) {
		if (i == mirror)
			cube = (Cube){0};
		else
			cube = apply_alg(rotation_algs[i], (Cube){0});

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

	/* Compute rotation_niss array, necessary for rotate_via_compose */
	for (r = 0; r != mirror; r++) {
		concat(rotation_algs[r], rotation_algs[r], rotation_niss[r]);
		for (i = len(rotation_algs[r]); rotation_niss[r][i].m; i++)
			rotation_niss[r][i].inverse = true;
	}

	if (read_rtables_file())
		return;

	fprintf(stderr, "rtables not found, generating them.\n");
  
	/* Initialize tables */
	for (m = 0; m < NTRANS; m++) {
		if (m == mirror) {
			intarrcopy(ep_mirror, eparr, 12);
			intarrcopy(cp_mirror, cparr, 8);
		} else {
			epcp = (CubeArray){ .ep = eparr, .cp = cparr };
			cube = apply_alg(rotation_algs[m], (Cube){0});
			cube_to_arrays(cube, &epcp, pf_epcp);
		}

		for (ui = 0; ui < factorial12/factorial8; ui++) {
			c[0] = admissible_ep((Cube){ .epose = ui }, pf_e);
			c[1] = admissible_ep((Cube){ .eposs = ui }, pf_s);
			c[2] = admissible_ep((Cube){ .eposm = ui }, pf_m);

			cube = rotate_via_compose(m,c[epose_source[m]],pf_ep);
			epose_rtable[m][ui] = cube.epose;

			cube = rotate_via_compose(m,c[eposs_source[m]],pf_ep);
			eposs_rtable[m][ui] = cube.eposs;

			cube = rotate_via_compose(m,c[eposm_source[m]],pf_ep);
			eposm_rtable[m][ui] = cube.eposm;
		}
		for (ui = 0; ui < pow2to11; ui++ ) {
			int_to_sum_zero_array(ui, 2, 12, eoarr);
			apply_permutation(eparr, eoarr, 12);
			eo_rtable[m][ui] = digit_array_to_int(eoarr, 11, 2);
		}
		for (ui = 0; ui < pow3to7; ui++) {
			int_to_sum_zero_array(ui, 3, 8, coarr);
			apply_permutation(cparr, coarr, 8);
			co_rtable[m][ui] = digit_array_to_int(coarr, 7, 3);
			if (m == mirror)
				co_rtable[m][ui] =
					invert_digits(co_rtable[m][ui], 3, 7);
		}
		for (ui = 0; ui < factorial8; ui++) {
			cube = (Cube){ .cp = ui };
			cube = rotate_via_compose(m, cube, pf_cp);
			cp_rtable[m][ui] = cube.cp;
		}
		for (ui = 0; ui < factorial6; ui++) {
			cube = (Cube){ .cpos = ui };
			cube = rotate_via_compose(m, cube, pf_cpos);
			cpos_rtable[m][ui] = cube.cpos;
		}
		for (mi = 0; mi < NMOVES; mi++) {
			if (m == mirror) {
				b1 = (mi >= U && mi <= Bw3);
				b2 = (mi >= S && mi <= E3);
				b3 = (mi >= x && mi <= z3);
				if (b1 || b2 || b3)
					moves_rtable[m][mi] =
						inverse_move_aux[mi];
				else
					moves_rtable[m][mi] = mi;

				if ((mi-1)/3==(R-1)/3 || (mi-1)/3==(Rw-1)/3)
					moves_rtable[m][mi] += 3;
				if ((mi-1)/3==(L-1)/3 || (mi-1)/3==(L2-1)/3)
					moves_rtable[m][mi] -= 3;
			} else {
				aux = apply_trans(m, apply_move(mi,(Cube){0}));
				for (move = 0; move < NMOVES; move++) {
					cube = apply_move(
						inverse_move_aux[move], aux);
					if (is_solved(cube, false))
						moves_rtable[m][mi] = move;
				}
			}
		}
	}

	if (!write_rtables_file())
		fprintf(stderr, "Error writing rtables\n");
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

static void
intarrcopy(int *src, int *dst, int n)
{
	int i;

	for (i = 0; i < n; i++)
		dst[i] = src[i];
}

static int
invert_digits(int a, int b, int n)
{
	int i, *r = malloc(n * sizeof(int));

	int_to_digit_array(a, b, n, r);
	for (i = 0; i < n; i++)
		r[i] = (b-r[i]) % b;

	return digit_array_to_int(r, n, b);
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
move_via_arrays(CubeArray arr, Cube c, PieceFilter f)
{
	CubeArray arrc;
	Cube ret;

	alloc_cubearray(&arrc, f);
	cube_to_arrays(c, &arrc, f);

	if (f.epose || f.eposs || f.eposm)
		apply_permutation(arr.ep, arrc.ep, 12);

	if (f.eofb) {
		apply_permutation(arr.ep, arrc.eofb, 12);
		sum_arrays_mod(arr.eofb, arrc.eofb, 12, 2);
	}

	if (f.eorl) {
		apply_permutation(arr.ep, arrc.eorl, 12);
		sum_arrays_mod(arr.eorl, arrc.eorl, 12, 2);
	}

	if (f.eoud) {
		apply_permutation(arr.ep, arrc.eoud, 12);
		sum_arrays_mod(arr.eoud, arrc.eoud, 12, 2);
	}

	if (f.cp)
		apply_permutation(arr.cp, arrc.cp, 8);

	if (f.coud) {
		apply_permutation(arr.cp, arrc.coud, 8);
		sum_arrays_mod(arr.coud, arrc.coud, 8, 3);
	}

	if (f.corl) {
		apply_permutation(arr.cp, arrc.corl, 8);
		sum_arrays_mod(arr.corl, arrc.corl, 8, 3);
	}

	if (f.cofb) {
		apply_permutation(arr.cp, arrc.cofb, 8);
		sum_arrays_mod(arr.cofb, arrc.cofb, 8, 3);
	}

	if (f.cpos)
		apply_permutation(arr.cpos, arrc.cpos, 6);

	ret = arrays_to_cube(arrc, f);
	free_cubearray(&arrc, f);

	return ret;
}

static Move *
moveset_to_movelist(bool *moveset, Step step)
{
	Cube c;
	int *a = malloc(NMOVES * sizeof(int)), b[NMOVES];
	int na = 0, nb = 0;
	Move i;

	if (moveset != NULL) {
		for (i = NULLMOVE+1; i < NMOVES; i++) {
			if (moveset[i]) {
				c = apply_move(i, (Cube){0});
				if (step.f != NULL && step.f(c)) {
					a[na++] = i;
				} else {
					b[nb++] = i;
				}
			}
		}
	}

	intarrcopy(b, a+na, nb);
	a[na+nb] = NULLMOVE;

	return (Move *)a;
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

static bool
read_rtables_file()
{
	FILE *ttf;
	int b = sizeof(uint16_t);
	bool r = true;
	Move m;

	if ((ttf = fopen("rtables", "rb")) != NULL) {
		for (m = 0; m < NTRANS; m++) {
			r = r && fread(epose_rtable[m],b,me[0],ttf) == me[0];
			r = r && fread(eposs_rtable[m],b,me[1],ttf) == me[1];
			r = r && fread(eposm_rtable[m],b,me[2],ttf) == me[2];
			r = r && fread(eo_rtable[m],b,me[3],ttf) == me[3];
			r = r && fread(cp_rtable[m],b,me[6],ttf) == me[6];
			r = r && fread(co_rtable[m],b,me[7],ttf) == me[7];
			r = r && fread(cpos_rtable[m],b,me[10],ttf) == me[10];
			r = r && fread(moves_rtable[m],b,me[11],ttf) == me[11];
		}
		fclose(ttf);
		return r;
	} else {
		return false;
	}
}

static bool
read_ttables_file()
{
	FILE *ttf;
	int m, b = sizeof(uint16_t);
	bool r = true;

	if ((ttf = fopen("ttables", "rb")) != NULL) {
		for (m = 0; m < NMOVES; m++) {
			r = r && fread(epose_ttable[m],b,me[0],ttf) == me[0];
			r = r && fread(eposs_ttable[m],b,me[1],ttf) == me[1];
			r = r && fread(eposm_ttable[m],b,me[2],ttf) == me[2];
			r = r && fread(eofb_ttable[m],b,me[3],ttf) == me[3];
			r = r && fread(eorl_ttable[m],b,me[4],ttf) == me[4];
			r = r && fread(eoud_ttable[m],b,me[5],ttf) == me[5];
			r = r && fread(cp_ttable[m],b,me[6],ttf) == me[6];
			r = r && fread(coud_ttable[m],b,me[7],ttf) == me[7];
			r = r && fread(corl_ttable[m],b,me[8],ttf) == me[8];
			r = r && fread(cofb_ttable[m],b,me[9],ttf) == me[9];
			r = r && fread(cpos_ttable[m],b,me[10],ttf) == me[10];
		}
		fclose(ttf);
		return r;
	} else {
		return false;
	}
}

static bool
realloc_alg(Alg alg, int n)
{
	if (n+1 >= BASEALGLEN) {
		if (reallocarray(alg, n+2, sizeof(NissMove)) == NULL) {
			fprintf(stderr, "Error reallocating alg\n");
			return false;
		}
	}

	return true;
}

static Cube
rotate_via_compose(Trans r, Cube c, PieceFilter f)
{
	Cube ret;
	CubeArray ma = {
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

	if (r != mirror) {
		ret = apply_alg_filtered(rotation_niss[r], c, f);
	} else {
		ret = move_via_arrays(ma, (Cube){0}, f);
		ret = compose_filtered(c, ret, f);
		ret = move_via_arrays(ma, ret, f);
	}

	return ret;
}

static void
solve_dfs(Cube c, Step s, SolveOptions opts, DfsData dd)
{
	DfsData newdd;
	Move move;
	NissMove nm;
	int i;
	bool found_many, prune, niss_makes_sense;

	found_many = dd.sols->len >= opts.max_solutions;
	prune = (!opts.can_niss || dd.niss) && dd.m + s.f(c) > dd.d;
	if (found_many || prune)
		return;

	if (!s.f(c)) {
		if (dd.m == dd.d)
			append_alg(dd.sols, dd.current_alg);
		return;
	}

	for (i = 0; opts.sorted_moves[i] != NULLMOVE; i++) {
		move = opts.sorted_moves[i];
		if (possible_next[dd.last2][dd.last1][move]) {
			nm = (NissMove){ .m = move, .inverse = dd.niss };
			newdd = (DfsData) {
				.d           = dd.d,
				.m           = dd.m + 1,
				.niss        = dd.niss,
				.last1       = move,
				.last2       = dd.last1,
				.sols        = dd.sols,
				.current_alg = dd.current_alg,
			};

			append_move(dd.current_alg, nm);
			solve_dfs(apply_move(move, c), s, opts, newdd);
			remove_last_moves(dd.current_alg, 1);
		}
	}

	niss_makes_sense = !dd.m || (s.f(apply_move(dd.last1, (Cube){0})));
	if (opts.can_niss && !dd.niss && niss_makes_sense) {
		newdd = (DfsData) {
			.d           = dd.d,
			.m           = dd.m,
			.niss        = true,
			.last1       = NULLMOVE,
			.last2       = NULLMOVE,
			.sols        = dd.sols,
			.current_alg = dd.current_alg,
		};
		solve_dfs(inverse_cube(c), s, opts, newdd);
	}
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
write_rtables_file()
{
	FILE *ttf;
	bool r = true;
	int b = sizeof(uint16_t);
	Move m;

	if ((ttf = fopen("rtables", "wb")) != NULL) {
		for (m = 0; m < NTRANS; m++) {
			r = r && fwrite(epose_rtable[m],b,me[0], ttf) == me[0];
			r = r && fwrite(eposs_rtable[m],b,me[1], ttf) == me[1];
			r = r && fwrite(eposm_rtable[m],b,me[2], ttf) == me[2];
			r = r && fwrite(eo_rtable[m],b,me[3], ttf) == me[3];
			r = r && fwrite(cp_rtable[m],b,me[6], ttf) == me[6];
			r = r && fwrite(co_rtable[m],b,me[7], ttf) == me[7];
			r = r && fwrite(cpos_rtable[m],b,me[10],ttf) == me[10];
			r = r && fwrite(moves_rtable[m],b,me[11],ttf)== me[11];
		}
		fclose(ttf);
		return r;
	} else {
		return false;
	}
}

static bool
write_ttables_file()
{
	FILE *ttf;
	int m, b = sizeof(uint16_t);;
	bool r = true;

	if ((ttf = fopen("ttables", "wb")) != NULL) {
		for (m = 0; m < NMOVES; m++) {
			r = r && fwrite(epose_ttable[m],b,me[0],ttf) == me[0];
			r = r && fwrite(eposs_ttable[m],b,me[1],ttf) == me[1];
			r = r && fwrite(eposm_ttable[m],b,me[2],ttf) == me[2];
			r = r && fwrite(eofb_ttable[m],b,me[3],ttf) == me[3];
			r = r && fwrite(eorl_ttable[m],b,me[4],ttf) == me[4];
			r = r && fwrite(eoud_ttable[m],b,me[5],ttf) == me[5];
			r = r && fwrite(cp_ttable[m],b,me[6],ttf) == me[6];
			r = r && fwrite(coud_ttable[m],b,me[7],ttf) == me[7];
			r = r && fwrite(corl_ttable[m],b,me[8],ttf) == me[8];
			r = r && fwrite(cofb_ttable[m],b,me[9],ttf) == me[9];
			r = r && fwrite(cpos_ttable[m],b,me[10],ttf) == me[10];
		}
		fclose(ttf);
		return r;
	} else {
		return false;
	}
}

/* Public functions implementation *******************************************/

Cube
apply_alg(Alg alg, Cube cube)
{
	return apply_alg_filtered(alg, cube, pf_all);
}

Cube
apply_move(Move m, Cube cube)
{
	Cube moved = {0};

	moved.epose = epose_ttable[m][cube.epose];
	moved.eposs = eposs_ttable[m][cube.eposs];
	moved.eposm = eposm_ttable[m][cube.eposm];
	moved.eofb  = eofb_ttable[m][cube.eofb];
	moved.eorl  = eorl_ttable[m][cube.eorl];
	moved.eoud  = eoud_ttable[m][cube.eoud];
	moved.coud  = coud_ttable[m][cube.coud];
	moved.cofb  = cofb_ttable[m][cube.cofb];
	moved.corl  = corl_ttable[m][cube.corl];
	moved.cp    = cp_ttable[m][cube.cp];
	moved.cpos  = cpos_ttable[m][cube.cpos];

	return moved;
}

Cube
apply_trans(Trans t, Cube cube)
{
	Cube transformed = {0};
	uint16_t aux_epos[3] = { cube.epose, cube.eposs, cube.eposm };
	uint16_t aux_eo[3]   = { cube.eoud,  cube.eorl,  cube.eofb  };
	uint16_t aux_co[3]   = { cube.coud,  cube.corl,  cube.cofb  };

	transformed.epose = epose_rtable[t][aux_epos[epose_source[t]]];
	transformed.eposs = eposs_rtable[t][aux_epos[eposs_source[t]]];
	transformed.eposm = eposm_rtable[t][aux_epos[eposm_source[t]]];
	transformed.eofb  = eo_rtable[t][aux_eo[eofb_source[t]]];
	transformed.eorl  = eo_rtable[t][aux_eo[eorl_source[t]]];
	transformed.eoud  = eo_rtable[t][aux_eo[eoud_source[t]]];
	transformed.coud  = co_rtable[t][aux_co[coud_source[t]]];
	transformed.corl  = co_rtable[t][aux_co[corl_source[t]]];
	transformed.cofb  = co_rtable[t][aux_co[cofb_source[t]]];
	transformed.cp    = cp_rtable[t][cube.cp];
	transformed.cpos  = cpos_rtable[t][cube.cpos];

	return transformed;
}

/* TODO: this has to be changed using pre-computations */
bool
block_solved(Cube cube, Block block)
{
	CubeArray arr;
	int i;
	bool ret = true;

	alloc_cubearray(&arr, pf_all);
	cube_to_arrays(cube, &arr, pf_all);

	for (i = 0; i < 12; i++)
		ret = ret && !(block.edge[i] && (arr.ep[i] != i || arr.eofb[i]));
	for (i = 0; i < 8; i++)
		ret = ret && !(block.corner[i] && (arr.cp[i] != i || arr.coud[i]));
	for (i = 0; i < 6; i++)
		ret = ret && !(block.center[i] && arr.cpos[i] != i);

	free_cubearray(&arr, pf_all);

	return ret;
}

Center
center_at(Cube cube, Center c)
{
	int ret;
	CubeArray arr;
	
	alloc_cubearray(&arr, pf_cpos);
	cube_to_arrays(cube, &arr, pf_cpos);
	ret = arr.cpos[c];
	free_cubearray(&arr, pf_cpos);

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
	CubeArray arr;
	
	alloc_cubearray(&arr, pf_cp);
	cube_to_arrays(cube, &arr, pf_cp);
	ret = arr.cp[c];
	free_cubearray(&arr, pf_cp);

	return ret;
}

Edge
edge_at(Cube cube, Edge e)
{
	int ret;
	CubeArray arr;
	
	alloc_cubearray(&arr, pf_ep);
	cube_to_arrays(cube, &arr, pf_ep);
	ret = arr.ep[e];
	free_cubearray(&arr, pf_ep);

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
	CubeArray arr, inv;
	Cube ret;
	int i;

	alloc_cubearray(&arr, pf_all);
	alloc_cubearray(&inv, pf_all);

	cube_to_arrays(cube, &arr, pf_all);

	for (i = 0; i < 12; i++) {
		inv.ep[arr.ep[i]]   = i;
		inv.eofb[arr.ep[i]] = arr.eofb[i];
		inv.eorl[arr.ep[i]] = arr.eorl[i];
		inv.eoud[arr.ep[i]] = arr.eoud[i];
	}

	for (i = 0; i < 8; i++) {
		inv.cp[arr.cp[i]]   = i;
		inv.coud[arr.cp[i]] = (3 - arr.coud[i]) % 3;
		inv.corl[arr.cp[i]] = (3 - arr.corl[i]) % 3;
		inv.cofb[arr.cp[i]] = (3 - arr.cofb[i]) % 3;
	}

	for (int i = 0; i < 6; i++)
		inv.cpos[arr.cpos[i]] = i;

	ret = arrays_to_cube(inv, pf_all);
	free_cubearray(&arr, pf_all);
	free_cubearray(&inv, pf_all);

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
is_solved(Cube cube, bool reorient)
{
	int i;

	if (reorient)
		for (i = 0; i < NROTATIONS; i++)
			if (is_solved(apply_alg(rotation_algs[i], cube), false))
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
	CubeArray arr;

	alloc_cubearray(&arr, pf_all);
	cube_to_arrays(cube, &arr, pf_all);

	for (int i = 0; i < 12; i++)
		printf(" %s ", edge_string[arr.ep[i]]);
	printf("\n");

	for (int i = 0; i < 12; i++)
		printf("  %c ", arr.eofb[i] + '0');
	printf("\n");

	for (int i = 0; i < 8; i++)
		printf("%s ", corner_string[arr.cp[i]]);
	printf("\n");

	for (int i = 0; i < 8; i++)
		printf("  %c ", arr.coud[i] + '0');
	printf("\n");

	for (int i = 0; i < 6; i++)
		printf("  %s ", center_string[arr.cpos[i]]);
	printf("\n");

	free_cubearray(&arr, pf_all);
}

void
concat(Alg src1, Alg src2, Alg dest)
{
	int n1 = len(src1), n2 = len(src2);

	if (!realloc_alg(dest, n1+n2))
		return;

	copy_alg(src1, dest);
	copy_alg(src2, dest + n1);
}

void
copy_alg(Alg src, Alg dest)
{
	int i, n = len(src);

	if (!realloc_alg(dest, n))
		return;

	for (i = 0; src[i].m != NULLMOVE; i++)
		dest[i] = src[i];
	dest[i].m = NULLMOVE;
}

void
invert_alg(Alg src, Alg dest)
{
	Move m;
	int i, n = len(src);

	if (!realloc_alg(dest, n))
		return;
  
	for (i = 0; i < n; i++) {
		m = inverse_move_aux[src[i].m];
		dest[n-i-1] = (NissMove) { .m = m, .inverse = src[i].inverse };
	}
	dest[n].m = NULLMOVE;
}

int
len(Alg alg)
{
	int i;

	for (i = 0; alg[i].m != NULLMOVE; i++);

	return i;
}

Alg
new_alg(char *str)
{
	Alg alg = malloc(BASEALGLEN * sizeof(NissMove));
	int i;
	bool niss = false;
	Move j, m;
	NissMove nm;

	alg[0] = (NissMove){ .m = NULLMOVE };
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
				nm = (NissMove){ .m = m, .inverse = niss };
				append_move(alg, nm);
				break;
			}
		}
	}
	append_move(alg, (NissMove){ .m = NULLMOVE });

	return alg;
}

void
print_alg(Alg alg)
{
	char fill[4];
	int i;
	bool niss = false;

	for (i = 0; alg[i].m != NULLMOVE; i++) {
		if (!niss && alg[i].inverse)
			strcpy(fill, i == 0 ? "(" : " (");
		if (niss && !alg[i].inverse)
			strcpy(fill, ") ");
		if (niss == alg[i].inverse)
			strcpy(fill, i == 0 ? "" : " ");

		printf("%s%s", fill, move_string[alg[i].m]);
		niss = alg[i].inverse;
	}
	printf("%s\n", niss ? ")" : "");
}

void
remove_last_moves(Alg alg, int k)
{
	int n = len(alg);
	int i = max(0, n-k);

	alg[i].m = NULLMOVE;
}

AlgList *
solve(Cube cube, Step step, SolveOptions opts)
{
	AlgList *sols = new_alglist();
	AlgListNode *node;
	Cube c;
	DfsData dd;
	int i;

	if (step.ready != NULL && !step.ready(cube))
		return sols;

	if (opts.sorted_moves == NULL)
		opts.sorted_moves = moveset_to_movelist(opts.moveset, step);

	for (i = opts.min_moves; i <= opts.max_moves; i++) {
		if (sols->len && opts.optimal_only)
			break;

		c = apply_trans(opts.pre_trans, cube);
		dd = (DfsData) {
			.d           = i,
			.m           = 0,
			.niss        = false,
			.last1       = NULLMOVE,
			.last2       = NULLMOVE,
			.sols        = sols,
			.current_alg = new_alg("")
		};

		solve_dfs(c, step, opts, dd);
	}

	for (node = sols->first; node != NULL; node = node->next)
		transform_alg(inverse_trans_aux[opts.pre_trans], node->alg);

	return sols;
}

void
transform_alg(Trans t, Alg alg)
{
	int i;
	
	for (i = 0; alg[i].m != NULLMOVE; i++)
		alg[i].m = moves_rtable[t][alg[i].m];
}


void
init()
{
	init_moves();
	init_auxtables();
	init_trans();
}
