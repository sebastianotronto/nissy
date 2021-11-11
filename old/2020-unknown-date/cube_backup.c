#include <stdio.h>
#include <string.h>
#include "cube.h"

/* The next few functions are used to convert from the Cube structure
 * representation to actual arrays of pieces. */
void cube_to_ep_array(Cube cube, int ep[12]);
void cube_to_eofb_array(Cube cube, int eo[12]);
void cube_to_eorl_array(Cube cube, int eo[12]);
void cube_to_eoud_array(Cube cube, int eo[12]);
void cube_to_cp_array(Cube cube, int cp[8]);
void cube_to_coud_array(Cube cube, int co[8]);
void cube_to_cofb_array(Cube cube, int co[8]);
void cube_to_corl_array(Cube cube, int co[8]);
void cube_to_centerpos_array(Cube cube, int centerpos[6]);
Cube ep_array_to_cube(int ep[12]);
Cube eofb_array_to_cube(int eo[12]);
Cube eorl_array_to_cube(int eo[12]);
Cube eoud_array_to_cube(int eo[12]);
Cube cp_array_to_cube(int cp[8]);
Cube coud_array_to_cube(int co[8]);
Cube cofb_array_to_cube(int co[8]);
Cube corl_array_to_cube(int co[8]);
Cube centerpos_array_to_cube(int centerpos[6]);
Cube move_array(Cube cube, void (cube_to_arr)(Cube, int *),
                           Cube (*arr_to_cube)(int *),
                           int *perm, int *orient, int n, int m);
                          

/* Transition tables */
int epose_ttable[NMOVES][factorial12/factorial8];
int eposs_ttable[NMOVES][factorial12/factorial8];
int eposm_ttable[NMOVES][factorial12/factorial8];
int eofb_ttable[NMOVES][pow2to11];
int eorl_ttable[NMOVES][pow2to11];
int eoud_ttable[NMOVES][pow2to11];
int cp_ttable[NMOVES][factorial8];
int coud_ttable[NMOVES][pow3to7];
int cofb_ttable[NMOVES][pow3to7];
int corl_ttable[NMOVES][pow3to7];
int centerpos_ttable[NMOVES][factorial6];

char edge_string[12][5] = {
  "UF", "UL", "UB", "UR", "DF", "DL", "DB", "DR", "FR", "FL", "BL", "BR"
};
char corner_string[8][5] = { "UFR", "UFL", "UBL", "UBR", "DFR", "DFL", "DBL", "DBR" };
char center_string[6][5] = { "U", "D", "R", "L", "F", "B" };
char move_string[NMOVES][5] = {
  "-",
  "U", "U2", "U\'", "D", "D2", "D\'", "R", "R2", "R\'",
  "L", "L2", "L\'", "F", "F2", "F\'", "B", "B2", "B\'",
  "Uw", "Uw2", "Uw\'", "Dw", "Dw2", "Dw\'", "Rw", "Rw2", "Rw\'",
  "Lw", "Lw2", "Lw\'", "Fw", "Fw2", "Fw\'", "Bw", "Bw2", "Bw\'",
  "M", "M2", "M\'", "S", "S2", "S\'", "E", "E2", "E\'",
  "x", "x2", "x\'", "y", "y2", "y\'", "z", "z2", "z\'",
};

int epe_solved[] = {FR, FL, BL, BR};
int eps_solved[] = {UL, UR, DL, DR};
int epm_solved[] = {UF, UB, DF, DB};

/**************************/
/* Internal use functions */
/**************************/

