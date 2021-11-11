#include "cube.h"

char edge_string[12][5] =
  { "UF", "UL", "UB", "UR", "DF", "DL", "DB", "DR", "FR", "FL", "BL", "BR" };
char corner_string[8][5] = { "UFR","UFL","UBL","UBR","DFR","DFL","DBL","DBR" };
char center_string[6][5] = { "U", "D", "R", "L", "F", "B" };
    
int epe_solved[] = {FR, FL, BL, BR};
int eps_solved[] = {UL, UR, DL, DR};
int epm_solved[] = {UF, UB, DF, DB};

PieceFilter fAll = {true,true,true,true,true,true,true,true,true,true,true};

Cube blank_cube() {
  Cube c = {0};
  return c;
}

/* Return axis (ud=0, rl=1, fb=0) of center c */
int center_axis(int c) {
  if (c == U_center || c == D_center)
    return 0;
  if (c == R_center || c == L_center)
    return 1;
  return 2;
}

/* Return slice (e=0, s=1, m=2) to which e belongs */
int edge_slice(int e) {
  if (e == FR || e == FL || e == BL || e == BR)
    return 0;
  if (e == UR || e == UL || e == DR || e == DL)
    return 1;
  return 2;
}

void cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f) {
  /* ep is the hardest */
  if (f.epose || f.eposs || f.eposm)
    for (int i = 0; i < 12; i++) arr->ep[i] = -1;
  if (f.epose) {
    int epe[4], epose[12];
    index_to_perm(cube.epose % factorial(4), 4, epe);
    index_to_subset(cube.epose / factorial(4), 12, 4, epose);
    for (int i = 0, ie = 0; i < 12; i++)
      if (epose[i]) arr->ep[i] = epe_solved[epe[ie++]];
  }
  if (f.eposs) {
    int eps[4], eposs[12];
    index_to_perm(cube.eposs % factorial(4), 4, eps);
    index_to_subset(cube.eposs / factorial(4), 12, 4, eposs);
    for (int i = 0; i < 4; i++) swap(&eposs[eps_solved[i]], &eposs[i+8]);
    for (int i = 0, is = 0; i < 12; i++)
      if (eposs[i]) arr->ep[i] = eps_solved[eps[is++]]; 
  }
  if (f.eposm) {
    int epm[4], eposm[12];
    index_to_perm(cube.eposm % factorial(4), 4, epm);
    index_to_subset(cube.eposm / factorial(4), 12, 4, eposm);
    for (int i = 0; i < 4; i++) swap(&eposm[epm_solved[i]], &eposm[i+8]);
    for (int i = 0, im = 0; i < 12; i++)
      if (eposm[i]) arr->ep[i] = epm_solved[epm[im++]]; 
  }

  /* All the others */
  if (f.eofb) int_to_sum_zero_array(cube.eofb, 2, 12, arr->eofb);
  if (f.eorl) int_to_sum_zero_array(cube.eorl, 2, 12, arr->eorl);
  if (f.eoud) int_to_sum_zero_array(cube.eoud, 2, 12, arr->eoud);
  if (f.cp)   index_to_perm(        cube.cp,   8,     arr->cp);
  if (f.coud) int_to_sum_zero_array(cube.coud, 3, 8,  arr->coud);
  if (f.corl) int_to_sum_zero_array(cube.corl, 3, 8,  arr->corl);
  if (f.cofb) int_to_sum_zero_array(cube.cofb, 3, 8,  arr->cofb);
  if (f.cpos) index_to_perm(        cube.cpos, 6,     arr->cpos);
}

