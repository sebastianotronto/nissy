#ifndef MOVES_H
#define MOVES_H

#include "alg.h"
#include "cube.h"
#include "env.h"

Cube        apply_alg(Alg *alg, Cube cube);
Cube        apply_alg_generic(Alg *alg, Cube c, PieceFilter f, bool a);
Cube        apply_move(Move m, Cube cube);
bool        commute(Move m1, Move m2);
bool        possible_next(Move m1, Move m2, Move m3);

void        init_moves();

#endif
