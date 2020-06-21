/* This is a simple program to solve the Rubik's Cube.
 * No idea how many features I am going to implement.
 * Open source license and whatnot.
 * I am trying to follow the C99 standard. */

/* This file contains the definitions of the basic moves of the cube.
 * There is no object or type representing the cube.
 * Data about the cube can be represented by arrays (describing the position
 * of pieces of certain types), integers (representing for example a bitmask
 * for the orientation of pieces of certain type, or the permutation index of
 * an array representing the permutation of pieces).
 * Each of the moves functions operates on one such piece of data. 
 *
 * For example, a way of representing the cube can be:
 *  - An integer eo, which is a bitmask for the orientation of the edges.
 *  - An integer co, same for corners.
 *  - An array ep[12], where a[i]=j means that the edge j is in place i.
 *  - An integer cp representing the permutation index of a permutation array
 *    which is the analogue of that described for edges.
 *
 * Different representations will be used for different use-cases. */

#include "coordinates.h"
#include "moves.h"

/* possible_next[i][j] is a bitmask representing the possible
 * next moves we can apply. For example, if the last moves a 0 R then it does
 * not make sense to apply R, R2 or R'. If they are U D2 it does not make
 * sense to apply any U* or D*. */
int possible_next[19][19];

int parallel(int m1, int m2) {
  if (m1 == 0 || m2 == 0) return 0;
  return ((m1-1)/6 == (m2-1)/6);
}

int compute_possible_next(int last1, int last2) {
  if (last1 == 0) return move_mask_all;

  /* Removes the 2 or ' (e.g. turns U2 to U, R' to R). */
  last2 = (last2 == 0) ? last2 : 3*((last2-1)/3) + 1;
  last1 = 3*((last1-1)/3) + 1;

  int mask = move_mask_all ^ (7 << last1);

  if (parallel(last1, last2))
    mask ^= 7 << last2;
  else if (last1 % 6 == 4) /*Always U before D, R before L, F before B*/
    mask ^= 7 << (last1-3);

  return mask;
}

void init_possible_next() {
  for (int i = 0; i < 19; i++)
    for (int j = 0; j < 19; j++)
      possible_next[i][j] = compute_possible_next(i, j);
}

/* Piece cycles depending on the move. For example edge_cycle[U2][UF]
 * gives the piece in position UF after applying U2 to a solved cube */

int edge_cycle[19][12] = {
  {UF, UL, UB, UR, DF, DL, DB, DR, FR, FL, BL, BR}, /* -  */
  {UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR}, /* U  */
  {UB, UR, UF, UL, DF, DL, DB, DR, FR, FL, BL, BR}, /* U2 */
  {UL, UB, UR, UF, DF, DL, DB, DR, FR, FL, BL, BR}, /* U' */
  {UF, UL, UB, UR, DL, DB, DR, DF, FR, FL, BL, BR}, /* D  */
  {UF, UL, UB, UR, DB, DR, DF, DL, FR, FL, BL, BR}, /* D2 */
  {UF, UL, UB, UR, DR, DF, DL, DB, FR, FL, BL, BR}, /* D' */
  {UF, UL, UB, FR, DF, DL, DB, BR, DR, FL, BL, UR}, /* R  */
  {UF, UL, UB, DR, DF, DL, DB, UR, BR, FL, BL, FR}, /* R2 */
  {UF, UL, UB, BR, DF, DL, DB, FR, UR, FL, BL, DR}, /* R' */
  {UF, BL, UB, UR, DF, FL, DB, DR, FR, UL, DL, BR}, /* L  */
  {UF, DL, UB, UR, DF, UL, DB, DR, FR, BL, FL, BR}, /* L2 */
  {UF, FL, UB, UR, DF, BL, DB, DR, FR, DL, UL, BR}, /* L' */
  {FL, UL, UB, UR, FR, DL, DB, DR, UF, DF, BL, BR}, /* F  */
  {DF, UL, UB, UR, UF, DL, DB, DR, FL, FR, BL, BR}, /* F2 */
  {FR, UL, UB, UR, FL, DL, DB, DR, DF, UF, BL, BR}, /* F' */
  {UF, UL, BR, UR, DF, DL, BL, DR, FR, FL, UB, DB}, /* B  */
  {UF, UL, DB, UR, DF, DL, UB, DR, FR, FL, BR, BL}, /* B2 */
  {UF, UL, BL, UR, DF, DL, BR, DR, FR, FL, DB, UB}  /* B' */
};

