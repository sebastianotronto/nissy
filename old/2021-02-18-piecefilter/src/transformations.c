#include "transformations.h"
/*
Cube apply_rotation_alg(Rotation r, Cube c);
void apply_rotation_cube(NissMove *alg);
Cube apply_mirror_cube(Cube c);
void apply_mirror_alg(NissMove *alg);
*/

void compute_sources();
Cube rotate_via_compose(Rotation r, Cube c);

/* Values mod 3 to determine from which side to take the state to convert */
int epose_source[NROTATIONS];  /* 0 = epose, 1 = eposs, 2 = eposm */
int eposs_source[NROTATIONS];
int eposm_source[NROTATIONS];
int eofb_source[NROTATIONS];   /* 0 = eofb,  1 = eorl,  2 = eoud  */
int eorl_source[NROTATIONS];
int eoud_source[NROTATIONS];
int coud_source[NROTATIONS];   /* 0 = coud,  1 = cofb,  2 = corl  */
int cofb_source[NROTATIONS];
int corl_source[NROTATIONS];

/* Transition tables for rotations */
uint16_t epose_rtable[NROTATIONS][factorial12/factorial8];
uint16_t eposs_rtable[NROTATIONS][factorial12/factorial8];
uint16_t eposm_rtable[NROTATIONS][factorial12/factorial8];
uint16_t eofb_rtable[NROTATIONS][pow2to11];
uint16_t eorl_rtable[NROTATIONS][pow2to11];
uint16_t eoud_rtable[NROTATIONS][pow2to11];
uint16_t cp_rtable[NROTATIONS][factorial8];
uint16_t coud_rtable[NROTATIONS][pow3to7];
uint16_t cofb_rtable[NROTATIONS][pow3to7];
uint16_t corl_rtable[NROTATIONS][pow3to7];

/* Transition tables for mirror */
uint16_t epose_mtable[factorial12/factorial8];
uint16_t eposs_mtable[factorial12/factorial8];
uint16_t eposm_mtable[factorial12/factorial8];
uint16_t eofb_mtable[pow2to11];
uint16_t eorl_mtable[pow2to11];
uint16_t eoud_mtable[pow2to11];
uint16_t cp_mtable[factorial8];
uint16_t coud_mtable[pow3to7];
uint16_t cofb_mtable[pow3to7];
uint16_t corl_mtable[pow3to7];

/* Same for moves */
uint16_t move_rtable[NROTATIONS][NMOVES];
uint16_t move_mtable[NMOVES];

/* Applying a rotation to the cube is equivalent to applying m (m) */
Move equiv_moves[NROTATIONS][3] = {
  [uf] = {0,0,0},  [ur] = {y,0,0},  [ub] = {y2,0,0},  [ul] = {y3,0,0},
  [df] = {z2,0,0}, [dr] = {y,z2,0}, [db] = {y2,z2,0}, [dl] = {y3,z2,0},
  [rf] = {z3,0,0}, [rd] = {z3,y,0}, [rb] = {z3,y2,0}, [ru] = {z3,y3,0},
  [lf] = {z,0,0},  [ld] = {z,y3,0}, [lb] = {z,y2,0},  [lu] = {z,y,0},
  [fu] = {x,y2,0}, [fr] = {x,y,0},  [fd] = {x,0,0},   [fl] = {x,y3,0},
  [bu] = {x3,0,0}, [br] = {x3,y,0}, [bd] = {x3,y2,0}, [bl] = {x3,y3,0},
};

int mirror_ep[12]  = {UF, UR, UB, UL, DF, DR, DB, DL, FL, FR, BR, BL};
int mirror_cp[8]   = {UFL, UFR, UBR, UBL, DFL, DFR, DBR, DBL};
int mirror_cpos[6] = {U_center,D_center,L_center,R_center,F_center,B_center};

void compute_sources() {
  /* epos{e,s,m} */
  CubeArray arr;
  for (int i = 0; i < NROTATIONS; i++) {
    cube_to_arrays(move_cube(equiv_moves[i][1],
                   move_cube(equiv_moves[i][0], blank_cube())), &arr, fAll);
    epose_source[i] = edge_slice(arr.ep[FR]);
    eposs_source[i] = edge_slice(arr.ep[UR]);
    eposm_source[i] = edge_slice(arr.ep[UF]);
    eofb_source[i] = 2 - center_axis(arr.cpos[F_center]);
    eorl_source[i] = 2 - center_axis(arr.cpos[R_center]);
    eoud_source[i] = 2 - center_axis(arr.cpos[U_center]);
    coud_source[i] = (2 * center_axis(arr.cpos[U_center])) % 3;
    cofb_source[i] = (2 * center_axis(arr.cpos[F_center])) % 3;
    corl_source[i] = (2 * center_axis(arr.cpos[R_center])) % 3;
  }
}

/* Use Piecefilter? */
Cube rotate_via_compose(Rotation r, Cube c) {
/* TODO: if this is slow, change compose() in cube.{c,h} to use PieceFilter */
  int j = 0;
  NissMove nm[10] = {0};
  for (int i = 1; i >= 0; i--) {
    if (equiv_moves[r][i] != NULLMOVE) {
      nm[j].inverse = true;
      nm[j++].m = inverse[equiv_moves[r][i]];
    }
  }
  for (int i = 0; equiv_moves[r][i] != NULLMOVE; i++) {
    nm[j].inverse = false;
    nm[j++].m = equiv_moves[r][i];
  }

  return apply_alg(nm, c);
}

/* Use PieceFilter? Not really necessary, but could be nice */
Cube mirror_via_cubearray(Cube c) {
