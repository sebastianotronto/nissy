#include "moves.h"

void move_cubearray(Move m, CubeArray *arr, PieceFilter f);
Cube move_via_array(Move m, Cube cube, PieceFilter f);
void sort_cancel_rotate(NissMove *alg, int n, bool inv, int top, int front);
bool read_ttables_file();
bool write_ttables_file();

/* Transition tables */
uint16_t epose_ttable[NMOVES][factorial12/factorial8];
uint16_t eposs_ttable[NMOVES][factorial12/factorial8];
uint16_t eposm_ttable[NMOVES][factorial12/factorial8];
uint16_t eofb_ttable[NMOVES][pow2to11];
uint16_t eorl_ttable[NMOVES][pow2to11];
uint16_t eoud_ttable[NMOVES][pow2to11];
uint16_t cp_ttable[NMOVES][factorial8];
uint16_t coud_ttable[NMOVES][pow3to7];
uint16_t cofb_ttable[NMOVES][pow3to7];
uint16_t corl_ttable[NMOVES][pow3to7];
uint16_t cpos_ttable[NMOVES][factorial6];

bool commute[NMOVES][NMOVES];
bool possible_next[NMOVES][NMOVES][NMOVES];
Move inverse[NMOVES];

char move_string[NMOVES][5] =
  { "-",
    "U", "U2", "U\'", "D", "D2", "D\'", "R", "R2", "R\'",
    "L", "L2", "L\'", "F", "F2", "F\'", "B", "B2", "B\'",
    "Uw", "Uw2", "Uw\'", "Dw", "Dw2", "Dw\'", "Rw", "Rw2", "Rw\'",
    "Lw", "Lw2", "Lw\'", "Fw", "Fw2", "Fw\'", "Bw", "Bw2", "Bw\'",
    "M", "M2", "M\'", "S", "S2", "S\'", "E", "E2", "E\'",
    "x", "x2", "x\'", "y", "y2", "y\'", "z", "z2", "z\'" };

/* For each type of pieces only the effects of U, x and y are described */  
int edge_cycle[NMOVES][12] =
  { [U] = {UR, UF, UL, UB, DF, DL, DB, DR, FR, FL, BL, BR},
    [x] = {DF, FL, UF, FR, DB, BL, UB, BR, DR, DL, UL, UR},
    [y] = {UR, UF, UL, UB, DR, DF, DL, DB, BR, FR, FL, BL} };
int eofb_flipped[NMOVES][12] =
  { [x] = { [UF] = 1, [UB] = 1, [DF] = 1, [DB] = 1 },
    [y] = { [FR] = 1, [FL] = 1, [BL] = 1, [BR] = 1 } };
int eorl_flipped[NMOVES][12] =
  { [x] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
    [y] = { [FR] = 1, [FL] = 1, [BL] = 1, [BR] = 1 } };
int eoud_flipped[NMOVES][12] =
  { [U] = { [UF] = 1, [UL] = 1, [UB] = 1, [UR] = 1 },
    [x] = { [UF] = 1, [UB] = 1, [DF] = 1, [DB] = 1 },
    [y] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  } };
int corner_cycle[NMOVES][8] =
  { [U] = {UBR, UFR, UFL, UBL, DFR, DFL, DBL, DBR},
    [x] = {DFR, DFL, UFL, UFR, DBR, DBL, UBL, UBR},
    [y] = {UBR, UFR, UFL, UBL, DBR, DFR, DFL, DBL} };
int coud_flipped[NMOVES][8] =
  { [x] = {[UFR]=2,[UBR]=1,[DBR]=2,[DFR]=1,[UFL]=1,[UBL]=2,[DBL]=1,[DFL]=2} };
int corl_flipped[NMOVES][8] =
  { [U] = { [UFR] = 1, [UBR] = 2, [UBL] = 1, [UFL] = 2 },
    [y] = {[UFR]=1,[UBR]=2,[UBL]=1,[UFL]=2,[DFR]=2,[DBR]=1,[DBL]=2,[DFL]=1} };