int corner_cycle[19][8] = {
  {UFR, UFL, UBL, UBR, DFR, DFL, DBL, DBR}, /* -  */
  {UBR, UFR, UFL, UBL, DFR, DFL, DBL, DBR}, /* U  */
  {UBL, UBR, UFR, UFL, DFR, DFL, DBL, DBR}, /* U2 */
  {UFL, UBL, UBR, UFR, DFR, DFL, DBL, DBR}, /* U' */
  {UFR, UFL, UBL, UBR, DFL, DBL, DBR, DFR}, /* D  */
  {UFR, UFL, UBL, UBR, DBL, DBR, DFR, DFL}, /* D2 */
  {UFR, UFL, UBL, UBR, DBR, DFR, DFL, DBL}, /* D' */
  {DFR, UFL, UBL, UFR, DBR, DFL, DBL, UBR}, /* R  */
  {DBR, UFL, UBL, DFR, UBR, DFL, DBL, UFR}, /* R2 */
  {UBR, UFL, UBL, DBR, UFR, DFL, DBL, DFR}, /* R' */
  {UFR, UBL, DBL, UBR, DFR, UFL, DFL, DBR}, /* L  */
  {UFR, DBL, DFL, UBR, DFR, UBL, UFL, DBR}, /* L2 */
  {UFR, DFL, UFL, UBR, DFR, DBL, UBL, DBR}, /* L' */
  {UFL, DFL, UBL, UBR, UFR, DFR, DBL, DBR}, /* F  */
  {DFL, DFR, UBL, UBR, UFL, UFR, DBL, DBR}, /* F2 */
  {DFR, UFR, UBL, UBR, DFL, UFL, DBL, DBR}, /* F' */
  {UFR, UFL, UBR, DBR, DFR, DFL, UBL, DBL}, /* B  */
  {UFR, UFL, DBR, DBL, DFR, DFL, UBR, UBL}, /* B2 */
  {UFR, UFL, DBL, UBL, DFR, DFL, DBR, UBR}, /* U' */
};

/* Transition tables */

int eofb_transition_table[pow2to11][19];
int eorl_transition_table[pow2to11][19];
int eoud_transition_table[pow2to11][19];
int coud_transition_table[pow3to7][19];
int cofb_transition_table[pow3to7][19];
int corl_transition_table[pow3to7][19];
int epud_transition_table[factorial8][19];
int eprl_transition_table[factorial8][19];
int epfb_transition_table[factorial8][19];
int epose_transition_table[binom12on4][19];
int eposs_transition_table[binom12on4][19];
int eposm_transition_table[binom12on4][19];
int epe_transition_table[factorial4][19];
int eps_transition_table[factorial4][19];
int epm_transition_table[factorial4][19];
int emslices_transition_table[binom12on4*binom8on4][19];
int cp_transition_table[factorial8][19];

/***/
/* Functions for permuting pieces (given in array format) */
/***/

void apply_move_ep_array(int move, int ep[12]) {
  int aux[12];
  for (int i = 0; i < 12; i++)
    aux[i] = ep[i];
  for (int i = 0; i < 12; i++)
    ep[i] = aux[edge_cycle[move][i]];
}

void apply_move_cp_array(int move, int cp[8]) {
  int aux[8];
  for (int i = 0; i < 8; i++)
    aux[i] = cp[i];
  for (int i = 0; i < 8; i++)
    cp[i] = aux[corner_cycle[move][i]];
}

/***/
/* Functions for permuting pieces (given in integer format) */
/***/

int apply_move_ep_int(int move, int ep) {
  int a[12];
  ep_int_to_array(ep, a);
  apply_move_ep_array(move, a);
  return ep_array_to_int(a);
}

int apply_move_epud_int(int move, int ep) {
  int a[12];
  epud_int_to_array(ep, a);
  apply_move_ep_array(move, a);
  return epud_array_to_int(a);
}

