#include <stdint.h>
#include "pruning_tables.h"
#include "moves.h"

/* The data contained in e.g. eofb pruning table is the same that is contained
 * in eolr pruning table and so on. For small tables the memory wasted is not
 * too much and it makes things easier. I may change this when I implement
 * bigger tables. */
int eofb_pruning_table[pow2to11];
int eorl_pruning_table[pow2to11];
int eoud_pruning_table[pow2to11];
int coud_pruning_table[pow3to7];
int cofb_pruning_table[pow3to7];
int corl_pruning_table[pow3to7];
int cp_pruning_table[factorial8];

int eorl_from_eofb_pruning_table[pow2to11];
int eoud_from_eofb_pruning_table[pow2to11];
int eoud_from_eorl_pruning_table[pow2to11];
int eofb_from_eorl_pruning_table[pow2to11];
int eofb_from_eoud_pruning_table[pow2to11];
int eorl_from_eoud_pruning_table[pow2to11];

int coud_from_eofb_pruning_table[pow3to7];
int coud_from_eorl_pruning_table[pow3to7];
int cofb_from_eorl_pruning_table[pow3to7];
int cofb_from_eoud_pruning_table[pow3to7];
int corl_from_eoud_pruning_table[pow3to7];
int corl_from_eofb_pruning_table[pow3to7];

int cp_drud_pruning_table[factorial8];
int cp_drfb_pruning_table[factorial8];
int cp_drrl_pruning_table[factorial8];
int epud_pruning_table[factorial8];
int epfb_pruning_table[factorial8];
int eprl_pruning_table[factorial8];

int cp_htr_pruning_table[factorial8];

int cpud_to_htr_pruning_table[factorial8];
int cpfb_to_htr_pruning_table[factorial8];
int cprl_to_htr_pruning_table[factorial8];


/* About 1Mb each */
int8_t eofb_epose_pruning_table[pow2to11][binom12on4];
int8_t eorl_eposs_pruning_table[pow2to11][binom12on4];
int8_t eoud_eposm_pruning_table[pow2to11][binom12on4];

/* About 4.5Mb each */
int8_t eofb_coud_pruning_table[pow2to11][pow3to7];
int8_t eofb_corl_pruning_table[pow2to11][pow3to7];
int8_t eorl_coud_pruning_table[pow2to11][pow3to7];
int8_t eorl_cofb_pruning_table[pow2to11][pow3to7];
int8_t eoud_cofb_pruning_table[pow2to11][pow3to7];
int8_t eoud_corl_pruning_table[pow2to11][pow3to7];

/* About 1Mb each */
int8_t coud_epose_from_eofb_pruning_table[pow3to7][binom12on4];
int8_t cofb_eposs_from_eorl_pruning_table[pow3to7][binom12on4];
int8_t corl_eposm_from_eoud_pruning_table[pow3to7][binom12on4];
int8_t coud_epose_from_eorl_pruning_table[pow3to7][binom12on4];
int8_t cofb_eposs_from_eoud_pruning_table[pow3to7][binom12on4];
int8_t corl_eposm_from_eofb_pruning_table[pow3to7][binom12on4];


/* Firs one is 88Mb, second one is 71Mb */
int8_t cp_co_pruning_table[factorial8][pow3to7];
int8_t triple_eo_pruning_table[pow2to11][binom12on4*binom8on4];

int initialized_small = 0;
int initialized_directdr = 0;
int initialized_drfromeo = 0;
int initialized_huge = 0;

void init_single_table(int n, int t_tab[][19], int p_tab[n], int mask) {
  int state[n];
  state[0] = 0;  /* 0 should always be the solved state. */
  p_tab[0] = 0;
  int state_count = 1;
  for (int i = 0; i < state_count; i++) {
    for (int m = 1; m < 19; m++) {
      int next = t_tab[state[i]][m];
      if (mask & (1<<m) && !p_tab[next] && next) {
        p_tab[next] = p_tab[state[i]] + 1;
        state[state_count++] = next;
      }
    }
  }
}

/* Similar to single table, but specific to "cp to htr".
 * The idea is that we are considering the distance not necessarily to the
 * solved state, but to any state that is either solved or reachable from
 * cp_pruning_table. */
