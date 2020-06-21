#include <stdint.h>
#include <stdio.h>

#include "utils.h"
#include "coordinates.h"
#include "moves.h"
#include "io.h"
#include "pruning_tables.h"

/* Applies inverse of moves, inverse of prev_moves and then inverse of scramble
 * and returns a coordinate determined by t_table. */
int premoves_inverse(int moves[21], int scramble[], int prev_moves[21],
                     int t_table[][19]) {
  int nprevmoves, nmoves, nscramble, coord = 0;

  for (nmoves = 0; moves[nmoves]; nmoves++);
  for (nprevmoves = 0; prev_moves[nprevmoves]; nprevmoves++);
  for (nscramble = 0; scramble[nscramble]; nscramble++);

  for (int i = nmoves - 1; i >= 0; i--)
    coord = t_table[coord][inverse_move[moves[i]]];
  for (int i = nprevmoves - 1; i >= 0; i--)
    coord = t_table[coord][inverse_move[prev_moves[i]]];
  for (int i = nscramble - 1; i >= 0; i--)
    coord = t_table[coord][inverse_move[scramble[i]]];

  return coord;
}


/******/
/* EO */
/******/
void niss_eo_dfs(int eo, int scramble[], int eo_list[][21], int *eo_count,
                 int t_table[pow2to11][19], int p_table[pow2to11], 
                 int last1, int last2, int moves, int m, int d, int niss,
                 int can_use_niss, int hide) {

  if (*eo_count >= m || moves > d ||
     ((!can_use_niss || niss) && moves + p_table[eo] > d))
    return;

  eo_list[*eo_count][moves] = 0;

  if (eo == 0) {
    /* If an early EO is found, or if "case F2 B", or if hide is on. */
    if (moves != d || (parallel(last1, last2) && last2 % 3 == 2) || 
       (hide && moves > 0 &&
         (last1 % 3 == 0 || (parallel(last1, last2) && last2 % 3 == 0))))
      return;
    /* Copy moves for the next solution */
    if (*eo_count < m - 1)
      copy_moves(eo_list[*eo_count], eo_list[(*eo_count)+1]);
    (*eo_count)++;
    return;
  }

  if (moves + p_table[eo] <= d) {
    for (int i = 1; i < 19; i++) {
      if (possible_next[last1][last2] & (1 << i)) {
        eo_list[*eo_count][moves] = niss ? -i : i;
        niss_eo_dfs(t_table[eo][i], scramble, eo_list, eo_count, t_table,
                    p_table, i, last1, moves+1, m, d, niss,
                    can_use_niss, hide);
      }
    }
  }
  
  eo_list[*eo_count][moves] = 0; 

  /* If not nissing already and we either have not done any move yet or
   * the last move was F/F' etc, and if I am allowed to niss, try niss! */
  if (!niss && (last1 == 0 || t_table[0][last1] != 0) && can_use_niss &&
      !(hide && moves > 0 &&
         (last1 % 3 == 0 || (parallel(last1, last2) && last2 % 3 == 0)))) {
    int aux[] = {0,0};
    niss_eo_dfs(premoves_inverse(eo_list[*eo_count], scramble, aux, t_table),
                scramble, eo_list, eo_count, t_table, p_table,
                0, 0, moves, m, d, 1, can_use_niss, hide);
  }
}

int eo_scram_spam(int scram[], int eo_list[][21], int fb, int rl, int ud,
                  int m, int b, int niss, int h) {
  
  init_small_pruning_tables();

  int n = 0, eofb = 0, eorl = 0, eoud = 0;
  for (int i = 0; scram[i]; i++) {
    eofb = eofb_transition_table[eofb][scram[i]];
    eorl = eorl_transition_table[eorl][scram[i]];
    eoud = eoud_transition_table[eoud][scram[i]];
  }
  for (int i = 0; i <= b; i++) {
    if (fb)
      niss_eo_dfs(eofb, scram, eo_list, &n, eofb_transition_table,
                  eofb_pruning_table, 0, 0, 0, m, i, 0, niss, h);
    if (rl)
      niss_eo_dfs(eorl, scram, eo_list, &n, eorl_transition_table,
                  eorl_pruning_table, 0, 0, 0, m, i, 0, niss, h);
    if (ud)
      niss_eo_dfs(eoud, scram, eo_list, &n, eoud_transition_table,
                  eoud_pruning_table, 0, 0, 0, m, i, 0, niss, h);
  }
  return n;
}     


/**************/
/* DR from EO */
/**************/