int apply_move_eprl_int(int move, int ep) {
  int a[12];
  eprl_int_to_array(ep, a);
  apply_move_ep_array(move, a);
  return eprl_array_to_int(a);
}

int apply_move_epfb_int(int move, int ep) {
  int a[12];
  epfb_int_to_array(ep, a);
  apply_move_ep_array(move, a);
  return epfb_array_to_int(a);
}

int apply_move_epose_int(int move, int ep) {
  int a[12];
  epose_int_to_array(ep, a);
  apply_move_ep_array(move, a);
  return epose_array_to_int(a);
}

int apply_move_eposs_int(int move, int ep) {
  int a[12];
  eposs_int_to_array(ep, a);
  apply_move_ep_array(move, a);
  return eposs_array_to_int(a);
}

int apply_move_eposm_int(int move, int ep) {
  int a[12];
  eposm_int_to_array(ep, a);
  apply_move_ep_array(move, a);
  return eposm_array_to_int(a);
}

int apply_move_epe_int(int move, int ep) {
  int a[12];
  epe_int_to_array(ep, a);
  apply_move_ep_array(move, a);
  return epe_array_to_int(a);
}

int apply_move_eps_int(int move, int ep) {
  int a[12];
  eps_int_to_array(ep, a);
  apply_move_ep_array(move, a);
  return eps_array_to_int(a);
}

int apply_move_epm_int(int move, int ep) {
  int a[12];
  epm_int_to_array(ep, a);
  apply_move_ep_array(move, a);
  return epm_array_to_int(a);
}

int apply_move_emslices_int(int move, int e) {
  int a[12];
  emslices_int_to_array(e, a);
  apply_move_ep_array(move, a);
  return emslices_array_to_int(a);
}

int apply_move_cp_int(int move, int cp) {
  int a[8];
  cp_int_to_array(cp, a);
  apply_move_cp_array(move, a);
  return cp_array_to_int(a);
}

int apply_move_eofb_int(int move, int eo) {
  int a[12];
  eo_11bits_to_array(eo, a);
  apply_move_ep_array(move, a);
  /* Change edge orientation */
  if (move == F || move == F3) { 
    a[UF] = 1 - a[UF];
    a[DF] = 1 - a[DF];
    a[FR] = 1 - a[FR];
    a[FL] = 1 - a[FL];
  }
  if (move == B || move == B3) {  
    a[UB]  = 1 - a[UB];
    a[DB]  = 1 - a[DB];
    a[BL]  = 1 - a[BL];
    a[BR]  = 1 - a[BR];
  }
  return eo_array_to_11bits(a);
}

int apply_move_eorl_int(int move, int eo) {
  int a[12];
  eo_11bits_to_array(eo, a);
  apply_move_ep_array(move, a);
  /* Change edge orientation */
  if (move == R || move == R3) { 
    a[UR] = 1 - a[UR];
    a[DR] = 1 - a[DR];
    a[FR] = 1 - a[FR];
    a[BR] = 1 - a[BR];
  }
  if (move == L || move == L3) {  
    a[UL]  = 1 - a[UL];
    a[DL]  = 1 - a[DL];
    a[FL]  = 1 - a[FL];
    a[BL]  = 1 - a[BL];
  }
  return eo_array_to_11bits(a);
}

int apply_move_eoud_int(int move, int eo) {
  int a[12];
  eo_11bits_to_array(eo, a);
  apply_move_ep_array(move, a);
  /* Change edge orientation */
  if (move == U || move == U3) { 
    a[UF] = 1 - a[UF];
    a[UL] = 1 - a[UL];
    a[UB] = 1 - a[UB];
    a[UR] = 1 - a[UR];
  }
  if (move == D || move == D3) {  
    a[DF]  = 1 - a[DF];
    a[DL]  = 1 - a[DL];
    a[DB]  = 1 - a[DB];
    a[DR]  = 1 - a[DR];
  }
  return eo_array_to_11bits(a);
}

