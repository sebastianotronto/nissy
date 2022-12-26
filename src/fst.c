#define FST_C

#include "fst.h"

static FstCube          ep_to_fst_epos(int *ep);
static void             transform_ep_only(Trans t, int *ep, Cube *dst);
static void             init_fst_corner_invtables();
static void             init_fst_eo_invtables();
static void             init_fst_eo_update(uint64_t, uint64_t, int, Cube *);
static void             init_fst_transalg();
static void             init_fst_where_is_edge();

static int edge_slice[12] = {[FR] = 0, [FL] = 0, [BL] = 0, [BR] = 0,
                             [UL] = 1, [UR] = 1, [DR] = 1, [DL] = 1,
                             [UF] = 2, [UB] = 2, [DF] = 2, [DB] = 2};

static uint16_t         inv_coud[FACTORIAL8][POW3TO7];
static uint16_t         inv_cp[FACTORIAL8];
static uint16_t         uf_cp_to_fr_cp[FACTORIAL8];
static uint16_t         uf_cp_to_rd_cp[FACTORIAL8];

static int16_t          eo_invtable[3][POW2TO11][BINOM12ON4*FACTORIAL4];

static int              trans_ep_alg[NROTATIONS][12];
static int              trans_ep_inv[NROTATIONS][12];

static uint16_t         fst_where_is_edge_arr[3][12][BINOM12ON4*FACTORIAL4];

FstCube
cube_to_fst(Cube *cube)
{
	Cube c;
	FstCube ret;

	copy_cube(cube, &c);
	ret.uf_eofb    = coord_eofb.i[0]->index(&c);
	ret.uf_eposepe = coord_eposepe.i[0]->index(&c);
	ret.uf_coud    = coord_coud.i[0]->index(&c);
	ret.uf_cp      = coord_cp.i[0]->index(&c);
	copy_cube(cube, &c);
	apply_trans(fr, &c);
	ret.fr_eofb    = coord_eofb.i[0]->index(&c);
	ret.fr_eposepe = coord_eposepe.i[0]->index(&c);
	ret.fr_coud    = coord_coud.i[0]->index(&c);
	copy_cube(cube, &c);
	apply_trans(rd, &c);
	ret.rd_eofb    = coord_eofb.i[0]->index(&c);
	ret.rd_eposepe = coord_eposepe.i[0]->index(&c);
	ret.rd_coud    = coord_coud.i[0]->index(&c);

	return ret;
}

static FstCube
ep_to_fst_epos(int *ep)
{
	/* TODO: maybe optimize */

/* TODO: this version if faster, but broken
	 probably need to fix transform_ep_only()

	FstCube ret;
	Cube c;

	memcpy(c.ep, ep, 12 * sizeof(int));
	ret.uf_eposepe = coord_eposepe.i[0]->index(&c);

	transform_ep_only(fr, ep, &c);
	ret.fr_eposepe = coord_eposepe.i[0]->index(&c);

	transform_ep_only(rd, ep, &c);
	ret.rd_eposepe = coord_eposepe.i[0]->index(&c);
*/

	FstCube ret;
	Cube c, d;

	make_solved(&c);
	memcpy(c.ep, ep, 12 * sizeof(int));

	copy_cube(&c, &d);
	ret.uf_eposepe = coord_eposepe.i[0]->index(&d);

	copy_cube(&c, &d);
	apply_trans(fr, &d);
	ret.fr_eposepe = coord_eposepe.i[0]->index(&d);

	copy_cube(&c, &d);
	apply_trans(rd, &d);
	ret.rd_eposepe = coord_eposepe.i[0]->index(&d);

	return ret;
}