/* Scramble includes premoves for previous EO */
void niss_dr_from_eo_dfs(int co, int epos, int scramble[], int eo_moves[21],
                         int dr_list[][21], int *dr_count,
                         int co_t_table[pow3to7][19],
                         int epos_t_table[binom12on4][19],
                         int8_t p_table[pow3to7][binom12on4], int mask,
                         int last1, int last2, int last1_inv, int last2_inv,
                         int moves, int m, int d, int niss,
                         int can_use_niss, int hide) {

  if (*dr_count >= m || moves > d ||
     ((!can_use_niss || niss) && moves + p_table[co][epos] > d))
    return;

  dr_list[*dr_count][moves] = 0;

  if (co == 0 && epos == 0) {
    if (moves != d || (parallel(last1, last2) && last2 % 3 == 2) ||
       (hide && moves > 0 &&
         (last1 % 3 == 0 || (parallel(last1, last2) && last2 % 3 == 0))))
      return;
    /* Copy moves for the next solution */
    if (*dr_count < m - 1)
      copy_moves(dr_list[*dr_count], dr_list[(*dr_count)+1]);
    (*dr_count)++;
    return;
  }

  if (moves + p_table[co][epos] <= d) {
    for (int i = 1; i < 19; i++) {
      if (possible_next[last1][last2] & (1 << i) & mask) {
        dr_list[*dr_count][moves] = niss ? -i : i;
        niss_dr_from_eo_dfs(co_t_table[co][i], epos_t_table[epos][i],
                            scramble, eo_moves, dr_list, dr_count,
                            co_t_table, epos_t_table, p_table, mask,
                            i, last1, last1_inv, last2_inv,
                            moves+1, m, d, niss, can_use_niss, hide);
      }
    }
  }
  
  dr_list[*dr_count][moves] = 0; 

  /* If not nissing already and we either have not done any move yet or
   * the last move was F/F' etc and I am allowed to niss, try niss! */
  if (!niss && (last1 == 0 || co_t_table[0][last1] != 0) && can_use_niss &&
      !(hide && moves > 0 &&
         (last1 % 3 == 0 || (parallel(last1, last2) && last2 % 3 == 0))))
    niss_dr_from_eo_dfs(premoves_inverse(dr_list[*dr_count], scramble,
                                         eo_moves, co_t_table),
                        premoves_inverse(dr_list[*dr_count], scramble,
                                         eo_moves, epos_t_table),
                        scramble, eo_moves, dr_list, dr_count,
                        co_t_table, epos_t_table, p_table,
                        mask, last1_inv, last2_inv, 0, 0,
                        moves, m, d, 1, can_use_niss, hide);
}

int drfrom_scram_spam(int scram[], int dr_list[][21], int from, int fb,
                      int rl, int ud, int m, int b, int niss, int hide) {

  init_drfromeo_pruning_tables();

  int n = 0;
  int eofb = 0, eorl = 0, eoud = 0;
  int epose = 0, eposm = 0, eposs = 0;
  int coud = 0, corl = 0, cofb = 0;
  
  for (int i = 0; scram[i]; i++) {
    eofb = eofb_transition_table[eofb][scram[i]];
    eorl = eorl_transition_table[eorl][scram[i]];
    eoud = eoud_transition_table[eoud][scram[i]];

    cofb = cofb_transition_table[cofb][scram[i]];
    corl = corl_transition_table[corl][scram[i]];
    coud = coud_transition_table[coud][scram[i]];

    epose = epose_transition_table[epose][scram[i]];
    eposm = eposm_transition_table[eposm][scram[i]];
    eposs = eposs_transition_table[eposs][scram[i]];
  }

  int fake_eom[2] = {0, 0}; /* Fake EO moves */

  if (from == 1) {
    if (eofb)
      return -1;
    for (int i = 0; i <= b; i++) {
      if (ud)
        niss_dr_from_eo_dfs(coud, epose, scram, fake_eom, dr_list, &n,
                            coud_transition_table, epose_transition_table,
                            coud_epose_from_eofb_pruning_table, move_mask_eofb,
                            0, 0, 0, 0, 0, m, i, 0, niss, hide);
      if (rl)
        niss_dr_from_eo_dfs(corl, eposm, scram, fake_eom, dr_list, &n,
                            corl_transition_table, eposm_transition_table,
                            corl_eposm_from_eofb_pruning_table, move_mask_eofb,
                            0, 0, 0, 0, 0, m, i, 0, niss, hide);
    }
  } else if (from == 2) {
    if (eorl)
      return -1;
    for (int i = 0; i <= b; i++) {
      if (fb)
        niss_dr_from_eo_dfs(cofb, eposs, scram, fake_eom, dr_list, &n,
                            cofb_transition_table, eposs_transition_table,
                            cofb_eposs_from_eorl_pruning_table, move_mask_eorl,
                            0, 0, 0, 0, 0, m, i, 0, niss, hide);
      if (ud)
        niss_dr_from_eo_dfs(coud, epose, scram, fake_eom, dr_list, &n,
                            coud_transition_table, epose_transition_table,
                            coud_epose_from_eorl_pruning_table, move_mask_eorl,
                            0, 0, 0, 0, 0, m, i, 0, niss, hide);
    }
  } else if (from == 3) {
    if (eoud)
      return -1;
    for (int i = 0; i <= b; i++) {
      if (rl)
        niss_dr_from_eo_dfs(corl, eposm, scram, fake_eom, dr_list, &n,
                            corl_transition_table, eposm_transition_table,
                            corl_eposm_from_eoud_pruning_table, move_mask_eoud,
                            0, 0, 0, 0, 0, m, i, 0, niss, hide);
      if (fb)
        niss_dr_from_eo_dfs(cofb, eposs, scram, fake_eom, dr_list, &n,
                            cofb_transition_table, eposs_transition_table,
                            cofb_eposs_from_eoud_pruning_table, move_mask_eoud,
                            0, 0, 0, 0, 0, m, i, 0, niss, hide);
    }
  } else {
    return -1;
  }
  return n;
}


