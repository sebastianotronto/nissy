/* Moves and other things */

#ifndef PIECES_H
#define PIECES_H

typedef enum {
  U, U2, U3, D, D2, D3, R, R2, R3, L, L2, L3, F, F2, F3, B, B2, B3,
  Uw, Uw2, Uw3, Dw, Dw2, Dw3, Rw, Rw2, Rw3,
  Lw, Lw2, Lw3, Fw, Fw2, Fw3, Bw, Bw2, Bw3,
  M, M2, M3, S, S2, S3, E, E2, E3,
  X, X2, X3, Y, Y2, Y3, Z, Z2, Z3,
  NULLMOVE
} Move;
typedef enum {
  U_center, D_center, R_center, L_center, F_center, B_center
} Center;
typedef enum { UF, UL, UB, UR, DF, DL, DB, DR, FR, FL, BL, BR } Edge;
typedef enum { UFR, UFL, UBL, UBR, DFR, DFL, DBL, DBR } Corner;

/* Arrays for strings of pieces and moves */
extern char edge_string[12][5];
extern char corner_string[8][5];
extern char center_string[6][5];
extern char move_string[NULLMOVE+1][5];

/* Three big arrays that determine the movement of edges, corners and
 * centers for some of the basic moves. Yes, one can be much more elegant and
 * write much fewer lines. But then things get less elegant somewhere else. */
extern int edge_cycle[NULLMOVE+1][12];
extern int corner_cycle[NULLMOVE+1][8];
extern int center_cycle[NULLMOVE+1][6];

/* Representation of the cube */
typedef struct {
  /* Edge orientation */
  int eofb;
  int eorl;
  int eoud;
  
  /* Corner orientation */
  int coud;
  int cofb;
  int corl;

  /* Position of M/E/S slice edges */
  /* TODO: keep in mind that the 4! positions with edges in correct slice
   * must be the first 4! indeces */
  int epose;
  int eposs;
  int eposm;

  /* Permutation of corners */
  int cp;

  /* Position of centers */
  int centerpos;
} Cube;

#endif