FstCube
fst_inverse(FstCube fst)
{
	FstCube ret;
	int i, ep_inv[12];

	for (i = 0; i < 12; i++)
		ep_inv[i] = fst_where_is_edge(i, fst);
	ret = ep_to_fst_epos(ep_inv);

	ret.uf_eofb = ((uint16_t)eo_invtable[0][fst.uf_eofb][fst.uf_eposepe]) |
	              ((uint16_t)eo_invtable[1][fst.uf_eofb][fst.fr_eposepe]) |
	              ((uint16_t)eo_invtable[2][fst.uf_eofb][fst.rd_eposepe]);
	ret.fr_eofb = ((uint16_t)eo_invtable[0][fst.fr_eofb][fst.uf_eposepe]) |
	              ((uint16_t)eo_invtable[1][fst.fr_eofb][fst.fr_eposepe]) |
	              ((uint16_t)eo_invtable[2][fst.fr_eofb][fst.rd_eposepe]);
	ret.rd_eofb = ((uint16_t)eo_invtable[0][fst.rd_eofb][fst.uf_eposepe]) |
	              ((uint16_t)eo_invtable[1][fst.rd_eofb][fst.fr_eposepe]) |
	              ((uint16_t)eo_invtable[2][fst.rd_eofb][fst.rd_eposepe]);

	ret.uf_cp = inv_cp[fst.uf_cp];

	ret.uf_coud = inv_coud[fst.uf_cp][fst.uf_coud];
	ret.fr_coud = inv_coud[uf_cp_to_fr_cp[fst.uf_cp]][fst.fr_coud];
	ret.rd_coud = inv_coud[uf_cp_to_rd_cp[fst.uf_cp]][fst.rd_coud];

	return ret;
}

FstCube
fst_move(Move m, FstCube fst)
{
	FstCube ret;
	Move m_fr, m_rd;

	m_fr = transform_move(fr, m);
	m_rd = transform_move(rd, m);

	ret.uf_eofb    = coord_eofb.mtable[m][fst.uf_eofb];
	ret.uf_eposepe = coord_eposepe.mtable[m][fst.uf_eposepe];
	ret.uf_coud    = coord_coud.mtable[m][fst.uf_coud];
	ret.uf_cp      = coord_cp.mtable[m][fst.uf_cp];

	ret.fr_eofb    = coord_eofb.mtable[m_fr][fst.fr_eofb];
	ret.fr_eposepe = coord_eposepe.mtable[m_fr][fst.fr_eposepe];
	ret.fr_coud    = coord_coud.mtable[m_fr][fst.fr_coud];

	ret.rd_eofb    = coord_eofb.mtable[m_rd][fst.rd_eofb];
	ret.rd_eposepe = coord_eposepe.mtable[m_rd][fst.rd_eposepe];
	ret.rd_coud    = coord_coud.mtable[m_rd][fst.rd_coud];

	return ret;
}

void
fst_to_cube(FstCube fst, Cube *cube)
{
	Cube e, s, m;
	int i;

	coord_eposepe.i[0]->to_cube(fst.uf_eposepe, &e);
	coord_eposepe.i[0]->to_cube(fst.fr_eposepe, &s);
	apply_trans(inverse_trans(fr), &s);
	coord_eposepe.i[0]->to_cube(fst.rd_eposepe, &m);
	apply_trans(inverse_trans(rd), &m);

	for (i = 0; i < 12; i++) {
		if (edge_slice[e.ep[i]] == 0)
			cube->ep[i] = e.ep[i];
		if (edge_slice[s.ep[i]] == 1)
			cube->ep[i] = s.ep[i];
		if (edge_slice[m.ep[i]] == 2)
			cube->ep[i] = m.ep[i];
	}

	coord_eofb.i[0]->to_cube((uint64_t)fst.uf_eofb, cube);
	coord_coud.i[0]->to_cube((uint64_t)fst.uf_coud, cube);
	coord_cp.i[0]->to_cube((uint64_t)fst.uf_cp, cube);

	for (i = 0; i < 6; i++)
		cube->xp[i] = i;
}

int
fst_where_is_edge(int e, FstCube fst)
{
	switch (edge_slice[e]) {
	case 0:
		return fst_where_is_edge_arr[0][e][fst.uf_eposepe];
	case 1:
		return fst_where_is_edge_arr[1][e][fst.fr_eposepe];
	default:
		return fst_where_is_edge_arr[2][e][fst.rd_eposepe];
	}

	return -1;
}

static void
transform_ep_only(Trans t, int *ep, Cube *dst)
{
	int i;

	for (i = 0; i < 12; i++)
		dst->ep[i] = trans_ep_alg[t][ep[trans_ep_inv[t][i]]];
}

void
init_fst()
{
	init_trans();
	gen_coord(&coord_eofb);
	gen_coord(&coord_eposepe);
	gen_coord(&coord_coud);
	gen_coord(&coord_cp);

	init_fst_corner_invtables();
	init_fst_eo_invtables();
	init_fst_transalg();
	init_fst_where_is_edge();
}

