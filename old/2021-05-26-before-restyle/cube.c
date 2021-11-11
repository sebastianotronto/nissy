#include "cube.h"

typedef struct {
  int ep[12],eofb[12],eorl[12],eoud[12],cp[8],coud[8],corl[8],cofb[8],cpos[6];
} CubeArrayAllocated;

void allocate_cubearray(CubeArray *arr, CubeArrayAllocated *all);

char edge_string[12][5] =
  { "UF", "UL", "UB", "UR", "DF", "DL", "DB", "DR", "FR", "FL", "BL", "BR" };
char corner_string[8][5] = { "UFR","UFL","UBL","UBR","DFR","DFL","DBL","DBR" };
char center_string[6][5] = { "U", "D", "R", "L", "F", "B" };
    
int epe_solved[4] = {FR, FL, BL, BR};
int eps_solved[4] = {UL, UR, DL, DR};
int epm_solved[4] = {UF, UB, DF, DB};

PieceFilter pf_all  = {true,true,true,true,true,true,true,true,true,true,true},
            pf_cpos = { .cpos  = true }, pf_cp   = { .cp    = true },
            pf_ep   = { .epose = true, .eposs = true, .eposm = true },
            pf_e    = {.epose=true}, pf_s={.eposs=true}, pf_m={.eposm=true},
            pf_eo   = { .eofb = true, .eorl = true, .eoud = true },
            pf_co   = { .coud = true, .cofb = true, .corl = true };

