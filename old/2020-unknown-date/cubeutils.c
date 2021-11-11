#include <stdio.h>
#include "utils.h"
#include "pieces.h"
#include "cubeutils.h"

int epe_solved[] = {FR, FL, BL, BR};
int eps_solved[] = {UL, UR, DL, DR};
int epm_solved[] = {UF, UB, DF, DB};

void cube_to_ep_array(Cube *cube, int ep[12]) {
  int epe[4], eps[4], epm[4];
  index_to_perm(cube->epose % factorial(4), 4, epe);
  index_to_perm(cube->eposs % factorial(4), 4, eps);
  index_to_perm(cube->eposm % factorial(4), 4, epm);

  int epose[12], eposs[12], eposm[12];
  index_to_subset(cube->epose / factorial(4), 12, 4, epose);
  index_to_subset(cube->eposs / factorial(4), 12, 4, eposs);
  index_to_subset(cube->eposm / factorial(4), 12, 4, eposm);
  for (int i = 0; i < 4; i++) {
    swap(&eposs[eps_solved[i]], &eposs[i+8]);
    swap(&eposm[epm_solved[i]], &eposm[i+8]);
  }

  for (int i = 0, ie = 0, is = 0, im = 0; i < 12; i++) {
    if (epose[i]) ep[i] = epe_solved[epe[ie++]]; 
    if (eposs[i]) ep[i] = eps_solved[eps[is++]]; 
    if (eposm[i]) ep[i] = epm_solved[epm[im++]]; 
  }
}

void ep_array_to_epos(int ep[12], Cube *cube) {
  int epe[4], eps[4], epm[4];
  int epose[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int eposs[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int eposm[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  for (int i = 0, ie = 0, is = 0, im = 0; i < 12; i++) {
    for (int j = 0; j < 4; j++) {
      if (ep[i] == epe_solved[j]) { epe[ie++] = j; epose[i] = 1; }
      if (ep[i] == eps_solved[j]) { eps[is++] = j; eposs[i] = 1; }
      if (ep[i] == epm_solved[j]) { epm[im++] = j; eposm[i] = 1; }
    }
  }
  for (int i = 0; i < 4; i++) {
    swap(&eposs[eps_solved[i]], &eposs[i+8]);
    swap(&eposm[epm_solved[i]], &eposm[i+8]);
  }
  cube->epose = factorial(4) * subset_to_index(epose, 12, 4)
                             + perm_to_index(epe, 4);
  cube->eposs = factorial(4) * subset_to_index(eposs, 12, 4)
                             + perm_to_index(eps, 4);
  cube->eposm = factorial(4) * subset_to_index(eposm, 12, 4)
                             + perm_to_index(epm, 4);
}

bool solvable(Cube *cube) {
  /* Since we memorize orientation truncating the last digit, we only need to
   * check that the permutations have the correct sign. */
  /* TODO: add check that every integer is in range */
  int ep[12], cp[12], c[6];
  cube_to_ep_array(cube, ep);
  index_to_perm(cube->cp, 8, cp);
  index_to_perm(cube->centerpos, 6, c);
  return (perm_sign(ep, 12) ^ perm_sign(c, 6)) == perm_sign(cp, 8);
}

Cube *solved_cube() {
  static Cube cube = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  return &cube;
}

bool solved(Cube *cube) {
  return (cube->eofb == 0 && cube->coud == 0 && cube->cp == 0 &&
          cube->epose == 0 && cube->eposs == 0 && cube->eposm == 0 &&
          cube->centerpos == 0);
}

char *to_string(Cube *cube) {
  int eo[12], co[8], ep[12], cp[8], cenpos[6];
  cube_to_eofb_array(cube->eofb, eo);
  cube_to_coud_array(cube->coud, co);
  cube_to_ep_array(cube, ep);
  cube_to_cp_array(cube->cp, cp);
  cube_to_centerpos_array(cube->centerpos, cenpos);

  char *ret[1000];

  for (int i = 0; i < 12; i++) sprintf(ret, " %s ", edge_string[ep[i]]);
  sprintf(ret, "\n");
  for (int i = 0; i < 12; i++) sprintf(ret, "  %d ", eo[i]);
  sprintf(ret, "\n");
  for (int i = 0; i < 8; i++) sprintf(ret, "%s ", corner_string[cp[i]]);
  sprintf(ret, "\n");
  for (int i = 0; i < 8; i++) sprintf(ret, "  %d ", co[i]);
  sprintf(ret, "\n");
  for (int i = 0; i < 6; i++) sprintf(ret, "  %s ", center_string[cenpos[i]]);
  sprintf(ret, "\n");

  return ret;
}
