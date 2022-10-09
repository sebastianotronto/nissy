#define TRANS_C

#include "trans.h"

/* Local functions ***********************************************************/

/* Tables and other data *****************************************************/

static Cube mirror_cube = {
.ep = { [UF] = UF, [UL] = UR, [UB] = UB, [UR] = UL,
	[DF] = DF, [DL] = DR, [DB] = DB, [DR] = DL,
	[FR] = FL, [FL] = FR, [BL] = BR, [BR] = BL },
.cp = { [UFR] = UFL, [UFL] = UFR, [UBL] = UBR, [UBR] = UBL,
	[DFR] = DFL, [DFL] = DFR, [DBL] = DBR, [DBR] = DBL },
.xp = { [U_center] = U_center, [D_center] = D_center,
	[R_center] = L_center, [L_center] = R_center,
	[F_center] = F_center, [B_center] = B_center }
};

static char rotation_alg_string[100][NROTATIONS] = {
	[uf] = "",     [ur] = "y",    [ub] = "y2",    [ul] = "y3",
	[df] = "z2",   [dr] = "y z2", [db] = "x2",    [dl] = "y3 z2",
	[rf] = "z3",   [rd] = "z3 y", [rb] = "z3 y2", [ru] = "z3 y3",
	[lf] = "z",    [ld] = "z y3", [lb] = "z y2",  [lu] = "z y",
	[fu] = "x y2", [fr] = "x y",  [fd] = "x",     [fl] = "x y3",
	[bu] = "x3",   [br] = "x3 y", [bd] = "x3 y2", [bl] = "x3 y3",
};

Alg  *rotation_alg_arr[NROTATIONS];
Move  moves_ttable[NTRANS][NMOVES];
Trans trans_ttable[NTRANS][NTRANS];
Trans trans_itable[NTRANS];

/* Public functions **********************************************************/

void
apply_trans(Trans t, Cube *cube)
{
	Cube aux;
	Alg *inv;
	int i;

	inv = inverse_alg(rotation_alg(t % NROTATIONS));
	copy_cube(cube, &aux);
	make_solved(cube);

	if (t >= NROTATIONS)
		compose(&mirror_cube, cube);
	apply_alg(inv, cube);
	compose(&aux, cube);
	apply_alg(rotation_alg(t % NROTATIONS), cube);
	if (t >= NROTATIONS) {
		compose(&mirror_cube, cube);
		for (i = 0; i < 8; i++)
			cube->co[i] = (3 - cube->co[i]) % 3;
	}

	free_alg(inv);
}

/*
Trans
inverse_trans(Trans t)
{
	static Trans inverse_trans_aux[NTRANS] = {
		[uf] = uf, [ur] = ul, [ul] = ur, [ub] = ub,
		[df] = df, [dr] = dr, [dl] = dl, [db] = db,
		[rf] = lf, [rd] = bl, [rb] = rb, [ru] = fr,
		[lf] = rf, [ld] = br, [lb] = lb, [lu] = fl,
		[fu] = fu, [fr] = ru, [fd] = bu, [fl] = lu,
		[bu] = fd, [br] = ld, [bd] = bd, [bl] = rd,

		[uf_mirror] = uf_mirror, [ur_mirror] = ur_mirror,
		[ul_mirror] = ul_mirror, [ub_mirror] = ub_mirror,
		[df_mirror] = df_mirror, [dr_mirror] = dl_mirror,
		[dl_mirror] = dr_mirror, [db_mirror] = db_mirror,
		[rf_mirror] = rf_mirror, [rd_mirror] = br_mirror,
		[rb_mirror] = lb_mirror, [ru_mirror] = fl_mirror,
		[lf_mirror] = lf_mirror, [ld_mirror] = bl_mirror,
		[lb_mirror] = rb_mirror, [lu_mirror] = fr_mirror,
		[fu_mirror] = fu_mirror, [fr_mirror] = lu_mirror,
		[fd_mirror] = bu_mirror, [fl_mirror] = ru_mirror,
		[bu_mirror] = fd_mirror, [br_mirror] = rd_mirror,
		[bd_mirror] = bd_mirror, [bl_mirror] = ld_mirror
	};

	return inverse_trans_aux[t];
}
*/

Trans
inverse_trans(Trans t)
{
	return trans_itable[t];
}

Alg *
rotation_alg(Trans i)
{
	return rotation_alg_arr[i % NROTATIONS];
}

void
transform_alg(Trans t, Alg *alg)
{
	int i;

	for (i = 0; i < alg->len; i++)
		alg->move[i] = transform_move(t, alg->move[i]);
}

Move
transform_move(Trans t, Move m)
{
	return moves_ttable[t][m];
}

Trans
transform_trans(Trans t, Trans m)
{
	return trans_ttable[t][m];
}

void
init_trans() {
	static bool initialized = false;
	if (initialized)
		return;
	initialized = true;

	int i;
	Alg *nonsym_alg, *nonsym_inv;
	Cube aux, cube;
	Move mi, move;
	Trans t, u, v;

	init_moves();

	for (i = 0; i < NROTATIONS; i++)
		rotation_alg_arr[i] = new_alg(rotation_alg_string[i]);

	for (t = 0; t < NTRANS; t++) {
		for (mi = 0; mi < NMOVES; mi++) {
			make_solved(&aux);
			apply_move(mi, &aux);
			apply_trans(t, &aux);
			for (move = 0; move < NMOVES; move++) {
				copy_cube(&aux, &cube);
				apply_move(inverse_move(move), &cube);
				if (is_solved(&cube)) {
					moves_ttable[t][mi] = move;
					break;
				}
			}
		}
	}

	nonsym_alg = new_alg("R' U' F");
	nonsym_inv = inverse_alg(nonsym_alg);
		
	for (t = 0; t < NTRANS; t++) {
		for (u = 0; u < NTRANS; u++) {
			make_solved(&aux);
			apply_alg(nonsym_alg, &aux);
			apply_trans(u, &aux);
			apply_trans(t, &aux);
			for (v = 0; v < NTRANS; v++) {
				copy_cube(&aux, &cube);
				apply_trans(v, &cube);
				apply_alg(nonsym_inv, &cube);
				if (is_solved(&cube)) {
					/* This is the inverse of the correct
					   value, it will be inverted later */
					trans_ttable[t][u] = v;
					if (v == uf)
						trans_itable[t] = u;
					break;
				}
			}
		}
	}
	for (t = 0; t < NTRANS; t++)
		for (u = 0; u < NTRANS; u++)
			trans_ttable[t][u] = trans_itable[trans_ttable[t][u]];


	free_alg(nonsym_alg);
	free_alg(nonsym_inv);
}

