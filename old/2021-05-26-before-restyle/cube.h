#ifndef CUBE_H
#define CUBE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "utils.h"

typedef enum {U_center,D_center,R_center,L_center,F_center,B_center} Center;
typedef enum { UF, UL, UB, UR, DF, DL, DB, DR, FR, FL, BL, BR } Edge;
typedef enum { UFR, UFL, UBL, UBR, DFR, DFL, DBL, DBR } Corner;

typedef struct {
  uint16_t eofb, eorl, eoud, coud, cofb, corl,
           epose, eposs, eposm, cp, cpos;
} Cube;

typedef struct {
  bool edge[12], corner[8], center[6];
} Block;

typedef struct {
  bool epose, eposs, eposm, eofb, eorl, eoud, cp, coud, cofb, corl, cpos;
} PieceFilter;

typedef struct {
  int *ep, *eofb, *eorl, *eoud, *cp, *coud, *corl, *cofb, *cpos;
} CubeArray;

extern PieceFilter pf_all, pf_cpos, pf_ep, pf_cp, 
                   pf_e, pf_s, pf_m, pf_eo, pf_co;

void cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f);
Cube arrays_to_cube(CubeArray arr, PieceFilter f);

/* piece can be edge or corner and orientation is any of the following:
   "eofb", "eorl", "eoud", "coud", "corl", "cofb"
   Return either 0 (oriented) or 1 for edges and 0, 1 or 2 for corners */
int piece_orientation(Cube cube, int piece, char *orientation);
Center center_at(Cube cube, Center c);
Edge   edge_at(Cube cube, Edge e);
Corner corner_at(Cube cube, Corner c);
bool block_solved(Cube cube, Block);
/* Aggiungi funzioni per "queries" sul cubo: se pezzo è orientato rispetto ad
   un certo asse, se il pezzo è risolto... */
/* Would be nice: a funciton block_solved(Cube c, Block b), where Block is
   something like struct {bool centers[6], edges[12], corners[8]}
   (The advantage over checking pieces one by one is that I can convert
   to cubearray only once and for all) */
/* Altro TODO, ma forse non ne vale la pena: pre-calcolare tutti i possibili
   valori per questi, e salvare i risultati in array (facile per cp e cpos,
   mentre per ep bisogna anche cercare quale tra epose, eposs e eposm contiene
   il valore giusto) */

bool equal(Cube c1, Cube c2);
bool is_solved(Cube cube);
void print_cube(Cube cube);
Cube admissible_ep(Cube cube, PieceFilter f); /* Returns admissible ep */
Cube inverse_cube(Cube cube);
Cube compose(Cube c2, Cube c1); /* Use c2 as an alg on c1 */
Cube compose_filtered(Cube c2, Cube c1, PieceFilter f);
Cube move_via_arrays(CubeArray arr, Cube c, PieceFilter pf);

#endif