void allocate_cubearray(CubeArray *arr, CubeArrayAllocated *all) {
  arr->ep   = all->ep;
  arr->eofb = all->eofb;
  arr->eorl = all->eorl;
  arr->eoud = all->eoud;
  arr->cp   = all->cp;
  arr->coud = all->coud;
  arr->corl = all->corl;
  arr->cofb = all->cofb;
  arr->cpos = all->cpos;
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

int piece_orientation(Cube cube, int piece, char *orientation) {
  int arr[12], n, b;
  uint16_t x;
  if (!strcmp(orientation, "eofb")) { x = cube.eofb; n = 12; b = 2; } else
  if (!strcmp(orientation, "eorl")) { x = cube.eorl; n = 12; b = 2; } else
  if (!strcmp(orientation, "eoud")) { x = cube.eoud; n = 12; b = 2; } else
  if (!strcmp(orientation, "coud")) { x = cube.coud; n = 8;  b = 3; } else
  if (!strcmp(orientation, "corl")) { x = cube.corl; n = 8;  b = 3; } else
  if (!strcmp(orientation, "cofb")) { x = cube.cofb; n = 8;  b = 3; }
  else return -1;

  int_to_sum_zero_array(x, b, n, arr);
  if (piece < n)
    return arr[piece];
  return -1;
}

Center center_at(Cube cube, Center c) {
  static CubeArrayAllocated all = {0};
  CubeArray arr = {0};
  allocate_cubearray(&arr, &all);
  cube_to_arrays(cube, &arr, pf_cpos);
  return arr.cpos[c];
}

Edge edge_at(Cube cube, Edge e) {
  static CubeArrayAllocated all = {0};
  CubeArray arr = {0};
  allocate_cubearray(&arr, &all);
  cube_to_arrays(cube, &arr, pf_ep);
  return arr.ep[e];
}

Corner corner_at(Cube cube, Corner c) {
  static CubeArrayAllocated all = {0};
  CubeArray arr = {0};
  allocate_cubearray(&arr, &all);
  cube_to_arrays(cube, &arr, pf_cp);
  return arr.cp[c];
}

bool block_solved(Cube cube, Block block) {
  static CubeArrayAllocated all = {0};
  CubeArray arr = {0};
  allocate_cubearray(&arr, &all);
  cube_to_arrays(cube, &arr, pf_all);

  bool ret = true;

  for (int i = 0; i < 12; i++)
    ret = ret && !(block.edge[i] && (arr.ep[i] != i || arr.eofb[i]));
  for (int i = 0; i < 8; i++)
    ret = ret && !(block.corner[i] && (arr.cp[i] != i || arr.coud[i]));
  for (int i = 0; i < 6; i++)
    ret = ret && !(block.center[i] && arr.cpos[i] != i);

  return ret;
}

bool equal(Cube c1, Cube c2) {
  return c1.eofb   == c2.eofb  && c1.epose == c2.epose && 
         c1.eposs  == c2.eposs && c1.eposm == c2.eposm &&
         c1.coud   == c2.coud  && c1.cp    == c2.cp    &&
         c1.cpos   == c2.cpos;
}

bool is_solved(Cube cube) {
  /* TODO: might return true if cube is not solvable but looks solved form one
           of the incompatible interpretations (e.g. eofb and ep solved, but
           eorl not solve) */
  return !cube.eofb  && !cube.coud  && !cube.cp    &&
         !cube.epose && !cube.eposs && !cube.eposm && !cube.cpos;
}

void print_cube(Cube cube) {
  static CubeArrayAllocated all = {0};
  CubeArray arrx = {0};
  allocate_cubearray(&arrx, &all);
  cube_to_arrays(cube, &arrx, pf_all);

  for (int i = 0; i < 12; i++) printf(" %s ",  edge_string[arrx.ep[i]]);
  printf("\n");
  for (int i = 0; i < 12; i++) printf("  %c ", arrx.eofb[i] + '0');
  printf("\n");
  for (int i = 0; i < 8; i++)  printf("%s ",   corner_string[arrx.cp[i]]);
  printf("\n");
  for (int i = 0; i < 8; i++)  printf("  %c ", arrx.coud[i] + '0');
  printf("\n");
  for (int i = 0; i < 6; i++)  printf("  %s ", center_string[arrx.cpos[i]]);
  printf("\n");
}

Cube admissible_ep(Cube cube, PieceFilter f) {
  static CubeArrayAllocated all = {0};
  CubeArray arrx = {0};
  allocate_cubearray(&arrx, &all);
  cube_to_arrays(cube, &arrx, f);

  bool used[12] = {0};
  for (int i = 0; i < 12; i++)
    if (arrx.ep[i] != -1)
      used[arrx.ep[i]] = true;
  for (int i = 0, j = 0; i < 12; i++) {
    while (j < 11 && used[j]) j++;
    if (arrx.ep[i] == -1)
      arrx.ep[i] = j++;
  }

  return arrays_to_cube(arrx, pf_ep);
}

Cube inverse_cube(Cube cube) {
  static CubeArrayAllocated all = {0}, invall = {0};
  CubeArray arrx = {0}, invx = {0};
  allocate_cubearray(&arrx, &all);
  allocate_cubearray(&invx, &invall);

  cube_to_arrays(cube, &arrx, pf_all);

  for (int i = 0; i < 12; i++) {
    invx.ep[arrx.ep[i]] = i;
    invx.eofb[arrx.ep[i]] = arrx.eofb[i];
    invx.eorl[arrx.ep[i]] = arrx.eorl[i];
    invx.eoud[arrx.ep[i]] = arrx.eoud[i];
  }
  for (int i = 0; i < 8; i++) {
    invx.cp[arrx.cp[i]] = i;
    invx.coud[arrx.cp[i]] = (3 - arrx.coud[i])%3;
    invx.corl[arrx.cp[i]] = (3 - arrx.corl[i])%3;
    invx.cofb[arrx.cp[i]] = (3 - arrx.cofb[i])%3;
  }
  for (int i = 0; i < 6; i++)
    invx.cpos[arrx.cpos[i]] = i;

  return arrays_to_cube(invx, pf_all);
}

Cube move_via_arrays(CubeArray arr, Cube c, PieceFilter f) {
  static CubeArrayAllocated all = {0};
  CubeArray arrx = {0};
  allocate_cubearray(&arrx, &all);

  cube_to_arrays(c, &arrx, f);

  if (f.epose || f.eposs || f.eposm)
                apply_permutation(   arr.ep,      arrx.ep,     12      );
  if (f.eofb) { apply_permutation(   arr.ep,      arrx.eofb,   12      );
                sum_arrays_mod(      arr.eofb,    arrx.eofb,   12, 2   ); }
  if (f.eorl) { apply_permutation(   arr.ep,      arrx.eorl,   12      );
                sum_arrays_mod(      arr.eorl,    arrx.eorl,   12, 2   ); }
  if (f.eoud) { apply_permutation(   arr.ep,      arrx.eoud,   12      );
                sum_arrays_mod(      arr.eoud,    arrx.eoud,   12, 2   ); }
  if (f.cp)     apply_permutation(   arr.cp,      arrx.cp,     8       );
  if (f.coud) { apply_permutation(   arr.cp,      arrx.coud,   8       );
                sum_arrays_mod(      arr.coud,    arrx.coud,   8, 3    ); }
  if (f.corl) { apply_permutation(   arr.cp,      arrx.corl,   8       );
                sum_arrays_mod(      arr.corl,    arrx.corl,   8, 3    ); }
  if (f.cofb) { apply_permutation(   arr.cp,      arrx.cofb,   8       );
                sum_arrays_mod(      arr.cofb,    arrx.cofb,   8, 3    ); }
  if (f.cpos)   apply_permutation(   arr.cpos,    arrx.cpos,   6       );

  return arrays_to_cube(arrx, f);
}

Cube compose_filtered(Cube c2, Cube c1, PieceFilter f) {
  static CubeArrayAllocated all = {0};
  CubeArray arrx = {0};
  allocate_cubearray(&arrx, &all);

  cube_to_arrays(c2, &arrx, f);
  return move_via_arrays(arrx, c1, f);
}

Cube compose(Cube c2, Cube c1) {
  return compose_filtered(c2, c1, pf_all);
}