int cube_to_ep_array(Cube cube, int ep[12]) {
  int epe[4], eps[4], epm[4];
  index_to_perm(cube.epose % factorial(4), 4, epe);
  index_to_perm(cube.eposs % factorial(4), 4, eps);
  index_to_perm(cube.eposm % factorial(4), 4, epm);

  int epose[12], eposs[12], eposm[12];
  index_to_subset(cube.epose / factorial(4), 12, 4, epose);
  index_to_subset(cube.eposs / factorial(4), 12, 4, eposs);
  index_to_subset(cube.eposm / factorial(4), 12, 4, eposm);
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

void cube_to_eofb_array(Cube cube, int eo[12]) {
  int_to_sum_zero_array(cube.eofb, 2, 12, eo);
}

void cube_to_eorl_array(Cube cube, int eo[12]) {
  int_to_sum_zero_array(cube.eorl, 2, 12, eo);
}

void cube_to_eoud_array(Cube cube, int eo[12]) {
  int_to_sum_zero_array(cube.eoud, 2, 12, eo);
}

void cube_to_cp_array(Cube cube, int cp[8]) {
  index_to_perm(cube.cp, 8, cp);
}

void cube_to_coud_array(Cube cube, int co[8]) {
  int_to_sum_zero_array(cube.coud, 3, 8, co);
}

void cube_to_cofb_array(Cube cube, int co[8]) {
  int_to_sum_zero_array(cube.cofb, 3, 8, co);
}

void cube_to_corl_array(Cube cube, int co[8]) {
  int_to_sum_zero_array(cube.corl, 3, 8, co);
}

void cube_to_centerpos_array(Cube cube, int centerpos[6]) {
  index_to_perm(cube.centerpos, 6, centerpos);
}

Cube ep_array_to_cube(int ep[12]) {
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
  return { .epose = factorial(4) * subset_to_index(epose, 12, 4)
                                 + perm_to_index(epe, 4),
           .eposs = factorial(4) * subset_to_index(eposs, 12, 4)
                                 + perm_to_index(eps, 4),
           .eposm = factorial(4) * subset_to_index(eposm, 12, 4)
                                 + perm_to_index(epm, 4) };
}

Cube eofb_array_to_cube(int eo[12]) {
  return { .eofb = digit_array_to_int(eo, 11, 2) };
}

Cube eorl_array_to_cube(int eo[12]) {
  return { .eorl = digit_array_to_int(eo, 11, 2) };
}

Cube eoud_array_to_cube(int eo[12]) {
  return { .eoud = digit_array_to_int(eo, 11, 2) };
}

Cube cp_array_to_cube(int cp[8]) {
  return { .cp = perm_to_index(cp, 8) };
}

Cube coud_array_to_cube(int co[8]) {
  return { .coud = digit_array_to_int(co, 7, 3) };
}

Cube cofb_array_to_cube(int co[8]) {
  return { .cofb = digit_array_to_int(co, 7, 3) };
}

Cube corl_array_to_cube(int co[8]) {
  return { .corl = digit_array_to_int(co, 7, 3) };
}

Cube centerpos_array_to_cube(int centerpos[6]) {
  return { .centerpos = perm_to_index(centerpos, 6) };
}

Cube move_array(Cube cube, void (*cube_to_arr)(Cube, int *),
                           void (*arr_to_cube)(int *),
                           int *perm, int *orient, int n, int m) {
  int arr[n];
  cube_to_arr(cube, arr);
  apply_permutation(perm, arr, n);
  sum_arrays_mod(arr, orient, n, m);
  return arr_to_cube(arr);
}

/***********************/
/* Interface functions */
/***********************/

Move inverse(Move m) {
  if (m == NULLMOVE)
    return m;
  int mod = (m-1)%3;
  Move base = m - mod;
  return base + 2 - mod;
}

void copy_alg(NissMove *src, NissMove *dest) {
  for (int i = 0; src[i].m != NULLMOVE; i++)
    dest[i] = src[i];
}

bool commute(Move m1, Move m2) {
  return equal(apply_move(m2, apply_move(m1, {0})),
               apply_move(m1, apply_move(m2, {0})));
}

bool equal(Cube c1, Cube c2) {
  return c1.eofb   == c2.eofb && c1.epose == c2.epose && 
         c1.eposs == c2.eposs && c1.eposm == c2.eposm &&
         c1.coud  == c2.coud  && c1.cp    == c2.cp    &&
         c1.centerpos == c2.centerpos;
}

bool solvable(Cube cube) {
  /* Since we memorize orientation truncating the last digit, we only need to
   * check that the permutations have the correct sign. */
  /* TODO: add check that every integer is in range */
  int ep[12], cp[12], c[6];
  /* TODO: change to use cube-specific functions */
  cube_to_ep_array(cube, ep);
  cube_to_cp_array(cube, 8, cp);
  cube_to_centerpos_array(cube, 6, c);
  return (perm_sign(ep, 12) ^ perm_sign(c, 6)) == perm_sign(cp, 8);
}

bool is_solved(Cube cube) {
  return (!cube.eofb  && !cube.coud  && !cube.cp    &&
          !cube.epose && !cube.eposs && !cube.eposm && !cube.centerpos);
}

char *to_string(Cube cube) {
  int eo[12], co[8], ep[12], cp[8], cenpos[6];
  cube_to_eofb_array(cube, eo);
  cube_to_coud_array(cube, co);
  cube_to_ep_array(cube, ep);
  cube_to_cp_array(cube, cp);
  cube_to_centerpos_array(cube, cenpos);

  static char ret[1000];

  for (int i = 0; i < 12; i++) {
    strcat(ret, " ");
    strcat(ret, edge_string[ep[i]]);
    strcat(ret, " ");
  }
  strcat(ret, "\n");
  for (int i = 0; i < 12; i++) {
    strcat(ret, "  ");
    char num[2] = { [0] = eo[i] + '0', [1] = 0 };
    strcat(ret, num);
    strcat(ret, " ");
  }
  strcat(ret, "\n");
  for (int i = 0; i < 8; i++) {
    strcat(ret, corner_string[cp[i]]);
    strcat(ret, " ");
  }
  strcat(ret, "\n");
  for (int i = 0; i < 8; i++) {
    strcat(ret, "  ");
    char num[2] = { [0] = co[i] + '0', [1] = 0 };
    strcat(ret, num);
    strcat(ret, " ");
  }
  strcat(ret, "\n");
  for (int i = 0; i < 6; i++) {
    strcat(ret, "  ");
    strcat(ret, center_string[cenpos[i]]);
    strcat(ret, " ");
  }
  strcat(ret, "\n");

  return ret;
}

void init_ttables() {
  FILE *ttf;

  if ((ttf = fopen("ttables", "rb")) != NULL) {
    for (int m = 0; m < NMOVES; m++) {
      fread(epose_ttable[m],     sizeof(int), factorial12/factorial8, ttf);
      fread(eposs_ttable[m],     sizeof(int), factorial12/factorial8, ttf);
      fread(eposm_ttable[m],     sizeof(int), factorial12/factorial8, ttf);
      fread(eofb_ttable[m],      sizeof(int), pow2to11,               ttf);
      fread(eorl_ttable[m],      sizeof(int), pow2to11,               ttf);
      fread(eoud_ttable[m],      sizeof(int), pow2to11,               ttf);
      fread(cp_ttable[m],        sizeof(int), factorial8,             ttf);
      fread(coud_ttable[m],      sizeof(int), pow3to7,                ttf);
      fread(corl_ttable[m],      sizeof(int), pow3to7,                ttf);
      fread(cofb_ttable[m],      sizeof(int), pow3to7,                ttf);
      fread(centerpos_ttable[m], sizeof(int), factorial6,             ttf);
    }
    fclose(ttf);
  } else {
    ttf = fopen("ttables", "wb");
    int empty[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
    /* For each type of pieces only the effects of U, x and y are described */  
    int edge_cycle[NMOVES][12] = {
      [U] = {UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR},
      [X] = {DF, FL, UF, FR, DB, BL, UB, BR, DR, DL, UL, UR},
      [Y] = {UR, UF, UL, UB, DR, DF, DL, DB, BR, FR, FL, BL},
    };
    int eofb_flipped[NMOVES][12] = {
      [X] = { [UF] = 1, [UB] = 1, [DF] = 1, [DB] = 1 },
      [Y] = { [FR] = 1, [FL] = 1, [BL] = 1, [BR] = 1 },
    };
    int eorl_flipped[NMOVES][12] = {
      [X] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
      [Y] = { [FR] = 1, [FL] = 1, [BL] = 1, [BR] = 1 },
    };
    int eoud_flipped[NMOVES][12] = {
      [U] = { [UF] = 1, [UL] = 1, [UB] = 1, [UR] = 1 },
      [X] = { [UF] = 1, [UB] = 1, [DF] = 1, [DB] = 1 },
      [Y] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  },
    };
    int corner_cycle[NMOVES][8] = {
      [U] = {UBR, UFR, UFL, UBL, DFR, DFL, DBL, DBR},
      [X] = {DFR, DFL, UFL, UFR, DBR, DBL, UBL, UBR},
      [Y] = {UBR, UFR, UFL, UBL, DBR, DFR, DFL, DBL},
    };
    int coud_flipped[NMOVES][8] = {
      [X] = {[UFR]=2,[UBR]=1,[DBR]=2,[DFR]=1,[UFL]=1,[UBL]=2,[DBL]=1,[DFL]=2},
    };
    int corl_flipped[NMOVES][8] = {
      [U] = { [UFR] = 1, [UBR] = 2, [UBL] = 1, [UFL] = 2 },
      [Y] = {[UFR]=1,[UBR]=2,[UBL]=1,[UFL]=2,[DFR]=2,[DBR]=1,[DBL]=2,[DFL]=1},
    };
    int cofb_flipped[NMOVES][8] = {
      [U] = { [UFR] = 2, [UBR] = 1, [UBL] = 2, [UFL] = 1 },
      [X] = {[UFR]=1,[UBR]=2,[DFR]=2,[DBR]=1,[UBL]=2,[UFL]=1,[DBL]=1,[DFL]=2},
      [Y] = {[UFR]=2,[UBR]=1,[UBL]=2,[UFL]=1,[DFR]=1,[DBR]=2,[DBL]=1,[DFL]=2},
    };
    int center_cycle[NMOVES][6] = {
      [X] = {F_center, B_center, R_center, L_center, D_center, U_center},
      [Y] = {U_center, D_center, B_center, F_center, R_center, L_center},
    };
  
    /* Each move is reduced to a combination of U, x and y using this table */
    Move equiv_moves[NMOVES][13] = {
      [U]   = { U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [U2]  = { U, U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [U3]  = { U, U, U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [D]   = { X, X, U, X, X, 0, 0, 0, 0, 0, 0, 0, 0 },
      [D2]  = { X, X, U, U, X, X, 0, 0, 0, 0, 0, 0, 0 },
      [D3]  = { X, X, U, U, U, X, X, 0, 0, 0, 0, 0, 0 },
      [R]   = { Y, X, U, X, X, X, Y, Y, Y, 0, 0, 0, 0 },
      [R2]  = { Y, X, U, U, X, X, X, Y, Y, Y, 0, 0, 0 },
      [R3]  = { Y, X, U, U, U, X, X, X, Y, Y, Y, 0, 0 },
      [L]   = { Y, Y, Y, X, U, X, X, X, Y, 0, 0, 0, 0 },
      [L2]  = { Y, Y, Y, X, U, U, X, X, X, Y, 0, 0, 0 },
      [L3]  = { Y, Y, Y, X, U, U, U, X, X, X, Y, 0, 0 },
      [F]   = { X, U, X, X, X, 0, 0, 0, 0, 0, 0, 0, 0 },
      [F2]  = { X, U, U, X, X, X, 0, 0, 0, 0, 0, 0, 0 },
      [F3]  = { X, U, U, U, X, X, X, 0, 0, 0, 0, 0, 0 },
      [B]   = { X, X, X, U, X, 0, 0, 0, 0, 0, 0, 0, 0 },
      [B2]  = { X, X, X, U, U, X, 0, 0, 0, 0, 0, 0, 0 },
      [B3]  = { X, X, X, U, U, U, X, 0, 0, 0, 0, 0, 0 },
    
      [Uw]  = { X, X, U, X, X, Y, 0, 0, 0, 0, 0, 0, 0 },
      [Uw2] = { X, X, U, U, X, X, Y, Y, 0, 0, 0, 0, 0 },
      [Uw3] = { X, X, U, U, U, X, X, Y, Y, Y, 0, 0, 0 },
      [Dw]  = { U, Y, Y, Y, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [Dw2] = { U, U, Y, Y, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [Dw3] = { U, U, U, Y, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [Rw]  = { Y, Y, Y, X, U, X, X, X, Y, X, 0, 0, 0 },
      [Rw2] = { Y, Y, Y, X, U, U, X, X, X, Y, X, X, 0 },
      [Rw3] = { Y, Y, Y, X, U, U, U, Y, X, X, X, Y, 0 },
      [Lw]  = { Y, X, U, X, X, X, Y, Y, Y, X, X, X, 0 },
      [Lw2] = { Y, X, U, U, X, X, X, Y, Y, Y, X, X, 0 },
      [Lw3] = { Y, X, U, U, U, X, X, X, Y, Y, Y, X, 0 },
      [Fw]  = { X, X, X, U, Y, Y, Y, X, 0, 0, 0, 0, 0 },
      [Fw2] = { X, X, X, U, U, Y, Y, X, 0, 0, 0, 0, 0 },
      [Fw3] = { X, X, X, U, U, U, Y, X, 0, 0, 0, 0, 0 },
      [Bw]  = { X, U, Y, Y, Y, X, X, X, 0, 0, 0, 0, 0 },
      [Bw2] = { X, U, U, Y, Y, X, X, X, 0, 0, 0, 0, 0 },
      [Bw3] = { X, U, U, U, Y, X, X, X, 0, 0, 0, 0, 0 },
    
      [M]   = { Y, X, U, X, X, U, U, U, Y, X, Y, Y, Y },
      [M2]  = { Y, X, U, U, X, X, U, U, X, X, X, Y, 0 },
      [M3]  = { Y, X, U, U, U, X, X, U, Y, X, X, X, Y },
      [S]   = { X, U, U, U, X, X, U, Y, Y, Y, X, 0, 0 },
      [S2]  = { X, U, U, X, X, U, U, Y, Y, X, 0, 0, 0 },
      [S3]  = { X, U, X, X, U, U, U, Y, X, 0, 0, 0, 0 },
      [E]   = { U, X, X, U, U, U, X, X, Y, Y, Y, 0, 0 },
      [E2]  = { U, U, X, X, U, U, X, X, Y, Y, 0, 0, 0 },
      [E3]  = { U, U, U, X, X, U, X, X, Y, 0, 0, 0, 0 },
    
      [X]   = { X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [X2]  = { X, X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [X3]  = { X, X, X, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [Y]   = { Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [Y2]  = { Y, Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [Y3]  = { Y, Y, Y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [Z]   = { Y, Y, Y, X, Y, 0, 0, 0, 0, 0, 0, 0, 0 },
      [Z2]  = { Y, Y, X, X, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
      [Z3]  = { Y, X, Y, Y, Y, 0, 0, 0, 0, 0, 0, 0, 0 },
    };
  
    /* Generate all move cycles and flips */
    for (int i = 0; i < NMOVES; i++) {
      if (i == U || i == X || i == Y)
        continue;
  
      Cube cube = {0};
  
      cube_to_ep_array(&cube, edge_cycle[i]);
      cube_to_eofb_array(&cube, eofb_flipped[i]);
      cube_to_eorl_array(&cube, eorl_flipped[i]);
      cube_to_eoud_array(&cube, eoud_flipped[i]);
      cube_to_cp_array(&cube, corner_cycle[i]);
      cube_to_coud_array(&cube, coud_flipped[i]);
      cube_to_cofb_array(&cube, cofb_flipped[i]);
      cube_to_corl_array(&cube, corl_flipped[i]);
      cube_to_centerpos_array(&cube, center_cycle[i]);
  
      for (int j = 0; equiv_moves[i][j]; j++) {
        int m = equiv_moves[i][j];
        
        apply_permutation(edge_cycle[m], edge_cycle[i], 12);
        apply_permutation(edge_cycle[m], eofb_flipped[i], 12);
        apply_permutation(edge_cycle[m], eorl_flipped[i], 12);
        apply_permutation(edge_cycle[m], eoud_flipped[i], 12);
  
        sum_arrays_mod(eofb_flipped[i], eofb_flipped[m], 12, 2);
        sum_arrays_mod(eorl_flipped[i], eorl_flipped[m], 12, 2);
        sum_arrays_mod(eoud_flipped[i], eoud_flipped[m], 12, 2);
  
        apply_permutation(corner_cycle[m], corner_cycle[i], 8);
        apply_permutation(corner_cycle[m], coud_flipped[i], 8);
        apply_permutation(corner_cycle[m], cofb_flipped[i], 8);
        apply_permutation(corner_cycle[m], corl_flipped[i], 8);
  
        sum_arrays_mod(coud_flipped[i], coud_flipped[m], 8, 3);
        sum_arrays_mod(corl_flipped[i], corl_flipped[m], 8, 3);
        sum_arrays_mod(cofb_flipped[i], cofb_flipped[m], 8, 3);
  
        apply_permutation(center_cycle[m], center_cycle[i], 6);
      }
    }
  
    /* Initialize transition tables */
    for (int m = 0; m < NMOVES; m++) {
      for (int i = 0; i < factorial12/factorial8; i++) {
        Cube cube = { .epose = i };
        move_array(&cube, cube_to_ep_array, ep_array_to_cube,
                   edge_cycle[m], empty, 12, 1);
        epose_ttable[m][i] = cube.epose;
        cube.eposs = i;
        move_array(&cube, cube_to_ep_array, ep_array_to_cube,
                   edge_cycle[m], empty, 12, 1);
        eposs_ttable[m][i] = cube.eposs;
        cube.eposm = i;
        move_array(&cube, cube_to_ep_array, ep_array_to_cube,
                   edge_cycle[m], empty, 12, 1);
        eposm_ttable[m][i] = cube.eposm;
      }
      for (int i = 0; i < pow2to11; i++ ) {
        Cube cube = { .eofb = i, .eorl = i, .eoud = i };
        move_array(&cube, cube_to_eofb_array, eofb_array_to_cube,
                   edge_cycle[m], eofb_flipped[m], 12, 2);
        move_array(&cube, cube_to_eorl_array, eorl_array_to_cube,
                   edge_cycle[m], eorl_flipped[m], 12, 2);
        move_array(&cube, cube_to_eoud_array, eoud_array_to_cube,
                   edge_cycle[m], eoud_flipped[m], 12, 2);
        eofb_ttable[m][i] = cube.eofb;
        eorl_ttable[m][i] = cube.eorl;
        eoud_ttable[m][i] = cube.eoud;
      }
      for (int i = 0; i < factorial8; i++) {
        /* TODO: Maybe do this by hand to optimize? */
        Cube cube = { .cp = i };
        move_array(&cube, cube_to_cp_array, ep_array_to_cube,
                   corner_cycle[m], empty, 8, 1);
        cp_ttable[m][i] = cube.cp;
      }
      for (int i = 0; i < pow3to7; i++) {
        Cube cube = { .coud = i, .corl = i, .cofb = i };
        move_array(&cube, cube_to_coud_array, coud_array_to_cube,
                   corner_cycle[m], coud_flipped[m], 8, 3);
        move_array(&cube, cube_to_corl_array, corl_array_to_cube,
                   corner_cycle[m], corl_flipped[m], 8, 3);
        move_array(&cube, cube_to_cofb_array, cofb_array_to_cube,
                   corner_cycle[m], cofb_flipped[m], 8, 3);
        coud_ttable[m][i] = cube.coud;
        corl_ttable[m][i] = cube.corl;
        cofb_ttable[m][i] = cube.cofb;
      }
      for (int i = 0; i < factorial6; i++) {
        Cube cube = { .centerpos = i };
        move_array(&cube, cube_to_centerpos_array, centerpos_array_to_cube,
                   center_cycle[m], empty, 6, 1);
        centerpos_ttable[m][i] = cube.centerpos;
      }
      fwrite(epose_ttable[m],     sizeof(int), factorial12/factorial8, ttf);
      fwrite(eposs_ttable[m],     sizeof(int), factorial12/factorial8, ttf);
      fwrite(eposm_ttable[m],     sizeof(int), factorial12/factorial8, ttf);
      fwrite(eofb_ttable[m],      sizeof(int), pow2to11,               ttf);
      fwrite(eorl_ttable[m],      sizeof(int), pow2to11,               ttf);
      fwrite(eoud_ttable[m],      sizeof(int), pow2to11,               ttf);
      fwrite(cp_ttable[m],        sizeof(int), factorial8,             ttf);
      fwrite(coud_ttable[m],      sizeof(int), pow3to7,                ttf);
      fwrite(corl_ttable[m],      sizeof(int), pow3to7,                ttf);
      fwrite(cofb_ttable[m],      sizeof(int), pow3to7,                ttf);
      fwrite(centerpos_ttable[m], sizeof(int), factorial6,             ttf);
    }
    fclose(ttf);
  }
}

Cube apply_move(Move m, Cube cube) {
  Cube moved = cube;

  moved.eofb = eofb_ttable[m][cube.eofb];
  moved.eorl = eorl_ttable[m][cube.eorl];
  moved.eoud = eoud_ttable[m][cube.eoud];
  moved.coud = coud_ttable[m][cube.coud];
  moved.cofb = cofb_ttable[m][cube.cofb];
  moved.corl = corl_ttable[m][cube.corl];
  moved.epose = epose_ttable[m][cube.epose];
  moved.eposs = eposs_ttable[m][cube.eposs];
  moved.eposm = eposm_ttable[m][cube.eposm];
  moved.cp = cp_ttable[m][cube.cp];
  moved.centerpos = centerpos_ttable[m][cube.centerpos];

  return moved;
}
