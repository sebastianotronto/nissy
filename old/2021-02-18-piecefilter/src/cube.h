#ifndef CUBE_H
#define CUBE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "utils.h"

typedef enum {U_center,D_center,R_center,L_center,F_center,B_center} Center;
typedef enum { UF, UL, UB, UR, DF, DL, DB, DR, FR, FL, BL, BR } Edge;
typedef enum { UFR, UFL, UBL, UBR, DFR, DFL, DBL, DBR } Corner;

typedef struct {
  bool epose, eposs, eposm, eofb, eorl, eoud, cp, coud, cofb, corl, cpos;
} PieceFilter;

typedef struct {
  uint16_t eofb, eorl, eoud, coud, cofb, corl,
           epose, eposs, eposm, cp, cpos;
} Cube;

typedef struct {
  int ep[12], eofb[12], eorl[12], eoud[12],
      cp[8],  coud[8],  corl[8],  cofb[8],  cpos[6];
} CubeArray;


extern PieceFilter fAll;

Cube blank_cube();
/* Return axis (ud=0, rl=1, fb=0) of center c */
int center_axis(int c);
/* Return slice (e=0, s=1, m=2) to which e belongs */
int edge_slice(int e);
void cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f);
Cube arrays_to_cube(CubeArray arr, PieceFilter f);
bool equal(Cube c1, Cube c2);
bool is_solvable(Cube cube);
bool is_solved(Cube cube);
void print_cube(Cube cube);
Cube inverse_cube(Cube cube);
/* Use c2 as an alg on c1 */
Cube compose_via_arrays(CubeArray c2, Cube c1, PieceFilter f);

#endif
