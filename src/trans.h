#ifndef TRANS_H
#define TRANS_H

#include "moves.h"

void        apply_trans(Trans t, Cube *cube);
Trans       inverse_trans(Trans t);
Alg *       rotation_alg(Trans i);
void        transform_alg(Trans t, Alg *alg);
Move        transform_move(Trans t, Move m);
Trans       transform_trans(Trans t, Trans m);

void        init_trans();

#ifndef TRANS_C

extern TransGroup tgrp_udfix;

#else

TransGroup
tgrp_udfix = {
	.n = 16,
	.t = { uf, ur, ub, ul,
	       df, dr, db, dl,
	       uf_mirror, ur_mirror, ub_mirror, ul_mirror,
	       df_mirror, dr_mirror, db_mirror, dl_mirror },
};

#endif

#endif
