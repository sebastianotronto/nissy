#include "transformations.h"

int edge_slice(int e); /* Return slice (e=0, s=1, m=2) to which e belongs */
Cube rotate_via_compose(Transformation r, Cube c);
bool read_rtables_file();
bool write_rtables_file();

/* Values mod 3 to determine from which side to take the state to convert */
int epose_source[NROTATIONS];  /* 0 = epose, 1 = eposs, 2 = eposm */
int eposs_source[NROTATIONS];
int eposm_source[NROTATIONS];
int eofb_source[NROTATIONS];   /* 0 = eoud,  1 = eorl,  2 = eofb  */
int eorl_source[NROTATIONS];
int eoud_source[NROTATIONS];
int coud_source[NROTATIONS];   /* 0 = coud,  1 = corl,  2 = cofb  */
int cofb_source[NROTATIONS];
int corl_source[NROTATIONS];

/* Transition tables for rotations (n+1 is mirror) */
uint16_t epose_rtable[NROTATIONS][factorial12/factorial8];
uint16_t eposs_rtable[NROTATIONS][factorial12/factorial8];
uint16_t eposm_rtable[NROTATIONS][factorial12/factorial8];
uint16_t eo_rtable[NROTATIONS][pow2to11];
/*uint16_t eofb_rtable[NROTATIONS][pow2to11];
uint16_t eorl_rtable[NROTATIONS][pow2to11];
uint16_t eoud_rtable[NROTATIONS][pow2to11];*/
uint16_t cp_rtable[NROTATIONS][factorial8];
uint16_t co_rtable[NROTATIONS][pow3to7];
/*uint16_t coud_rtable[NROTATIONS][pow3to7];
uint16_t cofb_rtable[NROTATIONS][pow3to7];
uint16_t corl_rtable[NROTATIONS][pow3to7];*/
uint16_t cpos_rtable[NROTATIONS][factorial6];

/* Same for moves */
uint16_t move_rtable[NROTATIONS][NMOVES];

NissMove rotation_niss[NROTATIONS][6];

int edge_slice(int e) {
  if (e == FR || e == FL || e == BL || e == BR)
    return 0;
  if (e == UR || e == UL || e == DR || e == DL)
    return 1;
  return 2;
}

Cube rotate_via_compose(Transformation r, Cube c) {
  if (r != mirror) {
    return apply_alg(rotation_niss[r], c);
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
      .cpos = mirror_cpos}, c, pf_all);
  }
}