Cube arrays_to_cube(CubeArray arr, PieceFilter f) {
  Cube ret = {0};

  /* Again, ep is the hardest part */
  if (f.epose) {
    int epe[4], epose[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    for (int i = 0, ie = 0; i < 12; i++)
      for (int j = 0; j < 4; j++)
        if (arr.ep[i] == epe_solved[j])
          { epe[ie++] = j; epose[i] = 1; }
    ret.epose = factorial(4)*subset_to_index(epose,12,4)+perm_to_index(epe,4);
  }
  if (f.eposs) {
    int eps[4], eposs[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    for (int i = 0, is = 0; i < 12; i++)
      for (int j = 0; j < 4; j++)
        if (arr.ep[i] == eps_solved[j])
          { eps[is++] = j; eposs[i] = 1; }
    for (int i = 0; i < 4; i++) swap(&eposs[eps_solved[i]], &eposs[i+8]);
    ret.eposs = factorial(4)*subset_to_index(eposs,12,4)+perm_to_index(eps,4);
  }
  if (f.eposm) {
    int epm[4], eposm[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    for (int i = 0, im = 0; i < 12; i++)
      for (int j = 0; j < 4; j++)
        if (arr.ep[i] == epm_solved[j])
          { epm[im++] = j; eposm[i] = 1; }
    for (int i = 0; i < 4; i++) swap(&eposm[epm_solved[i]], &eposm[i+8]);
    ret.eposm = factorial(4)*subset_to_index(eposm,12,4)+perm_to_index(epm,4);
  }
  if (f.eofb) ret.eofb = digit_array_to_int(arr.eofb, 11, 2);
  if (f.eorl) ret.eorl = digit_array_to_int(arr.eorl, 11, 2);
  if (f.eoud) ret.eoud = digit_array_to_int(arr.eoud, 11, 2);
  if (f.cp)   ret.cp   = perm_to_index(     arr.cp,   8    );
  if (f.coud) ret.coud = digit_array_to_int(arr.coud, 7,  3);
  if (f.corl) ret.corl = digit_array_to_int(arr.corl, 7,  3);
  if (f.cofb) ret.cofb = digit_array_to_int(arr.cofb, 7,  3);
  if (f.cpos) ret.cpos = perm_to_index(     arr.cpos, 6    );

  return ret;
}

bool equal(Cube c1, Cube c2) {
  return c1.eofb   == c2.eofb  && c1.epose == c2.epose && 
         c1.eposs  == c2.eposs && c1.eposm == c2.eposm &&
         c1.coud   == c2.coud  && c1.cp    == c2.cp    &&
         c1.cpos   == c2.cpos;
}

bool solvable(Cube cube) {
  /* Since we memorize orientation truncating the last digit, we only need to
   * check that the permutations have the correct sign. */
  CubeArray arr = {0};
  cube_to_arrays(cube, &arr, fAll);
  return (perm_sign(arr.ep,12) ^ perm_sign(arr.cpos,6)) == perm_sign(arr.cp,8);
}

bool is_solved(Cube cube) {
  return !cube.eofb  && !cube.coud  && !cube.cp    &&
         !cube.epose && !cube.eposs && !cube.eposm && cube.cpos;
}

void print_cube(Cube cube) {
  CubeArray arr = {0};
  cube_to_arrays(cube, &arr, fAll);

  for (int i = 0; i < 12; i++) printf(" %s ",  edge_string[arr.ep[i]]);
  printf("\n");
  for (int i = 0; i < 12; i++) printf("  %c ", arr.eofb[i] + '0');
  printf("\n");
  for (int i = 0; i < 8; i++)  printf("%s ",   corner_string[arr.cp[i]]);
  printf("\n");
  for (int i = 0; i < 8; i++)  printf("  %c ", arr.coud[i] + '0');
  printf("\n");
  for (int i = 0; i < 6; i++)  printf("  %s ", center_string[arr.cpos[i]]);
  printf("\n");
}
  
Cube inverse_cube(Cube cube) {
  CubeArray arr = {0}, inv = {0};
  cube_to_arrays(cube, &arr, fAll);

  for (int i = 0; i < 12; i++) {
    inv.ep[arr.ep[i]] = i;
    inv.eofb[arr.ep[i]] = arr.eofb[i];
    inv.eorl[arr.ep[i]] = arr.eorl[i];
    inv.eoud[arr.ep[i]] = arr.eoud[i];
  }
  for (int i = 0; i < 8; i++) {
    inv.cp[arr.cp[i]] = i;
    inv.coud[arr.cp[i]] = arr.coud[i];
    inv.corl[arr.cp[i]] = arr.corl[i];
    inv.cofb[arr.cp[i]] = arr.cofb[i];
  }
  for (int i = 0; i < 6; i++)
    inv.cpos[arr.cpos[i]] = i;

  return arrays_to_cube(inv, fAll);
}

Cube compose_via_arrays(CubeArray arr2, Cube c1, PieceFilter f) {
  /* This is basically the same as the move_cubearray function above */
  CubeArray arr1 = {0};
  cube_to_arrays(c1, &arr1, fAll);

  apply_permutation(arr2.ep,   arr1.ep,   12);
  apply_permutation(arr2.ep,   arr1.eofb, 12);
  apply_permutation(arr2.ep,   arr1.eorl, 12);
  apply_permutation(arr2.ep,   arr1.eoud, 12);
  sum_arrays_mod(arr2.eofb,    arr1.eofb, 12, 2);
  sum_arrays_mod(arr2.eorl,    arr1.eorl, 12, 2);
  sum_arrays_mod(arr2.eoud,    arr1.eoud, 12, 2);
  apply_permutation(arr2.cp,   arr1.cp,   8);
  apply_permutation(arr2.cp,   arr1.coud, 8);
  apply_permutation(arr2.cp,   arr1.corl, 8);
  apply_permutation(arr2.cp,   arr1.cofb, 8);
  sum_arrays_mod(arr2.coud,    arr1.coud, 8, 3);
  sum_arrays_mod(arr2.corl,    arr1.corl, 8, 3);
  sum_arrays_mod(arr2.cofb,    arr1.cofb, 8, 3);
  apply_permutation(arr2.cpos, arr1.cpos, 6);

  return arrays_to_cube(arr1, fAll);
}