static void
init_fst_corner_invtables()
{
	Cube c, d;
	uint64_t cp, coud;

	for (cp = 0; cp < FACTORIAL8; cp++) {
		make_solved_corners(&c);
		coord_cp.i[0]->to_cube(cp, &c);

		copy_cube_corners(&c, &d);
		invert_cube_corners(&d);
		inv_cp[cp] = coord_cp.i[0]->index(&d);

		for (coud = 0; coud < POW3TO7; coud++) {
			copy_cube_corners(&c, &d);
			coord_coud.i[0]->to_cube(coud, &d);
			invert_cube_corners(&d);
			inv_coud[cp][coud] = coord_coud.i[0]->index(&d);
		}

		copy_cube_corners(&c, &d);
		apply_trans(fr, &d);
		uf_cp_to_fr_cp[cp] = coord_cp.i[0]->index(&d);

		copy_cube_corners(&c, &d);
		apply_trans(rd, &d);
		uf_cp_to_rd_cp[cp] = coord_cp.i[0]->index(&d);
	}
}

static void
init_fst_eo_invtables()
{
	uint64_t ep, eo;
	Cube c, d;

	for (ep = 0; ep < BINOM12ON4 * FACTORIAL4; ep++) {
		make_solved(&c);
		coord_eposepe.i[0]->to_cube(ep, &c);
		for (eo = 0; eo < POW2TO11; eo++) {
			copy_cube_edges(&c, &d);
			coord_eofb.i[0]->to_cube(eo, &d);
			init_fst_eo_update(eo, ep, 0, &d);

			apply_trans(inverse_trans(fr), &d);
			coord_eofb.i[0]->to_cube(eo, &d);
			init_fst_eo_update(eo, ep, 1, &d);

			copy_cube_edges(&c, &d);
			apply_trans(inverse_trans(rd), &d);
			coord_eofb.i[0]->to_cube(eo, &d);
			init_fst_eo_update(eo, ep, 2, &d);
		}
	}
}

static void
init_fst_eo_update(uint64_t eo, uint64_t ep, int s, Cube *d)
{
	int i;

	for (i = 0; i < 12; i++) {
		if (edge_slice[d->ep[i]] == s && d->eo[i] && d->ep[i] != 11)
			eo_invtable[s][eo][ep] |=
			    ((uint16_t)1) << ((uint16_t)d->ep[i]);
	}
}

static void
init_fst_transalg()
{
	Trans t;
	Alg *alg;
	Cube c;
	int i;

	for (t = uf; t < NROTATIONS; t++) {
		make_solved(&c);
		alg = rotation_alg(t);
		apply_alg(alg, &c);
		for (i = 0; i < 12; i++)
			trans_ep_alg[t][i] = c.ep[i];
		invert_cube_edges(&c);
		for (i = 0; i < 12; i++)
			trans_ep_inv[t][i] = c.ep[i];
	}
}

static void
init_fst_where_is_edge()
{
	Cube c, d;
	uint64_t e;

	make_solved(&c);
	for (e = 0; e < BINOM12ON4 * FACTORIAL4; e++) {
		coord_eposepe.i[0]->to_cube(e, &c);

		copy_cube_edges(&c, &d);
		fst_where_is_edge_arr[0][FR][e] = where_is_edge(FR, &d);
		fst_where_is_edge_arr[0][FL][e] = where_is_edge(FL, &d);
		fst_where_is_edge_arr[0][BL][e] = where_is_edge(BL, &d);
		fst_where_is_edge_arr[0][BR][e] = where_is_edge(BR, &d);

		copy_cube_edges(&c, &d);
		apply_trans(inverse_trans(fr), &d);
		fst_where_is_edge_arr[1][UL][e] = where_is_edge(UL, &d);
		fst_where_is_edge_arr[1][UR][e] = where_is_edge(UR, &d);
		fst_where_is_edge_arr[1][DL][e] = where_is_edge(DL, &d);
		fst_where_is_edge_arr[1][DR][e] = where_is_edge(DR, &d);

		copy_cube_edges(&c, &d);
		apply_trans(inverse_trans(rd), &d);
		fst_where_is_edge_arr[2][UF][e] = where_is_edge(UF, &d);
		fst_where_is_edge_arr[2][UB][e] = where_is_edge(UB, &d);
		fst_where_is_edge_arr[2][DF][e] = where_is_edge(DF, &d);
		fst_where_is_edge_arr[2][DB][e] = where_is_edge(DB, &d);
	}
}
