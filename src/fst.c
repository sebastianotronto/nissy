#define FST_C

#include "fst.h"

static void             fst_to_ep(FstCube fst, int *ep);

FstCube
cube_to_fst(Cube *cube)
{
	Cube c;
	FstCube ret;

	copy_cube(cube, &c);
	ret.uf_eofb    = index_eofb(&c);
	ret.uf_eposepe = index_eposepe(&c);
	ret.uf_coud    = index_coud(&c);
	ret.uf_cp      = index_cp(&c);
	copy_cube(cube, &c);
	transform_cube(fr, &c);
	ret.fr_eofb    = index_eofb(&c);
	ret.fr_eposepe = index_eposepe(&c);
	ret.fr_coud    = index_coud(&c);
	transform_cube(rd, &c);
	ret.rd_eofb    = index_eofb(&c);
	ret.rd_eposepe = index_eposepe(&c);
	ret.rd_coud    = index_coud(&c);

	return ret;
}

FstCube
fst_inverse(FstCube fst)
{
	/* TODO */
}

FstCube
fst_move(Move m, FstCube fst)
{
	/* TODO */
}

void
fst_to_cube(FstCube fst, Cube *cube)
{
	invindex_eofb((uint64_t)fst.uf_eofb, cube);
	fst_to_ep(fst, cube->ep);
	invindex_coud((uint64_t)fst.uf_coud, cube);
	invindex_cp((uint64_t)fst.uf_cp, cube);
}

static void
fst_to_ep(FstCube fst, int *ep)
{
	/* TODO */
}

#endif
