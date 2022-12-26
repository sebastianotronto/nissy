#ifndef MOVES_H
#define MOVES_H

#include "alg.h"
#include "cube.h"
#include "env.h"

void        apply_alg(Alg *alg, Cube *cube);
void        apply_move(Move m, Cube *cube);
void        apply_move_centers(Move m, Cube *cube);
void        apply_move_corners(Move m, Cube *cube);
void        apply_move_edges(Move m, Cube *cube);
Alg *       cleanup(Alg *alg);

void        init_moves();

#endif