/***************/
/* HTR from DR */
/***************/

/* Scramble includes premoves for previous DR */
void niss_htr_from_dr_dfs(int cp, int eo3, int scramble[], int eodr_moves[21],
                          int htr_list[][21], int *htr_count,
                          int eo3_t_table[pow2to11][19],
                          int cp_to_htr_pruning_table[factorial8],
                          int cp_htr_pruning_table[factorial8],
                          int cp_finish_pruning_table[factorial8],
                          int mask, int last1, int last2,
                          int last1_inv, int last2_inv, int moves,
                          int m, int d, int niss,
                          int can_use_niss, int hide) {

  if (*htr_count >= m || moves > d ||
      ((!can_use_niss || niss) && moves + cp_to_htr_pruning_table[cp] > d) ||
      moves + cp_finish_pruning_table[cp] - 4 > d)
    return;

  htr_list[*htr_count][moves] = 0;

  if ((cp == 0 || cp_htr_pruning_table[cp]) && eo3 == 0) {
    if (moves != d || (parallel(last1, last2) && last2 % 3 == 2) ||
       (hide && moves > 0 &&
         (last1 % 3 == 0 || (parallel(last1, last2) && last2 % 3 == 0))))
      return;
    /* Copy moves for the next solution */
    if (*htr_count < m - 1)
      copy_moves(htr_list[*htr_count], htr_list[(*htr_count)+1]);
    (*htr_count)++;
    return;
  }

  if (moves + cp_htr_pruning_table[cp] <= d) {
    for (int i = 1; i < 19; i++) {
      if (possible_next[last1][last2] & (1 << i) & mask) {
        htr_list[*htr_count][moves] = niss ? -i : i;
        niss_htr_from_dr_dfs(cp_transition_table[cp][i], eo3_t_table[eo3][i],
                             scramble, eodr_moves, htr_list, htr_count,
                             eo3_t_table, cp_to_htr_pruning_table,
                             cp_htr_pruning_table, cp_finish_pruning_table,
                             mask, i, last1, last1_inv, last2_inv,
                             moves+1, m, d, niss, can_use_niss, hide);
      }
    }
  }
  
  htr_list[*htr_count][moves] = 0; 

  /* If not nissing already and we either have not done any move yet or
   * the last move was a quarter turn and I am allowed to niss, try niss! */
  if (!niss && last1 % 3 != 2 && can_use_niss && 
      !(hide && moves > 0 &&
         (last1 % 3 == 0 || (parallel(last1, last2) && last2 % 3 == 0)))) 
    niss_htr_from_dr_dfs(premoves_inverse(htr_list[*htr_count], scramble,
                                          eodr_moves, cp_transition_table),
                         premoves_inverse(htr_list[*htr_count], scramble,
                                          eodr_moves, eo3_t_table),
                         scramble, eodr_moves, htr_list, htr_count,
                         eo3_t_table, cp_to_htr_pruning_table,
                         cp_htr_pruning_table, cp_finish_pruning_table,
                         mask, last1_inv, last2_inv,
                         0, 0, moves, m, d, 1, can_use_niss, hide);
}

