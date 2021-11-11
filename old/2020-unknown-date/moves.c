#include <stdbool.h>
#include "pieces.h"
#include "moves.h"
#include "utils.h"

/* Transition tables */
int eofb_ttable[pow2to11][NULLMOVE];
int eorl_ttable[pow2to11][NULLMOVE];
int eoud_ttable[pow2to11][NULLMOVE];
int coud_ttable[pow3to7][NULLMOVE];
int cofb_ttable[pow3to7][NULLMOVE];
int corl_ttable[pow3to7][NULLMOVE];
int epose_ttable[factorial12/factorial8][NULLMOVE];
int eposs_ttable[factorial12/factorial8][NULLMOVE];
int eposm_ttable[factorial12/factorial8][NULLMOVE];
int cp_ttable[factorial8][NULLMOVE];
int centerpos_ttable[factorial6][NULLMOVE];

void apply_move_ep(Move m, int a[12]) {
  int aux[12];
  intarrcopy(a, aux, 12);
  for (int i = 0; i < 12; i++)
    a[i] = aux[edge_cycle[m][i]];
}

void apply_move_cp(Move m, int a[8]) {
  int aux[8];
  intarrcopy(a, aux, 8);
  for (int i = 0; i < 8; i++)
    a[i] = aux[corner_cycle[m][i]];
}

void apply_move_centerpos(Move m, int a[6]) {
  int aux[6];
  intarrcopy(a, aux, 6);
  for (int i = 0; i < 6; i++)
    a[i] = aux[center_cycle[m][i]];
}

void init_ttables() {
  /* TODO */
}

void apply_move(Move m, Cube *cube) {
  cube->eofb = eofb_ttable[cube->eofb][m];
  cube->eorl = eorl_ttable[cube->eorl][m];
  cube->eoud = eoud_ttable[cube->eoud][m];
  cube->coud = coud_ttable[cube->coud][m];
  cube->cofb = cofb_ttable[cube->cofb][m];
  cube->corl = corl_ttable[cube->corl][m];
  cube->epose = epose_ttable[cube->epose][m];
  cube->eposs = eposs_ttable[cube->eposs][m];
  cube->eposm = eposm_ttable[cube->eposm][m];
  cube->cp = cp_ttable[cube->cp][m];
  cube->centerpos = centerpos_ttable[cube->centerpos][m];
}

