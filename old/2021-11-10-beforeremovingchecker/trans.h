#ifndef TRANS_H
#define TRANS_H

#include "moves.h"

Cube        apply_trans(Trans t, Cube cube);
Trans       inverse_trans(Trans t);
Alg *       rotation_alg(Trans i);
void        transform_alg(Trans i, Alg *alg);

void        init_trans();

#endif