int htr_scram_spam(int scram[], int htr_list[][21], int from,
                   int m, int b, int niss, int hide) {

  init_small_pruning_tables();

  int n = 0;
  int eofb = 0, eorl = 0, eoud = 0;
  int coud = 0, corl = 0, cofb = 0;
  int cp = 0;
  
  for (int i = 0; scram[i]; i++) {
    eofb = eofb_transition_table[eofb][scram[i]];
    eorl = eorl_transition_table[eorl][scram[i]];
    eoud = eoud_transition_table[eoud][scram[i]];

    cofb = cofb_transition_table[cofb][scram[i]];
    corl = corl_transition_table[corl][scram[i]];
    coud = coud_transition_table[coud][scram[i]];
    
    cp = cp_transition_table[cp][scram[i]];
  }

  int fake_drm[2] = {0, 0}; /* Fake DR moves */

  if ((from == 1 || from == 0) && (!eofb && !eorl && !coud)) {
    for (int i = 0; i <= b; i++) {
      niss_htr_from_dr_dfs(cp, eoud, scram, fake_drm, htr_list, &n,
                           eoud_transition_table, cpud_to_htr_pruning_table,
                           cp_htr_pruning_table, cp_drud_pruning_table,
                           move_mask_drud, 0, 0, 0, 0, 0, m, i, 0, niss, hide);
    }
  } else if ((from == 2 || from == 0) && (!eorl && !eoud && !cofb)) {
    for (int i = 0; i <= b; i++) {
      niss_htr_from_dr_dfs(cp, eofb, scram, fake_drm, htr_list, &n,
                           eofb_transition_table, cpfb_to_htr_pruning_table,
                           cp_htr_pruning_table, cp_drfb_pruning_table,
                           move_mask_drfb, 0, 0, 0, 0, 0, m, i, 0, niss, hide);
    }
  } else if ((from == 3 || from == 0) && (!eoud && !eofb && !corl)) {
    for (int i = 0; i <= b; i++) {
      niss_htr_from_dr_dfs(cp, eorl, scram, fake_drm, htr_list, &n,
                           eorl_transition_table, cprl_to_htr_pruning_table,
                           cp_htr_pruning_table, cp_drrl_pruning_table,
                           move_mask_drrl, 0, 0, 0, 0, 0, m, i, 0, niss, hide);
    }
  } else {
    return -1;
  }
  return n;
}


/***********************/
/* Direct DR (no NISS) */
/***********************/
void dr_dfs(int eo, int eo2, int eslice, int co,
            int dr_list[][21], int *dr_count,
            int eo_t_table[pow2to11][19], int eo2_t_table[pow2to11][19],
            int eslice_t_table[binom12on4][19], int co_t_table[pow3to7][19],
            int8_t eo_eslice_p_table[pow2to11][binom12on4],
            int8_t eo_co_p_table[pow2to11][pow3to7],
            int8_t eo2_co_p_table[pow2to11][pow3to7],
            int last1, int last2, int moves, int max_sol,
            int depth, int hide) {
  if (*dr_count >= max_sol || moves + eo_eslice_p_table[eo][eslice] > depth ||
                              moves + eo_co_p_table[eo][co]         > depth ||
                              moves + eo2_co_p_table[eo2][co]       > depth)
    return;

  dr_list[*dr_count][moves] = 0;

  if (eo == 0 && eslice == 0 && co == 0) {
    /* If an early DR is found, or if "case R2 L". */
    if (moves != depth || (parallel(last1, last2) && last2 % 3 == 2) ||
       (hide && moves > 0 &&
         (last1 % 3 == 0 || (parallel(last1, last2) && last2 % 3 == 0))))
      return;
    /* Copy moves for the next solution */
    if (*dr_count < max_sol - 1)
      copy_moves(dr_list[*dr_count], dr_list[(*dr_count)+1]);
    (*dr_count)++;
    return;
  }

  for (int i = 1; i < 19; i++) {
    if (possible_next[last1][last2] & (1 << i)) {
        dr_list[*dr_count][moves] = i;
        dr_dfs(eo_t_table[eo][i], eo2_t_table[eo2][i],
               eslice_t_table[eslice][i], co_t_table[co][i],
               dr_list, dr_count,
               eo_t_table, eo2_t_table, eslice_t_table, co_t_table,
               eo_eslice_p_table, eo_co_p_table, eo2_co_p_table,
               i, last1, moves+1, max_sol, depth, hide);
    }
  }
}

