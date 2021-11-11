#include "transformations.h"

int edge_slice(int e); /* Return slice (e=0, s=1, m=2) to which e belongs */
Cube rotate_via_compose(Transformation r, Cube c, PieceFilter f);
bool read_rtables_file();
bool write_rtables_file();

/* Values mod 3 to determine from which side to take the state to convert */
int epose_source[NTRANS];  /* 0 = epose, 1 = eposs, 2 = eposm */
int eposs_source[NTRANS];
int eposm_source[NTRANS];
int eofb_source[NTRANS];   /* 0 = eoud,  1 = eorl,  2 = eofb  */
int eorl_source[NTRANS];
int eoud_source[NTRANS];
int coud_source[NTRANS];   /* 0 = coud,  1 = corl,  2 = cofb  */
int cofb_source[NTRANS];
int corl_source[NTRANS];

/* Transition tables for rotations (n+1 is mirror) */
uint16_t epose_rtable[NTRANS][factorial12/factorial8];
uint16_t eposs_rtable[NTRANS][factorial12/factorial8];
uint16_t eposm_rtable[NTRANS][factorial12/factorial8];
uint16_t eo_rtable[NTRANS][pow2to11];
uint16_t cp_rtable[NTRANS][factorial8];
uint16_t co_rtable[NTRANS][pow3to7];
uint16_t cpos_rtable[NTRANS][factorial6];

/* Same for moves */
uint16_t moves_rtable[NTRANS][NMOVES];

NissMove rotation_niss[NTRANS][6];

int edge_slice(int e) {
  if (e == FR || e == FL || e == BL || e == BR)
    return 0;
  if (e == UR || e == UL || e == DR || e == DL)
    return 1;
  return 2;
}

Cube rotate_via_compose(Transformation r, Cube c, PieceFilter f) {
  if (r != mirror) {
    return apply_alg_filtered(rotation_niss[r], c, f);
  } else {
    static int zero12[12]     = {0,0,0,0,0,0,0,0,0,0,0,0},
               zero8[12]      = {0,0,0,0,0,0,0,0},
               mirror_ep[12]  = {UF,UR,UB,UL,DF,DR,DB,DL,FL,FR,BR,BL},
               mirror_cp[8]   = {UFL, UFR, UBR, UBL, DFL, DFR, DBR, DBL},
               mirror_cpos[6] =
                 {U_center,D_center,L_center,R_center,F_center,B_center};
    return move_via_arrays((CubeArray){
      .ep   = mirror_ep, .eofb = zero12, .eorl = zero12, .eoud = zero12,
      .cp   = mirror_cp, .coud = zero8,  .corl = zero8,  .cofb = zero8,
      .cpos = mirror_cpos}, c, f);
  }
}

bool read_rtables_file() {
  FILE *ttf;
  long unsigned int me[12] = { factorial12/factorial8, factorial12/factorial8,
                   factorial12/factorial8, pow2to11, pow2to11, pow2to11,
                   factorial8, pow3to7, pow3to7, pow3to7, factorial6, NMOVES };
  if ((ttf = fopen("rtables", "rb")) != NULL) {
    bool r = true;
    for (int m = 0; m < NTRANS; m++) {
      r = r && fread(epose_rtable[m], sizeof(uint16_t), me[0],  ttf) == me[0];
      r = r && fread(eposs_rtable[m], sizeof(uint16_t), me[1],  ttf) == me[1];
      r = r && fread(eposm_rtable[m], sizeof(uint16_t), me[2],  ttf) == me[2];
      r = r && fread(eo_rtable[m],    sizeof(uint16_t), me[3],  ttf) == me[3];
      r = r && fread(cp_rtable[m],    sizeof(uint16_t), me[6],  ttf) == me[6];
      r = r && fread(co_rtable[m],    sizeof(uint16_t), me[7],  ttf) == me[7];
      r = r && fread(cpos_rtable[m],  sizeof(uint16_t), me[10], ttf) == me[10];
      r = r && fread(moves_rtable[m], sizeof(uint16_t), me[11], ttf) == me[11];
    }
    fclose(ttf);
    return r;
  } else return false;
}

bool write_rtables_file() {
  FILE *ttf;
  long unsigned int me[12] = { factorial12/factorial8, factorial12/factorial8,
                   factorial12/factorial8, pow2to11, pow2to11, pow2to11,
                   factorial8, pow3to7, pow3to7, pow3to7, factorial6, NMOVES };
  if ((ttf = fopen("rtables", "wb")) != NULL) {
    bool r = true;
    for (int m = 0; m < NTRANS; m++) {
      r = r && fwrite(epose_rtable[m], sizeof(uint16_t), me[0], ttf) == me[0];
      r = r && fwrite(eposs_rtable[m], sizeof(uint16_t), me[1], ttf) == me[1];
      r = r && fwrite(eposm_rtable[m], sizeof(uint16_t), me[2], ttf) == me[2];
      r = r && fwrite(eo_rtable[m],    sizeof(uint16_t), me[3], ttf) == me[3];
      r = r && fwrite(cp_rtable[m],    sizeof(uint16_t), me[6], ttf) == me[6];
      r = r && fwrite(co_rtable[m],    sizeof(uint16_t), me[7], ttf) == me[7];
      r = r && fwrite(cpos_rtable[m],  sizeof(uint16_t), me[10],ttf) == me[10];
      r = r && fwrite(moves_rtable[m], sizeof(uint16_t), me[11],ttf) == me[11];
    }
    fclose(ttf);
    return r;
  } else return false;
}