void init_cptohtr_table(int n, int t_tab[][19], int p_tab[n], int mask) {

  for (int i = 0; i < n; i++)
    p_tab[i] = 21;
  
  /* List of htr states */
  int good[n]; good[0] = 0;
  int good_count = 1;
  for (int i = 0; i < n; i++)
    if (cp_htr_pruning_table[i])
      good[good_count++] = i;
  
  /* Init pruning table starting from each possible state */
  int state[n];
  for (int j = 0; j < good_count; j++) {
    state[0] = good[j];
    p_tab[state[0]] = 0;
    int state_count = 1;
    for (int i = 0; i < state_count; i++) {
      for (int m = 1; m < 19; m++) {
        int next = t_tab[state[i]][m];
        if (mask & (1<<m) && (p_tab[next] > p_tab[state[i]] + 1) && next) {
          p_tab[next] = p_tab[state[i]] + 1;
          state[state_count++] = next;
        }
      }
    }
  }
}

void init_double_table(int n1, int n2,
                       int t_table1[n1][19], int t_table2[n2][19],
                       int8_t p_table[n1][n2], int mask) {
  static int state1[factorial8*pow3to7], state2[factorial8*pow3to7];
  state1[0] = 0;
  state2[0] = 0;
  p_table[0][0] = 0;
  int state_count = 1;
  for (int i = 0; i < state_count; i++) {
    for (int m = 1; m < 19; m++) {
      int next1 = t_table1[state1[i]][m];
      int next2 = t_table2[state2[i]][m];
      if (mask & (1<<m) && !p_table[next1][next2] && (next1 || next2)) {
        p_table[next1][next2] = p_table[state1[i]][state2[i]] + 1;
        state1[state_count] = next1;
        state2[state_count] = next2;
        state_count++;
      }
    }
  }
}

void init_eofb_pruning_table() {
  init_single_table(pow2to11, eofb_transition_table, eofb_pruning_table,
                    move_mask_all);
}

void init_eorl_pruning_table() {
  init_single_table(pow2to11, eorl_transition_table, eorl_pruning_table,
                    move_mask_all);
}

void init_eoud_pruning_table() {
  init_single_table(pow2to11, eoud_transition_table, eoud_pruning_table,
                    move_mask_all);
}

void init_coud_pruning_table() {
  init_single_table(pow3to7, coud_transition_table, coud_pruning_table,
                    move_mask_all);
}

void init_cofb_pruning_table() {
  init_single_table(pow3to7, cofb_transition_table, cofb_pruning_table,
                    move_mask_all);
}

void init_corl_pruning_table() {
  init_single_table(pow3to7, corl_transition_table, corl_pruning_table,
                    move_mask_all);
}

void init_cp_pruning_table() {
  init_single_table(factorial8, cp_transition_table, cp_pruning_table,
                    move_mask_all);
}

/* The following tables use the eo moveset */
void init_eorl_from_eofb_pruning_table() {
  init_single_table(pow2to11, eorl_transition_table,
                    eorl_from_eofb_pruning_table, move_mask_eofb);
}

void init_eoud_from_eofb_pruning_table() {
  init_single_table(pow2to11, eoud_transition_table,
                    eoud_from_eofb_pruning_table, move_mask_eofb);
}

void init_eoud_from_eorl_pruning_table() {
  init_single_table(pow2to11, eoud_transition_table,
                    eoud_from_eorl_pruning_table, move_mask_eorl);
}

void init_eofb_from_eorl_pruning_table() {
  init_single_table(pow2to11, eofb_transition_table,
                    eofb_from_eorl_pruning_table, move_mask_eorl);
}

void init_eofb_from_eoud_pruning_table() {
  init_single_table(pow2to11, eofb_transition_table,
                    eofb_from_eoud_pruning_table, move_mask_eoud);
}

void init_eorl_from_eoud_pruning_table() {
  init_single_table(pow2to11, eorl_transition_table,
                    eorl_from_eoud_pruning_table, move_mask_eoud);
}

void init_coud_from_eofb_pruning_table() {
  init_single_table(pow3to7, coud_transition_table,
                    coud_from_eofb_pruning_table, move_mask_eofb);
}

void init_corl_from_eofb_pruning_table() {
  init_single_table(pow3to7, corl_transition_table,
                    corl_from_eofb_pruning_table, move_mask_eofb);
}