int apply_move_coud_int(int move, int co) {
  int a[8];
  co_7trits_to_array(co, a);
  apply_move_cp_array(move, a);
  /* Change corner orientation */
  if (move == R || move == R3) {
    a[UFR] = (a[UFR] + 2) % 3;
    a[UBR] = (a[UBR] + 1) % 3;
    a[DBR] = (a[DBR] + 2) % 3;
    a[DFR] = (a[DFR] + 1) % 3;
  }
  if (move == L || move == L3) {
    a[UBL] = (a[UBL] + 2) % 3;
    a[UFL] = (a[UFL] + 1) % 3;
    a[DFL] = (a[DFL] + 2) % 3;
    a[DBL] = (a[DBL] + 1) % 3;
  }
  if (move == F || move == F3) {
    a[UFL] = (a[UFL] + 2) % 3;
    a[UFR] = (a[UFR] + 1) % 3;
    a[DFR] = (a[DFR] + 2) % 3;
    a[DFL] = (a[DFL] + 1) % 3;
  }
  if (move == B || move == B3) {
    a[UBR] = (a[UBR] + 2) % 3;
    a[UBL] = (a[UBL] + 1) % 3;
    a[DBL] = (a[DBL] + 2) % 3;
    a[DBR] = (a[DBR] + 1) % 3;
  }
  return co_array_to_7trits(a);
}

int apply_move_cofb_int(int move, int co) {
  int a[8];
  co_7trits_to_array(co, a);
  apply_move_cp_array(move, a);
  /* Change corner orientation */
  if (move == R || move == R3) {
    a[UFR] = (a[UFR] + 1) % 3;
    a[UBR] = (a[UBR] + 2) % 3;
    a[DBR] = (a[DBR] + 1) % 3;
    a[DFR] = (a[DFR] + 2) % 3;
  }
  if (move == L || move == L3) {
    a[UBL] = (a[UBL] + 1) % 3;
    a[UFL] = (a[UFL] + 2) % 3;
    a[DFL] = (a[DFL] + 1) % 3;
    a[DBL] = (a[DBL] + 2) % 3;
  }
  if (move == U || move == U3) {
    a[UFL] = (a[UFL] + 1) % 3;
    a[UFR] = (a[UFR] + 2) % 3;
    a[UBL] = (a[UBL] + 2) % 3;
    a[UBR] = (a[UBR] + 1) % 3;
  }
  if (move == D || move == D3) {
    a[DFL] = (a[DFL] + 2) % 3;
    a[DFR] = (a[DFR] + 1) % 3;
    a[DBL] = (a[DBL] + 1) % 3;
    a[DBR] = (a[DBR] + 2) % 3;
  }
  return co_array_to_7trits(a);
}

int apply_move_corl_int(int move, int co) {
  int a[8];
  co_7trits_to_array(co, a);
  apply_move_cp_array(move, a);
  /* Change corner orientation */
  if (move == F || move == F3) {
    a[UFR] = (a[UFR] + 2) % 3;
    a[UFL] = (a[UFL] + 1) % 3;
    a[DFL] = (a[DFL] + 2) % 3;
    a[DFR] = (a[DFR] + 1) % 3;
  }
  if (move == B || move == B3) {
    a[UBL] = (a[UBL] + 2) % 3;
    a[UBR] = (a[UBR] + 1) % 3;
    a[DBR] = (a[DBR] + 2) % 3;
    a[DBL] = (a[DBL] + 1) % 3;
  }
  if (move == U || move == U3) {
    a[UFL] = (a[UFL] + 2) % 3;
    a[UFR] = (a[UFR] + 1) % 3;
    a[UBL] = (a[UBL] + 1) % 3;
    a[UBR] = (a[UBR] + 2) % 3;
  }
  if (move == D || move == D3) {
    a[DFL] = (a[DFL] + 1) % 3;
    a[DFR] = (a[DFR] + 2) % 3;
    a[DBL] = (a[DBL] + 2) % 3;
    a[DBR] = (a[DBR] + 1) % 3;
  }
  return co_array_to_7trits(a);
}
 


/* Initialize transition tables */

void init_epud_transition_table() {
  for (int i = 0; i < factorial8; i++)
    for (int j = 0; j < 19; j++)
      if (move_mask_drud & (1 << j))
        epud_transition_table[i][j] = apply_move_epud_int(j, i);
}