void init_transformations(bool read, bool write) {
  /* Compute sources */
  for (int i = 0; i < NTRANS; i++) {
    Cube cube = {0};
    if (i != mirror)
      cube = apply_alg(rotation_algs[i], (Cube){0});
    epose_source[i] = edge_slice(edge_at(cube, FR));
    eposs_source[i] = edge_slice(edge_at(cube, UR));
    eposm_source[i] = edge_slice(edge_at(cube, UF));
    eofb_source[i]  = center_at(cube, F_center)/2;
    eorl_source[i]  = center_at(cube, R_center)/2;
    eoud_source[i]  = center_at(cube, U_center)/2;
    coud_source[i]  = center_at(cube, U_center)/2;
    cofb_source[i]  = center_at(cube, F_center)/2;
    corl_source[i]  = center_at(cube, R_center)/2;
  }

  /*TODO: maybe move down*/
  /* Compute rotation_niss array, necessary for rotate_via_compose */
  for (int r = 0; r != mirror; r++) {
    concat(rotation_algs[r], rotation_algs[r], rotation_niss[r]);
    for (int i = len(rotation_algs[r]); rotation_niss[r][i].m != NULLMOVE; i++)
      rotation_niss[r][i].inverse = true;
  }

  /* If I can read tables from file, I stop here */
  if (read)
    if (read_rtables_file())
      return;
  
  /* Initialize tables */
  for (int m = 0; m < NTRANS; m++) {
    int eparr[12] = {0,0,0,0,0,0,0,0,0,0,0,0}, cparr[8] = {0,0,0,0,0,0,0,0};
    CubeArray epcp = { .ep = eparr, .cp = cparr };
    cube_to_arrays(apply_alg(rotation_algs[m], (Cube){0}), &epcp,
                  (PieceFilter){.epose=true,.eposs=true,.eposm=true,.cp=true});
    for (uint16_t i = 0; i < factorial12/factorial8; i++) {
      Cube c[3] = { admissible_ep((Cube){ .epose = i}, pf_e),
                    admissible_ep((Cube){ .eposs = i}, pf_s),
                    admissible_ep((Cube){ .eposm = i}, pf_m) };
      epose_rtable[m][i]=rotate_via_compose(m,c[epose_source[m]],pf_ep).epose;
      eposs_rtable[m][i]=rotate_via_compose(m,c[eposs_source[m]],pf_ep).eposs;
      eposm_rtable[m][i]=rotate_via_compose(m,c[eposm_source[m]],pf_ep).eposm;
    }
    for (uint16_t i = 0; i < pow2to11; i++ ) {
      int eoarr[12];
      int_to_sum_zero_array(i, 2, 12, eoarr);
      apply_permutation(eparr, eoarr, 12);
      eo_rtable[m][i] = digit_array_to_int(eoarr, 11, 2);
    }
    for (uint16_t i = 0; i < pow3to7; i++) {
      int coarr[12];
      int_to_sum_zero_array(i, 3, 8, coarr);
      apply_permutation(cparr, coarr, 8);
      co_rtable[m][i] = digit_array_to_int(coarr, 7, 3);
    }
    for (uint16_t i = 0; i < factorial8; i++)
      cp_rtable[m][i] = rotate_via_compose(m, (Cube){.cp=i}, pf_cp).cp;
    for (uint16_t i = 0; i < factorial6; i++)
      cpos_rtable[m][i] = rotate_via_compose(m, (Cube){.cpos=i}, pf_cpos).cpos;
    for (Move i = 0; i < NMOVES; i++) {
      Cube aux = transform_cube(m, move_cube(i, (Cube){0}));
      for (Move move = 0; move < NMOVES; move++)
        if (is_solved(move_cube(inverse[move], aux)))
          moves_rtable[m][i] = move;
    }
  }

  if (write)
    if (!write_rtables_file())
      printf("Error in writing rtables: file not writable\n");
}

Cube transform_cube(Transformation t, Cube cube) {
  Cube transformed = {0};

  uint16_t aux_epos[3] = { cube.epose, cube.eposs, cube.eposm },
           aux_eo[3]   = { cube.eoud,  cube.eorl,  cube.eofb  },
           aux_co[3]   = { cube.coud,  cube.corl,  cube.cofb  };

  transformed.epose = epose_rtable[t][aux_epos[epose_source[t]]];
  transformed.eposs = eposs_rtable[t][aux_epos[eposs_source[t]]];
  transformed.eposm = eposm_rtable[t][aux_epos[eposm_source[t]]];
  transformed.eofb  = eo_rtable[t][aux_eo[eofb_source[t]]];
  transformed.eorl  = eo_rtable[t][aux_eo[eorl_source[t]]];
  transformed.eoud  = eo_rtable[t][aux_eo[eoud_source[t]]];
  transformed.coud  = co_rtable[t][aux_co[coud_source[t]]];
  transformed.corl  = co_rtable[t][aux_co[corl_source[t]]];
  transformed.cofb  = co_rtable[t][aux_co[cofb_source[t]]];
  transformed.cp    = cp_rtable[t][cube.cp];
  transformed.cpos  = cpos_rtable[t][cube.cpos];

  return transformed;
}

void transform_alg(Transformation t, NissMove *alg) {
  for (int i = 0; alg[i].m; i++)
    alg[i].m = moves_rtable[t][alg[i].m];
}