int dr_scram_spam(int scram[], int dr_list[][21], int fb, int rl, int ud,
                  int m, int b, int h) {

  init_directdr_pruning_tables();

  int n = 0;
  int eofb = 0, eorl = 0, eoud = 0;
  int epose = 0, eposm = 0, eposs = 0;
  int coud = 0, corl = 0, cofb = 0;
  
  for (int i = 0; scram[i]; i++) {
    eofb = eofb_transition_table[eofb][scram[i]];
    eorl = eorl_transition_table[eorl][scram[i]];
    eoud = eoud_transition_table[eoud][scram[i]];

    cofb = cofb_transition_table[cofb][scram[i]];
    corl = corl_transition_table[corl][scram[i]];
    coud = coud_transition_table[coud][scram[i]];

    epose = epose_transition_table[epose][scram[i]];
    eposm = eposm_transition_table[eposm][scram[i]];
    eposs = eposs_transition_table[eposs][scram[i]];
  }

  for (int i = 0; i <= b; i++) {
    if (ud)
      dr_dfs(eofb, eorl, epose, coud, dr_list, &n,
             eofb_transition_table, eorl_transition_table,
             epose_transition_table, coud_transition_table,
             eofb_epose_pruning_table, eofb_coud_pruning_table,
             eorl_coud_pruning_table, 0, 0, 0, m, i, h);
    if (fb)
      dr_dfs(eorl, eoud, eposs, cofb, dr_list, &n,
             eorl_transition_table, eoud_transition_table,
             eposs_transition_table, cofb_transition_table,
             eorl_eposs_pruning_table, eorl_cofb_pruning_table,
             eoud_cofb_pruning_table, 0, 0, 0, m, i, h);
    if (rl)
      dr_dfs(eoud, eofb, eposm, corl, dr_list, &n,
             eoud_transition_table, eofb_transition_table,
             eposm_transition_table, corl_transition_table,
             eoud_eposm_pruning_table, eoud_corl_pruning_table,
             eofb_corl_pruning_table, 0, 0, 0, m, i, h);
 }
  return n;
}     


/*************/
/* DR finish */
/*************/
void dr_finish_dfs(int cp, int ep8, int ep4, int sol[][21], int *sol_count,
                  int ep8_t_table[factorial8][19],
                  int ep4_t_table[factorial4][19],
                  int cp_p_table[factorial8],
                  int ep8_p_table[factorial8],
                  int mask, int last1, int last2, int moves, int m, int d) {


  if (*sol_count >= m || moves + cp_p_table[cp] > d ||
                         moves + ep8_p_table[ep8] > d)
    return;

  sol[*sol_count][moves] = 0;

  if (cp == 0 && ep8 == 0 && ep4 == 0) {
    if (moves != d)
      return;
    /* Copy moves for the next solution */
    if (*sol_count < m - 1)
      copy_moves(sol[*sol_count], sol[(*sol_count)+1]);
    (*sol_count)++;
    return;
  }

  for (int i = 1; i < 19; i++) {
    if (possible_next[last1][last2] & (1 << i) & mask) {
      sol[*sol_count][moves] = i;
      dr_finish_dfs(cp_transition_table[cp][i], ep8_t_table[ep8][i],
                        ep4_t_table[ep4][i], sol, sol_count,
                        ep8_t_table, ep4_t_table,
                        cp_p_table, ep8_p_table,
                        mask, i, last1, moves+1, m, d);
    }
  }

  return;
}

int dr_finish_scram_spam(int scram[], int sol[][21], int from, int m, int b) {
  
  init_small_pruning_tables();

  int n = 0;
  int eofb = 0, eorl = 0, eoud = 0;
  int coud = 0, corl = 0, cofb = 0;
  int cp = 0;
  int ep[12];
  ep_int_to_array(0, ep);
  
  for (int i = 0; scram[i]; i++) {
    eofb = eofb_transition_table[eofb][scram[i]];
    eorl = eorl_transition_table[eorl][scram[i]];
    eoud = eoud_transition_table[eoud][scram[i]];

    cofb = cofb_transition_table[cofb][scram[i]];
    corl = corl_transition_table[corl][scram[i]];
    coud = coud_transition_table[coud][scram[i]];
    
    cp = cp_transition_table[cp][scram[i]];
    apply_move_ep_array(scram[i], ep);
  }

  if ((from == 1 && (eofb || eorl || coud)) ||
      (from == 2 && (eorl || eoud || cofb)) ||
      (from == 3 && (eoud || eofb || corl)) ||
    ((eofb || eorl || coud) && (eorl || eoud ||cofb) && (eoud ||eofb || corl)))
    return -1;

  for (int i = 0; i <= b; i++) {
    if ((from == 1 || from == 0) && (!eofb && !eorl && !coud))
      dr_finish_dfs(cp, epud_array_to_int(ep), epe_array_to_int(ep),
                    sol, &n, epud_transition_table, epe_transition_table,
                    cp_drud_pruning_table, epud_pruning_table,
                    move_mask_drud, 0, 0, 0, m, i);
    if ((from == 2 || from == 0) && (!eorl && !eoud && !cofb))
      dr_finish_dfs(cp, epfb_array_to_int(ep), eps_array_to_int(ep),
                    sol, &n, epfb_transition_table, eps_transition_table,
                    cp_drfb_pruning_table, epfb_pruning_table,
                    move_mask_drfb, 0, 0, 0, m, i);
    if ((from == 3 || from == 0) && (!eoud && !eofb && !corl))
      dr_finish_dfs(cp, eprl_array_to_int(ep), epm_array_to_int(ep),
                    sol, &n, eprl_transition_table, epm_transition_table,
                    cp_drrl_pruning_table, eprl_pruning_table,
                    move_mask_drrl, 0, 0, 0, m, i);
  }
 
  return n;
}

