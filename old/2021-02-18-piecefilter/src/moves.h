#ifndef MOVES_H
#define MOVES_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "cube.h"
#include "utils.h"

#define NMOVES (z3+1)

typedef enum {
  NULLMOVE,
  U, U2, U3, D, D2, D3, R, R2, R3, L, L2, L3, F, F2, F3, B, B2, B3,
  Uw, Uw2, Uw3, Dw, Dw2, Dw3, Rw, Rw2, Rw3,
  Lw, Lw2, Lw3, Fw, Fw2, Fw3, Bw, Bw2, Bw3,
  M, M2, M3, S, S2, S3, E, E2, E3,
  x, x2, x3, y, y2, y3, z, z2, z3,
} Move;

/* An alg is an array of "NissMoves", which can be on normal or on inverse. */
typedef struct { bool inverse; Move m; } NissMove;

/* Movesets */
extern bool standard_moveset[NMOVES];

extern bool commute[NMOVES][NMOVES];
extern bool possible_next[NMOVES][NMOVES][NMOVES];
extern Move inverse[NMOVES];

bool is_solve_up_to_reorient(Cube cube);
int copy_alg(NissMove *src, NissMove *dest); /*return number of moves copied */
void print_moves(NissMove *alg);
int read_moves(char *str, NissMove *alg, int n); /* reads at most n moves */
void cleanup(NissMove *src, int n); /* rewrites using basic moves, at most n */
Cube move_cube(Move m, Cube cube);
/* I might want to replace this with two versions, one that uses PieceFilter */
Cube apply_alg(NissMove *alg, Cube cube);

/* Merge the following two?
   always in this order */
void init_ttables(bool read, bool write);
void init_aux_tables();


#endif
