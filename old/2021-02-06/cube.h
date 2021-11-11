#ifndef CUBE_H
#define CUBE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "utils.h"

#define NMOVES (z3+1)

/* Constants for moves and pieces */
typedef enum {
  NULLMOVE,
  U, U2, U3, D, D2, D3, R, R2, R3, L, L2, L3, F, F2, F3, B, B2, B3,
  Uw, Uw2, Uw3, Dw, Dw2, Dw3, Rw, Rw2, Rw3,
  Lw, Lw2, Lw3, Fw, Fw2, Fw3, Bw, Bw2, Bw3,
  M, M2, M3, S, S2, S3, E, E2, E3,
  x, x2, x3, y, y2, y3, z, z2, z3,
} Move;
typedef enum {U_center,D_center,R_center,L_center,F_center,B_center} Center;
typedef enum { UF, UL, UB, UR, DF, DL, DB, DR, FR, FL, BL, BR } Edge;
typedef enum { UFR, UFL, UBL, UBR, DFR, DFL, DBL, DBR } Corner;

/* An alg is an array of "NissMoves", which can be on normal or on inverse. */
typedef struct { bool inverse; Move m; } NissMove;

/* Representation of the cube */
typedef struct {
  uint16_t eofb, eorl, eoud, coud, cofb, corl,
           epose, eposs, eposm, cp, cpos;
} Cube;

extern bool commute[NMOVES][NMOVES];
extern bool possible_next[NMOVES][NMOVES][NMOVES];
extern Move inverse[NMOVES];
/* Movesets */
extern bool standard_moveset[NMOVES];

int copy_alg(NissMove *src, NissMove *dest); /*return number of moves copied */

bool equal(Cube c1, Cube c2);
bool is_solvable(Cube cube);
/* reorient=true allows solved in wrong orientation */
bool is_solved(Cube cube, bool reorient);
void print_cube(Cube cube);
void print_moves(NissMove *alg);
int read_moves(char *str, NissMove *alg, int n); /* reads at most n moves */
void cleanup(NissMove *src, int n); /* rewrites using basic moves, at most n */
Cube blank_cube();
Cube inverse_cube(Cube cube);
Cube move_cube(Move m, Cube cube);
Cube compose(Cube c2, Cube c1); /* Use c2 as an alg */
Cube apply_alg(NissMove *alg, Cube cube);

void init_ttables(bool read, bool write);
void init_aux_tables();

void init_dbg();

#endif