int htr_finish_scram_spam(int scram[], int sol[][21], int m, int b) {

  init_small_pruning_tables();

  int n = 0;
  int eofb = 0, eorl = 0, eoud = 0;
  int coud = 0, cp = 0;
  int ep[12];
  ep_int_to_array(0, ep);
  
  for (int i = 0; scram[i]; i++) {
    eofb = eofb_transition_table[eofb][scram[i]];
    eorl = eorl_transition_table[eorl][scram[i]];
    eoud = eoud_transition_table[eoud][scram[i]];

    coud = coud_transition_table[coud][scram[i]];
    
    cp = cp_transition_table[cp][scram[i]];
    apply_move_ep_array(scram[i], ep);
  }

  if (eofb || eorl || eoud || coud || cpud_to_htr_pruning_table[cp] != 0)
    return -1;

  for (int i = 0; i <= b; i++)
    dr_finish_dfs(cp, epud_array_to_int(ep), epe_array_to_int(ep),
                  sol, &n, epud_transition_table, epe_transition_table,
                  cp_drud_pruning_table, epud_pruning_table,
                  move_mask_htr, 0, 0, 0, m, i);
 
  return n;
}


/**************/
/* DR corners */
/**************/
void dr_corners_dfs(int cp, int sol[][21], int *sol_count,
                    int cp_p_table[factorial8], int mask, int last1, int last2,
                    int moves, int m, int d, int ignore) {

  if (*sol_count >= m || (!ignore && moves + cp_p_table[cp] > d) ||
                         (ignore && moves + cp_p_table[cp] - 2 > d))
    return;


  sol[*sol_count][moves] = 0;

  if (cp == 0 || (ignore && 
          (cp_transition_table[cp_transition_table[cp][U]][D3]  == 0 ||
           cp_transition_table[cp_transition_table[cp][U2]][D2] == 0 ||
           cp_transition_table[cp_transition_table[cp][U3]][D]  == 0 ))) {
    if (moves != d)
      return;
    /* Copy moves for the next solution */
    if (*sol_count < m - 1)
      copy_moves(sol[*sol_count], sol[(*sol_count)+1]);
    (*sol_count)++;
    return;
  }

  for (int i = 1; i < 19; i++) {
    if (possible_next[last1][last2] & (1 << i) & mask) {
      sol[*sol_count][moves] = i;
      dr_corners_dfs(cp_transition_table[cp][i], sol, sol_count,
                     cp_p_table, mask, i, last1, moves+1, m, d, ignore);
    }
  }
}

int dr_corners_scram_spam(int scram[], int sol[][21], int from, int m, int b,
                          int ignore) {

  init_small_pruning_tables();

  int n = 0;
  int eofb = 0, eorl = 0, eoud = 0;
  int coud = 0, corl = 0, cofb = 0;
  int cp = 0;
  
  for (int i = 0; scram[i]; i++) {
    eofb = eofb_transition_table[eofb][scram[i]];
    eorl = eorl_transition_table[eorl][scram[i]];
    eoud = eoud_transition_table[eoud][scram[i]];

    cofb = cofb_transition_table[cofb][scram[i]];
    corl = corl_transition_table[corl][scram[i]];
    coud = coud_transition_table[coud][scram[i]];
    
    cp = cp_transition_table[cp][scram[i]];
  }
  
  if ((from == 1 && coud) || (from == 2 && cofb) || (from == 3 && corl) ||
      (coud && cofb && corl))
    return -1;

  for (int i = 0; i <= b; i++) {
    if ((from == 1 || from == 0) && !coud)
      dr_corners_dfs(cp, sol, &n, cp_drud_pruning_table, move_mask_drud,
                     0, 0, 0, m, i, ignore);
    if ((from == 2 || from == 0) && !cofb)
      dr_corners_dfs(cp, sol, &n, cp_drfb_pruning_table, move_mask_drfb,
                     0, 0, 0, m, i, ignore);
    if ((from == 3 || from == 0) && !corl)
      dr_corners_dfs(cp, sol, &n, cp_drrl_pruning_table, move_mask_drrl,
                     0, 0, 0, m, i, ignore);
  }
 
  return n;
}