bool read_rtables_file() {
  FILE *ttf;
  long unsigned int me[12] = { factorial12/factorial8, factorial12/factorial8,
                   factorial12/factorial8, pow2to11, pow2to11, pow2to11,
                   factorial8, pow3to7, pow3to7, pow3to7, factorial6, NMOVES };
  if ((ttf = fopen("rtables", "rb")) != NULL) {
    bool r = true;
    for (int m = 0; m < NROTATIONS; m++) {
      r = r && fread(epose_rtable[m], sizeof(uint16_t), me[0],  ttf) == me[0];
      r = r && fread(eposs_rtable[m], sizeof(uint16_t), me[1],  ttf) == me[1];
      r = r && fread(eposm_rtable[m], sizeof(uint16_t), me[2],  ttf) == me[2];
      r = r && fread(eo_rtable[m],    sizeof(uint16_t), me[3],  ttf) == me[3];
      /*r = r && fread(eofb_rtable[m],  sizeof(uint16_t), me[3],  ttf) == me[3];
      r = r && fread(eorl_rtable[m],  sizeof(uint16_t), me[4],  ttf) == me[4];
      r = r && fread(eoud_rtable[m],  sizeof(uint16_t), me[5],  ttf) == me[5];*/
      r = r && fread(cp_rtable[m],    sizeof(uint16_t), me[6],  ttf) == me[6];
      r = r && fread(co_rtable[m],    sizeof(uint16_t), me[7],  ttf) == me[7];
      /*r = r && fread(coud_rtable[m],  sizeof(uint16_t), me[7],  ttf) == me[7];
      r = r && fread(corl_rtable[m],  sizeof(uint16_t), me[8],  ttf) == me[8];
      r = r && fread(cofb_rtable[m],  sizeof(uint16_t), me[9],  ttf) == me[9];*/
      r = r && fread(cpos_rtable[m],  sizeof(uint16_t), me[10], ttf) == me[10];
      r = r && fread(move_rtable[m],  sizeof(uint16_t), me[11], ttf) == me[11];
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
    for (int m = 0; m < NROTATIONS; m++) {
      r = r && fwrite(epose_rtable[m], sizeof(uint16_t), me[0], ttf) == me[0];
      r = r && fwrite(eposs_rtable[m], sizeof(uint16_t), me[1], ttf) == me[1];
      r = r && fwrite(eposm_rtable[m], sizeof(uint16_t), me[2], ttf) == me[2];
      r = r && fwrite(eo_rtable[m],    sizeof(uint16_t), me[3], ttf) == me[3];
      /*r = r && fwrite(eofb_rtable[m],  sizeof(uint16_t), me[3], ttf) == me[3];
      r = r && fwrite(eorl_rtable[m],  sizeof(uint16_t), me[4], ttf) == me[4];
      r = r && fwrite(eoud_rtable[m],  sizeof(uint16_t), me[5], ttf) == me[5];*/
      r = r && fwrite(cp_rtable[m],    sizeof(uint16_t), me[6], ttf) == me[6];
      r = r && fwrite(co_rtable[m],    sizeof(uint16_t), me[7], ttf) == me[7];
      /*r = r && fwrite(coud_rtable[m],  sizeof(uint16_t), me[7], ttf) == me[7];
      r = r && fwrite(corl_rtable[m],  sizeof(uint16_t), me[8], ttf) == me[8];
      r = r && fwrite(cofb_rtable[m],  sizeof(uint16_t), me[9], ttf) == me[9];*/
      r = r && fwrite(cpos_rtable[m],  sizeof(uint16_t), me[10],ttf) == me[10];
      r = r && fwrite(move_rtable[m],  sizeof(uint16_t), me[11],ttf) == me[11];
    }
    fclose(ttf);
    return r;
  } else return false;
}

void init_transformations(bool read, bool write) {
  /* Compute sources */
  for (int i = 0; i < NROTATIONS; i++) {
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
  for (int m = 0; m < NROTATIONS; m++) {
    int eparr[12] = {0,0,0,0,0,0,0,0,0,0,0,0}, cparr[8] = {0,0,0,0,0,0,0,0};
    CubeArray epcp = { .ep = eparr, .cp = cparr };
    cube_to_arrays(apply_alg(rotation_algs[m], (Cube){0}), &epcp,
                  (PieceFilter){.epose=true,.eposs=true,.eposm=true,.cp=true});
    for (uint16_t i = 0; i < factorial12/factorial8; i++) {
      Cube c[3] = { admissible_ep((Cube){ .epose = i}, pf_e),
                    admissible_ep((Cube){ .eposs = i}, pf_s),
                    admissible_ep((Cube){ .eposm = i}, pf_m) };
      epose_rtable[m][i] = rotate_via_compose(m, c[epose_source[m]]).epose;
      eposs_rtable[m][i] = rotate_via_compose(m, c[eposs_source[m]]).eposs;
      eposm_rtable[m][i] = rotate_via_compose(m, c[eposm_source[m]]).eposm;
    }
    for (uint16_t i = 0; i < pow2to11; i++ ) {
      int eoarr[12];
      int_to_sum_zero_array(i, 2, 12, eoarr);
      apply_permutation(eparr, eoarr, 12);
      eo_rtable[m][i] = digit_array_to_int(eoarr, 11, 2);
      /*Cube c[3] = {(Cube){.eoud=i}, (Cube){.eorl=i}, (Cube){.eofb=i}};
      eofb_rtable[m][i] = apply_alg(rotation_algs[m], (Cube){.eofb=i}).eofb;
      eorl_rtable[m][i] = rotate_via_compose(m, c[eorl_source[m]]).eorl;
      eoud_rtable[m][i] = rotate_via_compose(m, c[eoud_source[m]]).eoud;*/
    }
    for (uint16_t i = 0; i < pow3to7; i++) {
      int coarr[12];
      int_to_sum_zero_array(i, 3, 8, coarr);
      apply_permutation(cparr, coarr, 8);
      co_rtable[m][i] = digit_array_to_int(coarr, 8, 3);
      /*Cube c[3] = {(Cube){.coud=i}, (Cube){.corl=i}, (Cube){.cofb=i}};
      coud_rtable[m][i] = rotate_via_compose(m, c[coud_source[m]]).coud;
      corl_rtable[m][i] = rotate_via_compose(m, c[corl_source[m]]).corl;
      cofb_rtable[m][i] = rotate_via_compose(m, c[cofb_source[m]]).cofb;*/
    }
    for (uint16_t i = 0; i < factorial8; i++)
      cp_rtable[m][i] = rotate_via_compose(m, (Cube){.cp=i}).cp;
    for (uint16_t i = 0; i < factorial6; i++)
      cpos_rtable[m][i] = rotate_via_compose(m, (Cube){.cpos=i}).cpos;
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

/*
  printf("%d\n", coud_source[t]);
  int ccc[8];
  int_to_sum_zero_array(cube.cofb, 3, 8, ccc);
  for (int i = 0; i < 8; i++)
    printf("%d ", ccc[i]);
  printf("\n");
  */

  return transformed;
}
