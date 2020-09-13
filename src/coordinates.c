#include <stdio.h>

#include "utils.h"
#include "coordinates.h"

/* Names of pieces and moves. */
char edge_string_list[12][5] = {
  "UF", "UL", "UB", "UR", "DF", "DL", "DB", "DR", "FR", "FL", "BL", "BR"
};

char corner_string_list[8][5] = {
  "UFR", "UFL", "UBL", "UBR", "DFR", "DFL", "DBL", "DBR"
};

char move_string_list[19][5] = {
  "-",
  "U", "U2", "U\'", "D", "D2", "D\'", "R", "R2", "R\'",
  "L", "L2", "L\'", "F", "F2", "F\'", "B", "B2", "B\'"
};

int inverse_move[19] = {
  -1, U3, U2, U, D3, D2, D, R3, R2, R, L3, L2, L, F3, F2, F, B3, B2, B
};

/* Convert piece representation from integer to array.
 * Come convertions are not "perfect": for example, and epud type of piece
 * is represented by a permutation index in 8! elements, but it as an array
 * it is converted to the first 8 elements of a 12 elements ep array (with
 * meaningless values for the other 4 elements). */

void ep_int_to_array(int ep, int a[12]) {
  index_to_perm(ep, 12, a);
}

void epud_int_to_array(int epud, int a[12]) {
  index_to_perm(epud, 8, a); /* Last 4 elements are left untouched. */
}

void epfb_int_to_array(int epfb, int a[12]) {
  int edges[] = {UF, UB, DF, DB, FR, FL, BL, BR};
  int b[8];
  index_to_perm(epfb, 8, b);
  for (int i = 0; i < 8; i++)
    a[edges[i]] = edges[b[i]];
}

void eprl_int_to_array(int eprl, int a[12]) {
  int edges[] = {UL, UR, DL, DR, FR, FL, BL, BR};
  int b[8];
  index_to_perm(eprl, 8, b); 
  for (int i = 0; i < 8; i++)
    a[edges[i]] = edges[b[i]];
}

void epose_int_to_array(int epos, int a[12]) {
  int edges[] = {FR, FL, BL, BR};
  index_to_subset(epos, 12, 4, a);
  for (int i = 0, j = 0; i < 12; i++)
    a[i] = (a[i] == 1) ? edges[j++] : -1;
}

void eposs_int_to_array(int epos, int a[12]) {
  int edges[] = {UL, UR, DL, DR};
  index_to_subset(epos, 12, 4, a);
  for (int i = 0, j = 0; i < 12; i++)
    a[i] = (a[i] == 1) ? edges[j++] : -1;
  /* Swap with last 4, so 0 is alway solved state */
  for (int i = 0; i < 4; i++)
    swap(&a[edges[i]], &a[i+8]);
}

void eposm_int_to_array(int epos, int a[12]) {
  int edges[] = {UF, UB, DF, DB};
  index_to_subset(epos, 12, 4, a);
  for (int i = 0, j = 0; i < 12; i++)
    a[i] = (a[i] == 1) ? edges[j++] : -1;
  /* Swap with last 4, so 0 is alway solved state */
  for (int i = 0; i < 4; i++)
    swap(&a[edges[i]], &a[i+8]);
}

void epe_int_to_array(int epe, int a[12]) {
  index_to_perm(epe, 4, a+8);
  for (int i = 0; i < 4; i++)
    a[i+8] += 8;
}

void eps_int_to_array(int eps, int a[12]) {
  int edges[] = {UL, UR, DL, DR};
  int b[4];
  index_to_perm(eps, 4, b);
  for (int i = 0; i < 4; i++)
   a[edges[i]] = edges[b[i]];
} 

void epm_int_to_array(int epm, int a[12]) {
  int edges[] = {UF, UB, DF, DB};
  int b[4];
  index_to_perm(epm, 4, b);
  for (int i = 0; i < 4; i++)
    a[edges[i]] = edges[b[i]];
}

void emslices_int_to_array(int emslices, int a[12]) {
  int b[] = {0,0,0,0,0,0,0,0};
  int eslice[] = {FR, FL, BL, BR};
  int mslice[] = {UF, UB, DF, DB};

  index_to_subset(emslices % binom12on4, 12, 4, a);
  index_to_subset(emslices / binom12on4, 8,  4, b);

  if (emslices % binom12on4 == 0) {
    swap(&b[UF], &b[DL]);
    swap(&b[UB], &b[DR]);
    /*for (int i = 0; i < 4; i++)
      swap(&b[mslice[i]], &b[i+4]);*/
  }

  for (int i = 0, j = 0; j < 8; i++, j++) {
    while (a[i])
      i++;
    a[i] = b[j] ? 2 : -1;
  }
  for (int i = 0, j1 = 0, j2 = 0; i < 12; i++) {
    if (a[i] == 1)
      a[i] = eslice[j1++];
    if (a[i] == 2)
      a[i] = mslice[j2++];
  }
}

void cp_int_to_array(int cp, int a[8]) {
  index_to_perm(cp, 8, a);
}