/***************/
/* Full solver */
/***************/

int is_ep_solved(int ep, int moves[21]) {
  int ep_arr[12];
  ep_int_to_array(ep, ep_arr);
  for (int i = 0; moves[i]; i++)
    apply_move_ep_array(moves[i], ep_arr);
  return !ep_array_to_int(ep_arr);
}

/* Solves directly using only small tables. Suitable for short solutions. */
void small_optimal_dfs(int eofb, int eorl, int eoud, int ep,
                       int coud, int cofb, int corl, int cp,
                       int sol[][21], int *sol_count, int last1, int last2,
                       int moves, int m, int d) {
  if (moves + eofb_pruning_table[eofb] > d ||
      moves + eorl_pruning_table[eorl] > d ||
      moves + eoud_pruning_table[eoud] > d ||
      moves + coud_pruning_table[coud] > d ||
      moves + cofb_pruning_table[cofb] > d ||
      moves + corl_pruning_table[corl] > d ||
      moves + cp_pruning_table[cp]     > d ||
      *sol_count >= m)
    return;

  sol[*sol_count][moves] = 0;
  
  if (eofb == 0 && coud == 0 && cp == 0) {
    if (is_ep_solved(ep, sol[*sol_count])) {
      if (moves != d)
        return;
      if (*sol_count < m - 1)
        copy_moves(sol[*sol_count], sol[(*sol_count)+1]);
      (*sol_count)++;
      return;
    }
  }

  for (int i = 1; i < 19; i++) {
    if (possible_next[last1][last2] & (1 << i)) {
      sol[*sol_count][moves] = i;
      small_optimal_dfs(eofb_transition_table[eofb][i],
                        eorl_transition_table[eorl][i],
                        eoud_transition_table[eoud][i], ep,
                        coud_transition_table[coud][i],
                        cofb_transition_table[cofb][i],
                        corl_transition_table[corl][i],
                        cp_transition_table[cp][i],
                        sol, sol_count, i, last1, moves+1, m, d);
    }
  }
}

/* Solves directly using only medium tables. Suitable for short solutions. */
void medium_optimal_dfs(int eofb, int eorl, int eoud,
                        int epose, int eposs, int eposm, int ep,
                        int coud, int cofb, int corl, int cp,
                        int sol[][21], int *sol_count, int last1, int last2,
                        int moves, int m, int d) {
  if (moves + eofb_epose_pruning_table[eofb][epose] > d ||
      moves + eorl_eposs_pruning_table[eorl][eposs] > d ||
      moves + eoud_eposm_pruning_table[eoud][eposm] > d ||
      moves + eofb_coud_pruning_table[eofb][coud]   > d ||
      moves + eofb_corl_pruning_table[eofb][corl]   > d ||
      moves + eorl_coud_pruning_table[eorl][coud]   > d ||
      moves + eorl_cofb_pruning_table[eorl][cofb]   > d ||
      moves + eoud_cofb_pruning_table[eoud][cofb]   > d ||
      moves + eoud_corl_pruning_table[eoud][corl]   > d ||
      moves + cp_pruning_table[cp]                  > d ||
      *sol_count >= m)
    return;

  sol[*sol_count][moves] = 0;
  
  if (eofb == 0 && coud == 0 && cp == 0) {
    if (is_ep_solved(ep, sol[*sol_count])) {
      if (moves != d)
        return;
      if (*sol_count < m - 1)
        copy_moves(sol[*sol_count], sol[(*sol_count)+1]);
      (*sol_count)++;
      return;
    }
  }

  for (int i = 1; i < 19; i++) {
    if (possible_next[last1][last2] & (1 << i)) {
      sol[*sol_count][moves] = i;
      medium_optimal_dfs(eofb_transition_table[eofb][i],
                        eorl_transition_table[eorl][i],
                        eoud_transition_table[eoud][i],
                        epose_transition_table[epose][i],
                        eposs_transition_table[eposs][i],
                        eposm_transition_table[eposm][i], ep,
                        coud_transition_table[coud][i],
                        cofb_transition_table[cofb][i],
                        corl_transition_table[corl][i],
                        cp_transition_table[cp][i],
                        sol, sol_count, i, last1, moves+1, m, d);
    }
  }
}
      