void init_eprl_transition_table() {
  for (int i = 0; i < factorial8; i++)
    for (int j = 0; j < 19; j++)
      if (move_mask_drrl & (1 << j))
        eprl_transition_table[i][j] = apply_move_eprl_int(j, i);
}

void init_epfb_transition_table() {
  for (int i = 0; i < factorial8; i++)
    for (int j = 0; j < 19; j++)
      if (move_mask_drfb & (1 << j))
        epfb_transition_table[i][j] = apply_move_epfb_int(j, i);
}

void init_epose_transition_table() {
  for (int i = 0; i < binom12on4; i++)
    for (int j = 0; j < 19; j++)
      epose_transition_table[i][j] = apply_move_epose_int(j, i);
}

void init_eposs_transition_table() {
  for (int i = 0; i < binom12on4; i++)
    for (int j = 0; j < 19; j++)
      eposs_transition_table[i][j] = apply_move_eposs_int(j, i);
}

void init_eposm_transition_table() {
  for (int i = 0; i < binom12on4; i++)
    for (int j = 0; j < 19; j++)
      eposm_transition_table[i][j] = apply_move_eposm_int(j, i);
}

void init_epe_transition_table() {
  for (int i = 0; i < factorial4; i++) {
    for (int j = 0; j < 19; j++)
      if (move_mask_drud & (1 << j))
        epe_transition_table[i][j] = apply_move_epe_int(j, i);
  }
}

void init_eps_transition_table() {
  for (int i = 0; i < factorial4; i++) {
    for (int j = 0; j < 19; j++)
      if (move_mask_drfb & (1 << j))
        eps_transition_table[i][j] = apply_move_eps_int(j, i);
  }
}

void init_epm_transition_table() {
  for (int i = 0; i < factorial4; i++) {
    for (int j = 0; j < 19; j++)
      if (move_mask_drrl & (1 << j))
        epm_transition_table[i][j] = apply_move_epm_int(j, i);
  }
}

void init_emslices_transition_table() {
  for (int i = 0; i < binom12on4*binom8on4; i++) {
    for (int j = 0; j < 19; j++)
      emslices_transition_table[i][j] = apply_move_emslices_int(j, i);
  }
}

void init_cp_transition_table() {
  for (int i = 0; i < factorial8; i++)
    for (int j = 0; j < 19; j++)
      cp_transition_table[i][j] = apply_move_cp_int(j, i);
}

void init_eofb_transition_table() {
  for (int i = 0; i < pow2to11; i++)
    for (int j = 0; j < 19; j++)
      eofb_transition_table[i][j] = apply_move_eofb_int(j, i);
}

void init_eorl_transition_table() {
  for (int i = 0; i < pow2to11; i++)
    for (int j = 0; j < 19; j++)
      eorl_transition_table[i][j] = apply_move_eorl_int(j, i);
}

void init_eoud_transition_table() {
  for (int i = 0; i < pow2to11; i++)
    for (int j = 0; j < 19; j++)
      eoud_transition_table[i][j] = apply_move_eoud_int(j, i);
}

void init_coud_transition_table() {
  for (int i = 0; i < pow3to7; i++)
    for (int j = 0; j < 19; j++ )
      coud_transition_table[i][j] = apply_move_coud_int(j, i);
}

void init_cofb_transition_table() {
  for (int i = 0; i < pow3to7; i++)
    for (int j = 0; j < 19; j++ )
      cofb_transition_table[i][j] = apply_move_cofb_int(j, i);
}

void init_corl_transition_table() {
  for (int i = 0; i < pow3to7; i++)
    for (int j = 0; j < 19; j++ )
      corl_transition_table[i][j] = apply_move_corl_int(j, i);
}

void init_transition_table() {
  init_epud_transition_table();
  init_eprl_transition_table();
  init_epfb_transition_table();
  init_epose_transition_table();
  init_eposs_transition_table();
  init_eposm_transition_table();
  init_epe_transition_table();
  init_eps_transition_table();
  init_epm_transition_table();
  init_emslices_transition_table();
  init_cp_transition_table();
  init_eofb_transition_table();
  init_eorl_transition_table();
  init_eoud_transition_table();
  init_coud_transition_table();
  init_cofb_transition_table();
  init_corl_transition_table();
}

