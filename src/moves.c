#define MOVES_C

#include "moves.h"

/* Local functions ***********************************************************/

static void        cleanup_aux(Alg *alg, Alg *ret, bool inv);

/* Tables and other data *****************************************************/

/* Moves are represented as cubes and applied using compose(). Every move is *
 * translated to a an <U, x, y> alg before filling the transition tables.    *
 * See init_moves().                                                         */

static Cube move_array[NMOVES];

static char equiv_alg_string[100][NMOVES] = {
	[NULLMOVE] = "",

	[U]   = "        U           ",
	[U2]  = "        UU          ",
	[U3]  = "        UUU         ",
	[D]   = "  xx    U    xx      ",
	[D2]  = "  xx    UU   xx      ",
	[D3]  = "  xx    UUU  xx      ",
	[R]   = "  yx    U    xxxyyy  ",
	[R2]  = "  yx    UU   xxxyyy  ",
	[R3]  = "  yx    UUU  xxxyyy  ",
	[L]   = "  yyyx  U    xxxy    ",
	[L2]  = "  yyyx  UU   xxxy    ",
	[L3]  = "  yyyx  UUU  xxxy    ",
	[F]   = "  x     U    xxx     ",
	[F2]  = "  x     UU   xxx     ",
	[F3]  = "  x     UUU  xxx     ",
	[B]   = "  xxx   U    x       ",
	[B2]  = "  xxx   UU   x       ",
	[B3]  = "  xxx   UUU  x       ",

	[Uw]  = "  xx    U    xx      y        ",
	[Uw2] = "  xx    UU   xx      yy       ",
	[Uw3] = "  xx    UUU  xx      yyy      ",
	[Dw]  = "        U            yyy      ",
	[Dw2] = "        UU           yy       ",
	[Dw3] = "        UUU          y        ",
	[Rw]  = "  yyyx  U    xxxy    x        ",
	[Rw2] = "  yyyx  UU   xxxy    xx       ",
	[Rw3] = "  yyyx  UUU  xxxy    xxx      ",
	[Lw]  = "  yx    U    xxxyyy  xxx      ",
	[Lw2] = "  yx    UU   xxxyyy  xx       ",
	[Lw3] = "  yx    UUU  xxxyyy  x        ",
	[Fw]  = "  xxx   U    x       yxxxyyy  ",
	[Fw2] = "  xxx   UU   x       yxxyyy   ",
	[Fw3] = "  xxx   UUU  x       yxyyy    ",
	[Bw]  = "  x     U    xxx     yxyyy    ",
	[Bw2] = "  x     UU   xxx     yxxyyy   ",
	[Bw3] = "  x     UUU  xxx     yxxxyyy  ",

	[M]   = "  yx  U    xx  UUU  yxyyy  ",
	[M2]  = "  yx  UU   xx  UU   xxxy   ",
	[M3]  = "  yx  UUU  xx  U    yxxxy  ",
	[S]   = "  x   UUU  xx  U    yyyx   ",
	[S2]  = "  x   UU   xx  UU   yyx    ",
	[S3]  = "  x   U    xx  UUU  yx     ",
	[E]   = "      U    xx  UUU  xxyyy  ",
	[E2]  = "      UU   xx  UU   xxyy   ",
	[E3]  = "      UUU  xx  U    xxy    ",

	[x]   = "       x         ",
	[x2]  = "       xx        ",
	[x3]  = "       xxx       ",
	[y]   = "       y         ",
	[y2]  = "       yy        ",
	[y3]  = "       yyy       ",
	[z]   = "  yyy  x    y    ",
	[z2]  = "  yy   xx        ",
	[z3]  = "  y    x    yyy  "
};


/* Public functions **********************************************************/

void
apply_alg(Alg *alg, Cube *cube)
{
	Cube aux;
	int i;

	copy_cube(cube, &aux);
	make_solved(cube);

	for (i = 0; i < alg->len; i++)
		if (alg->inv[i])
			apply_move(alg->move[i], cube);

	invert_cube(cube);
	compose(&aux, cube);

	for (i = 0; i < alg->len; i++)
		if (!alg->inv[i])
			apply_move(alg->move[i], cube);
}

void
apply_move(Move m, Cube *cube)
{
	compose(&move_array[m], cube);
}

void
apply_move_centers(Move m, Cube *cube)
{
	compose_centers(&move_array[m], cube);
}

void
apply_move_corners(Move m, Cube *cube)
{
	compose_corners(&move_array[m], cube);
}

void
apply_move_edges(Move m, Cube *cube)
{
	compose_edges(&move_array[m], cube);
}