/* Uses huge tables */
int optimal_dfs(int ep, int cp, int eo, int co, int emslices,
                int sol[][21], int last1, int last2, int moves, int d) {
  if (moves + cp_co_pruning_table[cp][co] > d ||
      moves + triple_eo_pruning_table[eo][emslices] > d)
    return 0;

  sol[0][moves] = 0;

  /* If solved, no need to check the depth */
  if (cp == 0 && co == 0 && eo == 0 && emslices == 0)
    if (is_ep_solved(ep, sol[0]))
      return 1;

  for (int i = 1; i < 19; i++) {
    if (possible_next[last1][last2] & (1 << i)) {
      sol[0][moves] = i;
      if (optimal_dfs(ep, cp_transition_table[cp][i],
                      eofb_transition_table[eo][i],
                      coud_transition_table[co][i],
                      emslices_transition_table[emslices][i],
                      sol, i, last1, moves+1, d))
        return 1;
    }
  }
  return 0;
}

int solve_scram(int scram[], int sol[][21], int m, int b, int optimal) {
 
  /* Initialize pieces. */ 
  int eofb = 0, eorl = 0, eoud = 0, ep = 0;
  int epose = 0, eposs = 0, eposm = 0;
  int coud = 0, cofb = 0, corl = 0, cp = 0;
  int emslices = 0;
  for (int i = 0; scram[i]; i++) {
    eofb     = eofb_transition_table[eofb][scram[i]];
    eorl     = eorl_transition_table[eorl][scram[i]];
    eoud     = eoud_transition_table[eoud][scram[i]];
    
    epose    = epose_transition_table[epose][scram[i]];
    eposs    = eposs_transition_table[eposs][scram[i]];
    eposm    = eposm_transition_table[eposm][scram[i]];

    ep       = apply_move_ep_int(scram[i], ep);

    coud     = coud_transition_table[coud][scram[i]];
    cofb     = cofb_transition_table[cofb][scram[i]];
    corl     = corl_transition_table[corl][scram[i]];
    cp       = cp_transition_table[cp][scram[i]];

    emslices = emslices_transition_table[emslices][scram[i]];
  }

  /* First we check if there are solutions of up to max_small moves. */
  int max_small = 10;
  int n = 0;
  init_small_pruning_tables();
  for (int i = 0; i <= min(b, max_small); i++) {
    small_optimal_dfs(eofb, eorl, eoud, ep, coud, cofb, corl, cp,
                      sol, &n, 0, 0, 0, m, i);
    if (n > 0 && optimal)
      b = min(b, len(sol[0]));
  }
      

  if (n >= m || b <= 10)
    return n;

  /* Then we try a slightly larger optimal solver */
  /* 
  int max_medium = 11;
  init_directdr_pruning_tables();
  for (int i = 0; i <= min(b, max_medium); i++)
    medium_optimal_dfs(eofb, eorl, eoud, epose, eposs, eposm, ep,
                       coud, cofb, corl, cp, sol, &n, 0, 0, 0, m, i);
  */

  /* If we found at least a solution, we return */
  if (n > 0)
    return n;

  /* Then we try a 2-step solver */
  int max_step1 = 5000;
  int db = 14;
  int step1[max_step1+10][21];
  int ss[300], step2[2][21];
  int best = b+1;

  /* TODO maybe: for now, multiple solutions can be found only using the
   * short solver. */

  int n_step1 = dr_scram_spam(scram, step1, 1, 1, 1, max_step1, min(b, db), 0);
  for (int i = 0; i < n_step1; i++) {
    copy_moves(scram, ss);
    append_moves(step1[i], ss);
    if (dr_finish_scram_spam(ss, step2, 0, 1, min(best-1,b) - len(step1[i]))) {
      copy_moves(step1[i], sol[0]);
      append_moves(step2[0], sol[0]);
      best = len(sol[0]);
    }
  }

  /* If optimal solving was not required, or we have already found an optimal
   * solution, we return. */
  if (best <= len(step1[n_step1-1]) || !optimal)
    return best > b ? 0 : 1;

  /* Otherwise, we go on with the optimal solver. */
  int searched = len(step1[n_step1-1])-1;

  printf("Searched up to %d moves, no solution found.\n", searched);
  printf("Using huge pruning tables, if not loaded it might take a while.\n");
  init_huge_pruning_tables();

  for (int i = searched+1; i <= min(b, best-1); i++) {
    if (i >= 10)
      printf("Searching at depth %d.\n", i);
    if (optimal_dfs(ep, cp, eofb, coud, emslices, sol, 0, 0, 0, i)) {
      return 1;
    }
  }
  return best > b ? 0 : 1;
}