void eo_11bits_to_array(int eo, int a[12]) {
  int_to_sum_zero_array(eo, 2, 12, a);
}

void co_7trits_to_array(int co, int a[8]) {
  int_to_sum_zero_array(co, 3, 8, a);
}





int ep_array_to_int(int ep[12]) {
  return perm_to_index(ep, 12);
}

int epud_array_to_int(int ep[12]) {
  return perm_to_index(ep, 8); /* Last 4 elements are ignored */
}

int epfb_array_to_int(int ep[12]) {
  int index[] = {0, -1, 1, -1, 2, -1, 3, -1, 4, 5, 6, 7};
  int b[8];
  for (int i = 0; i < 12; i++)
    if (index[i] != -1)
      b[index[i]] = index[ep[i]];
  return perm_to_index(b, 8);
}

int eprl_array_to_int(int ep[12]) {
  int index[] = {-1, 0, -1, 1, -1, 2, -1, 3, 4, 5, 6, 7};
  int b[8];
  for (int i = 0; i < 12; i++)
    if (index[i] != -1)
      b[index[i]] = index[ep[i]];
  return perm_to_index(b, 8);
}

int epose_array_to_int(int ep[12]) {
  int a[12];
  for (int i = 0; i < 12; i++)
    a[i] = (ep[i] >= FR);
  return subset_to_index(a, 12, 4);
}

int eposs_array_to_int(int ep[12]) {
  int a[12];
  int edges[] = {UL, UR, DL, DR};
  for (int i = 0; i < 12; i++)
    a[i] = (ep[i] == UL || ep[i] == UR || ep[i] == DL || ep[i] == DR);
  /* Swap with last 4, so 0 is alway solved state */
  for (int i = 0; i < 4; i++)
    swap(&a[edges[i]], &a[i+8]);
  return subset_to_index(a, 12, 4);
}

int eposm_array_to_int(int ep[12]) {
  int a[12];
  int edges[] = {UF, UB, DF, DB};
  for (int i = 0; i < 12; i++)
    a[i] = (ep[i] == UF || ep[i] == UB || ep[i] == DF || ep[i] == DB);
  /* Swap with last 4, so 0 is alway solved state */
  for (int i = 0; i < 4; i++)
    swap(&a[edges[i]], &a[i+8]);
  return subset_to_index(a, 12, 4);
}

int epe_array_to_int(int ep[12]) {
  int b[4];
  for (int i = 0; i < 4; i++)
    b[i] = ep[i+8] - 8;
  return perm_to_index(b, 4);
}

int eps_array_to_int(int ep[12]) {
  int index[] = {-1, 0, -1, 1, -1, 2, -1, 3, -1, -1, -1, -1};
  int b[4];
  for (int i = 0; i < 12; i++)
    if (index[i] != -1)
      b[index[i]] = index[ep[i]];
  return perm_to_index(b, 4);
}

int epm_array_to_int(int ep[12]) {
  int index[] = {0, -1, 1, -1, 2, -1, 3, -1, -1, -1, -1, -1};
  int b[4];
  for (int i = 0; i < 12; i++)
    if (index[i] != -1)
      b[index[i]] = index[ep[i]];
  return perm_to_index(b, 4);
}

int emslices_array_to_int(int ep[12]) {
  int a[12], b[12], c[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  /*int edges[] = {UF, UB, DF, DB};*/
  for (int i = 0; i < 12; i++) {
    a[i] = (ep[i] >= FR) ? 1 : 0;
    b[i] = (ep[i] == UF || ep[i] == UB || ep[i] == DF || ep[i] == DB) ? 1 : 0;
  }

  /*for ( int i = 0; i < 12; i++)
    printf("%d ", ep[i]);
  printf("\n");*/
 
  for (int i = 0, j = 0; i < 12; i++, j++) {
    if (a[i])
      j--;
    if (b[i])
      c[j] = 1;
  }

  int epose = subset_to_index(a, 12, 4);

  /*if (epose == 0) {
    printf("Before: ");
    for (int i = 0; i < 8; i++)
      printf("%d ", c[i]);
    printf("\n");
    for (int i = 0; i < 4; i++)
      swap(&c[edges[i]], &c[i+4]); 
    printf("After: ");
    for (int i = 0; i < 8; i++)
      printf("%d ", c[i]);
    printf("\n");
  }*/
  if (epose == 0) {
    swap(&c[UF], &c[DL]);
    swap(&c[UB], &c[DR]);
  }

  /*for ( int i = 0; i < 8; i++)
    printf("%d ", c[i]);
  printf("\n");*/
  int eposm = subset_to_index(c, 8, 4);

  return epose + 495*eposm;
}


int cp_array_to_int(int cp[8]) {
  return perm_to_index(cp, 8);
}

int eo_array_to_11bits(int a[12]) {
  return digit_array_to_int(a, 11, 2);
}

int co_array_to_7trits(int a[8]) {
  return digit_array_to_int(a, 7, 3);
}