Alg *
cleanup(Alg *alg)
{
	int i, j, k, b[2], n, L;
	Move bb, m;
	Alg *ret;

	ret = new_alg("");
	cleanup_aux(alg, ret, false);
	cleanup_aux(alg, ret, true);

	do {
		for (i = 0, j = 0, n = 0; i < ret->len; i = j) {
			if (ret->move[i] > B3) {
				ret->move[n] = ret->move[i];
				ret->inv[n] = ret->inv[i];
				n++;
				j++;
				continue;
			}

			bb = 1 + ((base_move(ret->move[i]) - 1)/6)*6;
			while (j < ret->len &&
			       ret->move[j] <= B3 &&
			       ret->inv[j] == ret->inv[i] &&
			       1 + ((base_move(ret->move[j]) - 1)/6)*6 == bb)
				j++;
			
			for (k = i, b[0] = 0, b[1] = 0; k < j; k++) {
				m = ret->move[k];
				if (base_move(m) == bb)
					b[0] = (b[0]+1+m-base_move(m)) % 4;
				else
					b[1] = (b[1]+1+m-base_move(m)) % 4;
			}

			for (k = 0; k < 2; k++) {
				if (b[k] != 0) {
					ret->move[n] = bb + b[k] - 1 + 3*k;
					ret->inv[n] = ret->inv[i];
					n++;
				}
			}
		}

		L = ret->len;
		ret->len = n;
	} while (L != n);

	return ret;
}

static void
cleanup_aux(Alg *alg, Alg *ret, bool inv)
{
	int i, j;
	Cube c, d;
	Move m;
	Alg *equiv_alg;
	
	make_solved(&c);
	for (i = 0; i < alg->len; i++) {
		if (alg->inv[i] != inv)
			continue;

		equiv_alg = new_alg(equiv_alg_string[alg->move[i]]);

		for (j = 0; j < equiv_alg->len; j++)
			if (equiv_alg->move[j] == U)
				append_move(ret, 3 * c.xp[U_center] + 1, inv);
			else
				apply_move(equiv_alg->move[j], &c);

		free_alg(equiv_alg);
	}

	m = NULLMOVE;
	switch (c.xp[F_center]) {
	case U_center:
		m = x3;
		break;
	case D_center:
		m = x;
		break;
	case R_center:
		m = y;
		break;
	case L_center:
		m = y3;
		break;
	case B_center:
		if (c.xp[U_center] == U_center)
			m = y2;
		else
			m = x2;
		break;
	default:
		break;
	}

	make_solved(&d);
	apply_move(m, &d);
	if (m != NULLMOVE)
		append_move(ret, m, inv);

	m = NULLMOVE;
	if (c.xp[U_center] == d.xp[D_center]) {
		m = z2;
	} else if (c.xp[U_center] == d.xp[R_center]) {
		m = z3;
	} else if (c.xp[U_center] == d.xp[L_center]) {
		m = z;
	}
	if (m != NULLMOVE)
		append_move(ret, m, inv);
}

void
init_moves() {
	static bool initialized = false;
	if (initialized)
		return;
	initialized = true;

	Move m;
	Alg *equiv_alg[NMOVES];

	static const Cube mcu = {
		.ep = { UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR },
		.cp = { UBR, UFR, UFL, UBL, DFR, DFL, DBL, DBR },
	};
	static const Cube mcx = {
		.ep = { DF, FL, UF, FR, DB, BL, UB, BR, DR, DL, UL, UR },
		.eo = { [UF] = 1, [UB] = 1, [DF] = 1, [DB] = 1 },
		.cp = { DFR, DFL, UFL, UFR, DBR, DBL, UBL, UBR },
		.co = { [UFR] = 2, [UBR] = 1, [UFL] = 1, [UBL] = 2,
			[DBR] = 2, [DFR] = 1, [DBL] = 1, [DFL] = 2 },
		.xp = { F_center, B_center, R_center,
		        L_center, D_center, U_center },
	};
	static const Cube mcy = {
		.ep = { UR, UF, UL, UB, DR, DF, DL, DB, BR, FR, FL, BL },
		.eo = { [FR] = 1, [FL] = 1, [BL] = 1, [BR] = 1 },
		.cp = { UBR, UFR, UFL, UBL, DBR, DFR, DFL, DBL },
		.xp = { U_center, D_center, B_center,
		        F_center, R_center, L_center },
	};

	move_array[U] = mcu;
	move_array[x] = mcx;
	move_array[y] = mcy;

	for (m = 0; m < NMOVES; m++)
		equiv_alg[m] = new_alg(equiv_alg_string[m]);

	for (m = 0; m < NMOVES; m++) {
		switch (m) {
		case NULLMOVE:
			make_solved(&move_array[m]);
			break;
		case U:
		case x:
		case y:
			break;
		default:
			make_solved(&move_array[m]);
			apply_alg(equiv_alg[m], &move_array[m]);
			break;
		}
	}

	for (m = 0; m < NMOVES; m++)
		free_alg(equiv_alg[m]);
}