int cofb_flipped[NMOVES][8] =
  { [U] = { [UFR] = 2, [UBR] = 1, [UBL] = 2, [UFL] = 1 },
    [x] = {[UFR]=1,[UBR]=2,[DFR]=2,[DBR]=1,[UBL]=2,[UFL]=1,[DBL]=1,[DFL]=2},
    [y] = {[UFR]=2,[UBR]=1,[UBL]=2,[UFL]=1,[DFR]=1,[DBR]=2,[DBL]=1,[DFL]=2} };
int center_cycle[NMOVES][6] =
  { [x] = {F_center, B_center, R_center, L_center, D_center, U_center},
    [y] = {U_center, D_center, B_center, F_center, R_center, L_center} };

/* Each move is reduced to a combination of U, x and y using this table */
Move equiv_moves[NMOVES][14] = {
  [U]   = { U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [U2]  = { U, U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [U3]  = { U, U, U, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [D]   = { x, x, U, x, x, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [D2]  = { x, x, U, U, x, x, 0, 0, 0, 0, 0, 0, 0, 0 },
  [D3]  = { x, x, U, U, U, x, x, 0, 0, 0, 0, 0, 0, 0 },
  [R]   = { y, x, U, x, x, x, y, y, y, 0, 0, 0, 0, 0 },
  [R2]  = { y, x, U, U, x, x, x, y, y, y, 0, 0, 0, 0 },
  [R3]  = { y, x, U, U, U, x, x, x, y, y, y, 0, 0, 0 },
  [L]   = { y, y, y, x, U, x, x, x, y, 0, 0, 0, 0, 0 },
  [L2]  = { y, y, y, x, U, U, x, x, x, y, 0, 0, 0, 0 },
  [L3]  = { y, y, y, x, U, U, U, x, x, x, y, 0, 0, 0 },
  [F]   = { x, U, x, x, x, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [F2]  = { x, U, U, x, x, x, 0, 0, 0, 0, 0, 0, 0, 0 },
  [F3]  = { x, U, U, U, x, x, x, 0, 0, 0, 0, 0, 0, 0 },
  [B]   = { x, x, x, U, x, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [B2]  = { x, x, x, U, U, x, 0, 0, 0, 0, 0, 0, 0, 0 },
  [B3]  = { x, x, x, U, U, U, x, 0, 0, 0, 0, 0, 0, 0 },

  [Uw]  = { x, x, U, x, x, y, 0, 0, 0, 0, 0, 0, 0, 0 },
  [Uw2] = { x, x, U, U, x, x, y, y, 0, 0, 0, 0, 0, 0 },
  [Uw3] = { x, x, U, U, U, x, x, y, y, y, 0, 0, 0, 0 },
  [Dw]  = { U, y, y, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [Dw2] = { U, U, y, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [Dw3] = { U, U, U, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [Rw]  = { y, y, y, x, U, x, x, x, y, x, 0, 0, 0, 0 },
  [Rw2] = { y, y, y, x, U, U, x, x, x, y, x, x, 0, 0 },
  [Rw3] = { y, y, y, x, U, U, U, y, x, x, x, y, 0, 0 },
  [Lw]  = { y, x, U, x, x, x, y, y, y, x, x, x, 0, 0 },
  [Lw2] = { y, x, U, U, x, x, x, y, y, y, x, x, 0, 0 },
  [Lw3] = { y, x, U, U, U, x, x, x, y, y, y, x, 0, 0 },
  [Fw]  = { x, x, x, U, y, y, y, x, 0, 0, 0, 0, 0, 0 },
  [Fw2] = { x, x, x, U, U, y, y, x, 0, 0, 0, 0, 0, 0 },
  [Fw3] = { x, x, x, U, U, U, y, x, 0, 0, 0, 0, 0, 0 },
  [Bw]  = { x, U, y, y, y, x, x, x, 0, 0, 0, 0, 0, 0 },
  [Bw2] = { x, U, U, y, y, x, x, x, 0, 0, 0, 0, 0, 0 },
  [Bw3] = { x, U, U, U, y, x, x, x, 0, 0, 0, 0, 0, 0 },

  [M]   = { y, x, U, x, x, U, U, U, y, x, y, y, y, 0 },
  [M2]  = { y, x, U, U, x, x, U, U, x, x, x, y, 0, 0 },
  [M3]  = { y, x, U, U, U, x, x, U, y, x, x, x, y, 0 },
  [S]   = { x, U, U, U, x, x, U, y, y, y, x, 0, 0, 0 },
  [S2]  = { x, U, U, x, x, U, U, y, y, x, 0, 0, 0, 0 },
  [S3]  = { x, U, x, x, U, U, U, y, x, 0, 0, 0, 0, 0 },
  [E]   = { U, x, x, U, U, U, x, x, y, y, y, 0, 0, 0 },
  [E2]  = { U, U, x, x, U, U, x, x, y, y, 0, 0, 0, 0 },
  [E3]  = { U, U, U, x, x, U, x, x, y, 0, 0, 0, 0, 0 },

  [x]   = { x, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [x2]  = { x, x, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [x3]  = { x, x, x, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [y]   = { y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [y2]  = { y, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [y3]  = { y, y, y, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [z]   = { y, y, y, x, y, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [z2]  = { y, y, x, x, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  [z3]  = { y, x, y, y, y, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
};

/* Movesets */
bool standard_moveset[NMOVES] = {
  [U] = true, [U2] = true, [U3] = true, [D] = true, [D2] = true, [D3] = true,
  [R] = true, [R2] = true, [R3] = true, [L] = true, [L2] = true, [L3] = true,
  [F] = true, [F2] = true, [F3] = true, [B] = true, [B2] = true, [B3] = true,
};

bool is_solved_up_to_reorient(Cube cube) {
  if (is_solved(cube))
    return true;

  /*TODO: improve this crap */

  bool ret = false;
  for (int i = x; i <= z3; i++) {
    ret = ret || is_solved(move_cube(i, cube));
    for (int j = x; j <= z3; j++)
      ret = ret || is_solved(move_cube(i, move_cube(j, cube)));
  }
  return ret;
}

void move_cubearray(Move m, CubeArray *arr, PieceFilter f) {
  if (f.epose || f.eposs || f.eposm)
    apply_permutation(edge_cycle[m], arr->ep, 12);
  if (f.eofb) {
    apply_permutation(edge_cycle[m], arr->eofb, 12);
    sum_arrays_mod(eofb_flipped[m],  arr->eofb, 12, 2);
  }
  if (f.eorl) {
    apply_permutation(edge_cycle[m], arr->eorl, 12);
    sum_arrays_mod(eorl_flipped[m],  arr->eorl, 12, 2);
  }
  if (f.eoud) {
    apply_permutation(edge_cycle[m], arr->eoud, 12);
    sum_arrays_mod(eoud_flipped[m],  arr->eoud, 12, 2);
  }
  if (f.cp)
    apply_permutation(corner_cycle[m], arr->cp,   8);
  if (f.coud) {
    apply_permutation(corner_cycle[m], arr->coud, 8);
    sum_arrays_mod(coud_flipped[m],    arr->coud, 8, 3);
  }
  if (f.corl) {
    apply_permutation(corner_cycle[m], arr->corl, 8);
    sum_arrays_mod(corl_flipped[m],    arr->corl, 8, 3);
  }
  if (f.cofb) {
    apply_permutation(corner_cycle[m], arr->cofb, 8);
    sum_arrays_mod(cofb_flipped[m],    arr->cofb, 8, 3);
  }
  if (f.cpos)
    apply_permutation(center_cycle[m], arr->cpos, 6);
}

Cube move_via_array(Move m, Cube cube, PieceFilter f) {
  CubeArray arr = {0};
  cube_to_arrays(cube, &arr, f);
  move_cubearray(m, &arr, f);
  return arrays_to_cube(arr, f);
}

int copy_alg(NissMove *src, NissMove *dest) {
  int i;
  for (i = 0; src[i].m != NULLMOVE; i++)
    dest[i] = src[i];
  dest[i].m = NULLMOVE;
  return i;
}

/* TODO: all strings start with space?? */
void print_moves(NissMove *alg) {
  bool niss = false;
  for (int i = 0; alg[i].m != NULLMOVE; i++) {
    char *fill = !niss &&  alg[i].inverse ? " (" :
                 (niss && !alg[i].inverse ? ") " : " ");
    printf("%s%s", fill, move_string[alg[i].m]);
    niss = alg[i].inverse;
  }
  printf("%s\n", niss ? ")" : "");
}

int read_moves(char *str, NissMove *alg, int n) {
  bool niss = false;
  int c = 0;

  for (int i = 0; str[i] && c < n; i++) {
    if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n')
      continue;

    if (str[i] == '(' || str[i] == ')') {
      if ((niss && str[i] == '(') || (!niss && str[i] == ')'))
        return -1;
      niss = !niss;
      continue;
    }

    alg[c].inverse = niss; alg[c].m = NULLMOVE;
    for (Move j = 0; j < NMOVES; j++) {
      if (str[i] == move_string[j][0]) {
        alg[c].m = j;
        if (alg[c].m <= B && str[i+1]=='w')       { alg[c].m += Uw - U; i++; }
        if (str[i+1]=='2')                        { alg[c].m += 1;      i++; }
        else if (str[i+1]=='\'' || str[i+1]=='3') { alg[c].m += 2;      i++; }
        c++;
        break;
      }
    }
  }

  alg[c].m = NULLMOVE;
  return c;
}

/* Helper function for cleanup. alg must contain only basic moves, no 2 or '.
   top and front describe an admissible orientation of the cube. */
void sort_cancel_rotate(NissMove *alg, int n, bool inv, int top, int front) {
  int c = 0, i = 0;
  PieceFilter cpos_only = { .cpos = true };
  NissMove aux[n+3];
  aux[0].m = NULLMOVE;

  while (i < n && alg[i].m != NULLMOVE) {
    int j = i;
    while (j < n && commute[alg[i].m][alg[j].m]) j++;
    Move base = 6*((alg[i].m-1)/6);
    int t1 = 0, t2 = 0;
    for (int k = i; k < j; k++)
      if (alg[k].m == base+1) t1 = (t1+1)%4;
      else                       t2 = (t2+1)%4;
    if (t1) { aux[c].inverse = inv; aux[c].m = base+t1;   c++; }
    if (t2) { aux[c].inverse = inv; aux[c].m = base+t2+3; c++; }
    i = j;
  }
  aux[c].m = NULLMOVE;

  CubeArray q;
  cube_to_arrays(blank_cube(), &q, cpos_only);
  /* First we try to rotate in one move, then we try an x or y rotation
     followed by a z rotation */
  for (int r = x; r <= z3; r++) {
    move_cubearray(r, &q, cpos_only);
    if (q.cpos[F_center] == front && q.cpos[U_center] == top) {
      aux[c].inverse = inv; aux[c].m = r;

      aux[++c].m = NULLMOVE;
      copy_alg(aux, alg);
      return;
    }
    move_cubearray(inverse[r], &q, cpos_only);
  }
  for (int r = x; r <= y3; r++) {
    move_cubearray(r, &q, cpos_only);
    if (q.cpos[F_center] == front) {
      aux[c].inverse = inv; aux[c++].m = r;
      break;
    }
    move_cubearray(inverse[r], &q, cpos_only);
  }
  for (int r = z; r <= z3; r++) {
    move_cubearray(r, &q, cpos_only);
    if (q.cpos[U_center] == top) {
      aux[c].inverse = inv; aux[c++].m = r;
      break;
    }
    move_cubearray(inverse[r], &q, cpos_only);
  }

  aux[c].m = NULLMOVE;
  copy_alg(aux, alg);
}

/* TODO: does not work with niss + rotations */
void cleanup(NissMove *alg, int n) {
  int count_n = 0, count_i = 0, *count;
  PieceFilter cpos_only = { .cpos = true };
  NissMove aux_n[n+1], aux_i[n+1], *aux;
  CubeArray cube_n, cube_i, *cube;
  cube_to_arrays(blank_cube(), &cube_n, cpos_only);
  cube_to_arrays(blank_cube(), &cube_i, cpos_only);
  
  for (int i = 0; count_n + count_i < n && alg[i].m != NULLMOVE; i++) {
    if (alg[i].inverse) { count = &count_i; aux = aux_i; cube = &cube_i; }
    else                { count = &count_n; aux = aux_n; cube = &cube_n; }
    
    for (int j = 0; equiv_moves[alg[i].m][j]; j++) {
      Move m = equiv_moves[alg[i].m][j];
      aux[*count].inverse = alg[i].inverse;
      move_cubearray(m, cube, cpos_only);
      if (m == U) aux[(*count)++].m = 3 * cube->cpos[0] + 1;
    }
  }

  aux_n[count_n].m = NULLMOVE;
  aux_i[count_i].m = NULLMOVE;
  sort_cancel_rotate(aux_n, count_n, false, cube_n.cpos[0], cube_n.cpos[4]); 
  sort_cancel_rotate(aux_i, count_i, true,  cube_i.cpos[0], cube_n.cpos[4]); 
  copy_alg(aux_n, alg);
  copy_alg(aux_i, alg+count_n);
}

bool read_ttables_file() {
  FILE *ttf;
  if ((ttf = fopen("ttables", "rb")) != NULL) {
    for (int m = 0; m < NMOVES; m++) {
      fread(epose_ttable[m], sizeof(uint16_t), factorial12/factorial8, ttf);
      fread(eposs_ttable[m], sizeof(uint16_t), factorial12/factorial8, ttf);
      fread(eposm_ttable[m], sizeof(uint16_t), factorial12/factorial8, ttf);
      fread(eofb_ttable[m],  sizeof(uint16_t), pow2to11,               ttf);
      fread(eorl_ttable[m],  sizeof(uint16_t), pow2to11,               ttf);
      fread(eoud_ttable[m],  sizeof(uint16_t), pow2to11,               ttf);
      fread(cp_ttable[m],    sizeof(uint16_t), factorial8,             ttf);
      fread(coud_ttable[m],  sizeof(uint16_t), pow3to7,                ttf);
      fread(corl_ttable[m],  sizeof(uint16_t), pow3to7,                ttf);
      fread(cofb_ttable[m],  sizeof(uint16_t), pow3to7,                ttf);
      fread(cpos_ttable[m],  sizeof(uint16_t), factorial6,             ttf);
    }
    fclose(ttf);
    return true;
  } else return false;
}

bool write_ttables_file() {
  FILE *ttf;
  if ((ttf = fopen("ttables", "wb")) != NULL) {
    for (int m = 0; m < NMOVES; m++) {
      fwrite(epose_ttable[m], sizeof(uint16_t), factorial12/factorial8, ttf);
      fwrite(eposs_ttable[m], sizeof(uint16_t), factorial12/factorial8, ttf);
      fwrite(eposm_ttable[m], sizeof(uint16_t), factorial12/factorial8, ttf);
      fwrite(eofb_ttable[m],  sizeof(uint16_t), pow2to11,               ttf);
      fwrite(eorl_ttable[m],  sizeof(uint16_t), pow2to11,               ttf);
      fwrite(eoud_ttable[m],  sizeof(uint16_t), pow2to11,               ttf);
      fwrite(cp_ttable[m],    sizeof(uint16_t), factorial8,             ttf);
      fwrite(coud_ttable[m],  sizeof(uint16_t), pow3to7,                ttf);
      fwrite(corl_ttable[m],  sizeof(uint16_t), pow3to7,                ttf);
      fwrite(cofb_ttable[m],  sizeof(uint16_t), pow3to7,                ttf);
      fwrite(cpos_ttable[m],  sizeof(uint16_t), factorial6,             ttf);
    }
    fclose(ttf);
    return true;
  } else return false;
}

void init_ttables(bool read, bool write) {
  /* Generate all move cycles and flips; I do this regardless */
  for (int i = 0; i < NMOVES; i++) {
    if (i == U || i == x || i == y)
      continue;

    CubeArray arr = {0};
    cube_to_arrays(blank_cube(), &arr, fAll);
    for (int j = 0; equiv_moves[i][j]; j++)
      move_cubearray(equiv_moves[i][j], &arr, fAll);

    intarrcopy(arr.ep,   edge_cycle[i],   12);
    intarrcopy(arr.eofb, eofb_flipped[i], 12);
    intarrcopy(arr.eorl, eorl_flipped[i], 12);
    intarrcopy(arr.eoud, eoud_flipped[i], 12);
    intarrcopy(arr.cp,   corner_cycle[i],  8);
    intarrcopy(arr.coud, coud_flipped[i],  8);
    intarrcopy(arr.corl, corl_flipped[i],  8);
    intarrcopy(arr.cofb, cofb_flipped[i],  8);
    intarrcopy(arr.cpos, center_cycle[i],  6);
  }

  if (read)
    if (read_ttables_file())
      return;

  /* Initialize transition tables */
  Cube c = {0};
  PieceFilter fe  = {.epose=true}, fs = {.eposs=true}, fm = {.eposm=true};
  PieceFilter feo = { .eofb = true, .eorl = true, .eoud = true };
  PieceFilter fcp = { .cp = true };
  PieceFilter fco = { .cofb = true, .corl = true, .coud = true };
  PieceFilter fcc = { .cpos = true };
  for (int m = 0; m < NMOVES; m++) {
    for (uint16_t i = 0; i < factorial12/factorial8; i++) {
      c.epose = i; epose_ttable[m][i] = move_via_array(m, c, fe).epose;
      c.eposs = i; eposs_ttable[m][i] = move_via_array(m, c, fs).eposs;
      c.eposm = i; eposm_ttable[m][i] = move_via_array(m, c, fm).eposm;
    }
    for (uint16_t i = 0; i < pow2to11; i++ ) {
      c.eofb = i; eofb_ttable[m][i] = move_via_array(m, c, feo).eofb;
      c.eorl = i; eorl_ttable[m][i] = move_via_array(m, c, feo).eorl;
      c.eoud = i; eoud_ttable[m][i] = move_via_array(m, c, feo).eoud;
    }
    for (uint16_t i = 0; i < factorial8; i++) {
      c.cp = i; cp_ttable[m][i] = move_via_array(m, c, fcp).cp;
    }
    for (uint16_t i = 0; i < pow3to7; i++) {
      c.coud = i; coud_ttable[m][i] = move_via_array(m, c, fco).coud;
      c.corl = i; corl_ttable[m][i] = move_via_array(m, c, fco).corl;
      c.cofb = i; cofb_ttable[m][i] = move_via_array(m, c, fco).cofb;
    }
    for (uint16_t i = 0; i < factorial6; i++) {
      c.cpos = i; cpos_ttable[m][i] = move_via_array(m, c, fcc).cpos;
    }
  }

  if (write) write_ttables_file();
}

Cube move_cube(Move m, Cube cube) {
  Cube moved = cube;

  moved.epose   = epose_ttable[m][cube.epose];
  moved.eposs   = eposs_ttable[m][cube.eposs];
  moved.eposm   = eposm_ttable[m][cube.eposm];
  moved.eofb    = eofb_ttable[m][cube.eofb];
  moved.eorl    = eorl_ttable[m][cube.eorl];
  moved.eoud    = eoud_ttable[m][cube.eoud];
  moved.coud    = coud_ttable[m][cube.coud];
  moved.cofb    = cofb_ttable[m][cube.cofb];
  moved.corl    = corl_ttable[m][cube.corl];
  moved.cp      = cp_ttable[m][cube.cp];
  moved.cpos    = cpos_ttable[m][cube.cpos];

  return moved;
}

Cube apply_alg(NissMove *alg, Cube cube) {
  Cube ret = {0};
  for (int i = 0; alg[i].m != NULLMOVE; i++)
    if (alg[i].inverse)
      ret = move_cube(alg[i].m, ret);
  ret = compose(cube, inverse_cube(ret));

  for (int i = 0; alg[i].m != NULLMOVE; i++)
    if (!alg[i].inverse)
      ret = move_cube(alg[i].m, ret);
  return ret;
}

void init_aux_tables() {
  /* Commute */
  for (int i = 0; i < NMOVES; i++)
    for (int j = 0; j < NMOVES; j++)
      commute[i][j] = equal(move_cube(i, move_cube(j, blank_cube())),
                            move_cube(j, move_cube(i, blank_cube())));

  /* Possible next (if the sequence i j k is valid) */
  for (int i = 0; i < NMOVES; i++)
    for (int j = 0; j < NMOVES; j++)
      for (int k = 0; k < NMOVES; k++)
        possible_next[i][j][k] =
          (j == 0) ||
          (j != 0 && (j-(j-1)%3) != (k-(k-1)%3) &&
          !(i != 0 && commute[i][j] && (i-(i-1)%3) == (k-(k-1)%3)));

  /* Inverse */
  for (int i = 0; i < NMOVES; i++)
    inverse[i] = i == 0 ? 0 : i + 2 - 2*((i-1)%3);
}