void init_coud_from_eorl_pruning_table() {
  init_single_table(pow3to7, coud_transition_table,
                    coud_from_eorl_pruning_table, move_mask_eorl);
}

void init_cofb_from_eorl_pruning_table() {
  init_single_table(pow3to7, cofb_transition_table,
                    cofb_from_eorl_pruning_table, move_mask_eorl);
}

void init_corl_from_eoud_pruning_table() {
  init_single_table(pow3to7, corl_transition_table,
                    corl_from_eoud_pruning_table, move_mask_eoud);
}

void init_cofb_from_eoud_pruning_table() {
  init_single_table(pow3to7, cofb_transition_table,
                    cofb_from_eoud_pruning_table, move_mask_eoud);
}

/* The following tables always use DR moveset */
void init_epud_pruning_table() {
  init_single_table(factorial8, epud_transition_table, epud_pruning_table,
                    move_mask_drud);
}

void init_epfb_pruning_table() {
  init_single_table(factorial8, epfb_transition_table, epfb_pruning_table,
                    move_mask_drfb);
}

void init_eprl_pruning_table() {
  init_single_table(factorial8, eprl_transition_table, eprl_pruning_table,
                    move_mask_drrl);
}

void init_cp_drud_pruning_table() {
  init_single_table(factorial8, cp_transition_table, cp_drud_pruning_table,
                    move_mask_drud);
}

void init_cp_drfb_pruning_table() {
  init_single_table(factorial8, cp_transition_table, cp_drfb_pruning_table,
                    move_mask_drfb);
}

void init_cp_drrl_pruning_table() {
  init_single_table(factorial8, cp_transition_table, cp_drrl_pruning_table,
                    move_mask_drrl);
}

void init_cp_htr_table() {
  init_single_table(factorial8, cp_transition_table, cp_htr_pruning_table,
                    move_mask_htr);
}

void init_cpud_to_htr_table() {
  init_cptohtr_table(factorial8, cp_transition_table,
                     cpud_to_htr_pruning_table, move_mask_drud);
}

void init_cpfb_to_htr_table() {
  init_cptohtr_table(factorial8, cp_transition_table,
                     cpfb_to_htr_pruning_table, move_mask_drfb);
}

void init_cprl_to_htr_table() {
  init_cptohtr_table(factorial8, cp_transition_table,
                     cprl_to_htr_pruning_table, move_mask_drrl);
}


void init_eofb_epose_pruning_table() {
  init_double_table(pow2to11, binom12on4,
                    eofb_transition_table, epose_transition_table,
                    eofb_epose_pruning_table, move_mask_all);
}

void init_eorl_eposs_pruning_table() {
  init_double_table(pow2to11, binom12on4,
                    eorl_transition_table, eposs_transition_table,
                    eorl_eposs_pruning_table, move_mask_all);
}

void init_eoud_eposm_pruning_table() {
  init_double_table(pow2to11, binom12on4,
                    eoud_transition_table, eposm_transition_table,
                    eoud_eposm_pruning_table, move_mask_all);
}



void init_eofb_coud_pruning_table() {
  init_double_table(pow2to11, pow3to7,
                    eofb_transition_table, coud_transition_table,
                    eofb_coud_pruning_table, move_mask_all);
}

void init_eofb_corl_pruning_table() {
  init_double_table(pow2to11, pow3to7,
                    eofb_transition_table, corl_transition_table,
                    eofb_corl_pruning_table, move_mask_all);
}

void init_eorl_coud_pruning_table() {
  init_double_table(pow2to11, pow3to7,
                    eorl_transition_table, coud_transition_table,
                    eorl_coud_pruning_table, move_mask_all);
}

void init_eorl_cofb_pruning_table() {
  init_double_table(pow2to11, pow3to7,
                    eorl_transition_table, cofb_transition_table,
                    eorl_cofb_pruning_table, move_mask_all);
}

void init_eoud_corl_pruning_table() {
  init_double_table(pow2to11, pow3to7,
                    eoud_transition_table, corl_transition_table,
                    eoud_corl_pruning_table, move_mask_all);
}

void init_eoud_cofb_pruning_table() {
  init_double_table(pow2to11, pow3to7,
                    eoud_transition_table, cofb_transition_table,
                    eoud_cofb_pruning_table, move_mask_all);
}

