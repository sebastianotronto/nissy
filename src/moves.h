#ifndef MOVES_H
#define MOVES_H

#include "alg.h"
#include "cube.h"
#include "env.h"

void        apply_alg(Alg *alg, Cube *cube);
void        apply_move(Move m, Cube *cube);
Alg *       cleanup(Alg *alg);

void        init_moves();

#endif