void init_coud_epose_from_eofb_pruning_table() {
  init_double_table(pow3to7, binom12on4,
                    coud_transition_table, epose_transition_table,
                    coud_epose_from_eofb_pruning_table, move_mask_eofb);
}

void init_cofb_eposs_from_eorl_pruning_table() {
  init_double_table(pow3to7, binom12on4,
                    cofb_transition_table, eposs_transition_table,
                    cofb_eposs_from_eorl_pruning_table, move_mask_eorl);
}

void init_corl_eposm_from_eoud_pruning_table() {
  init_double_table(pow3to7, binom12on4,
                    corl_transition_table, eposm_transition_table,
                    corl_eposm_from_eoud_pruning_table, move_mask_eoud);
}

void init_coud_epose_from_eorl_pruning_table() {
  init_double_table(pow3to7, binom12on4,
                    coud_transition_table, epose_transition_table,
                    coud_epose_from_eorl_pruning_table, move_mask_eorl);
}

void init_cofb_eposs_from_eoud_pruning_table() {
  init_double_table(pow3to7, binom12on4,
                    cofb_transition_table, eposs_transition_table,
                    cofb_eposs_from_eoud_pruning_table, move_mask_eoud);
}

void init_corl_eposm_from_eofb_pruning_table() {
  init_double_table(pow3to7, binom12on4,
                    corl_transition_table, eposm_transition_table,
                    corl_eposm_from_eofb_pruning_table, move_mask_eofb);
}

void init_cp_co_pruning_table() {
  init_double_table(factorial8, pow3to7,
                    cp_transition_table, coud_transition_table,
                    cp_co_pruning_table, move_mask_all);
}

void init_triple_eo_pruning_table() {
  init_double_table(pow2to11, binom12on4*binom8on4,
                    eofb_transition_table, emslices_transition_table,
                    triple_eo_pruning_table, move_mask_all);
}


void init_small_pruning_tables() {
  if (initialized_small)
    return;

  init_eofb_pruning_table();
  init_eorl_pruning_table();
  init_eoud_pruning_table();
  init_coud_pruning_table();
  init_cofb_pruning_table();
  init_corl_pruning_table();
  init_cp_pruning_table();

  init_eorl_from_eofb_pruning_table();
  init_eoud_from_eofb_pruning_table();
  init_eoud_from_eorl_pruning_table();
  init_eofb_from_eorl_pruning_table();
  init_eofb_from_eoud_pruning_table();
  init_eorl_from_eoud_pruning_table();

  init_coud_from_eofb_pruning_table();
  init_corl_from_eofb_pruning_table();
  init_coud_from_eorl_pruning_table();
  init_cofb_from_eorl_pruning_table();
  init_cofb_from_eoud_pruning_table();
  init_corl_from_eoud_pruning_table();

  init_epud_pruning_table();
  init_epfb_pruning_table();
  init_eprl_pruning_table();
  init_cp_drud_pruning_table();
  init_cp_drfb_pruning_table();
  init_cp_drrl_pruning_table();

  init_cp_htr_table();
  init_cpud_to_htr_table();
  init_cpfb_to_htr_table();
  init_cprl_to_htr_table();

  initialized_small = 1;
}

void init_directdr_pruning_tables() {
  if (initialized_directdr)
    return;
  
  init_eofb_epose_pruning_table();
  init_eorl_eposs_pruning_table();
  init_eoud_eposm_pruning_table();

  init_eofb_coud_pruning_table();
  init_eofb_corl_pruning_table();
  init_eorl_coud_pruning_table();
  init_eorl_cofb_pruning_table();
  init_eoud_cofb_pruning_table();
  init_eoud_corl_pruning_table();

  initialized_directdr = 1;
}

void init_drfromeo_pruning_tables() {
  if (initialized_drfromeo)
    return;

  init_coud_epose_from_eofb_pruning_table();
  init_cofb_eposs_from_eorl_pruning_table();
  init_corl_eposm_from_eoud_pruning_table();
  init_coud_epose_from_eorl_pruning_table();
  init_cofb_eposs_from_eoud_pruning_table();
  init_corl_eposm_from_eofb_pruning_table();

  initialized_drfromeo = 1;
}

void init_huge_pruning_tables() {
  if (initialized_huge)
    return;
  
  init_cp_co_pruning_table();
  init_triple_eo_pruning_table();

  initialized_huge = 1;
}
  
