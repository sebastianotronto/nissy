#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "coordinates.h"
#include "moves.h"
#include "utils.h"

/* Functions for nice output */
char *edge_string(int i) {
  return (i > -1 && i < 12) ? edge_string_list[i] : "-";
}

char *corner_string(int i) {
  return (i > -1 && i < 8) ? corner_string_list[i] : "-";
}

char *move_string(int i) {
  return (i > -1 && i < 19) ? move_string_list[i] : "err";
}

void print_ep_array(int ep[12]) {
  for (int i = 0; i < 12; i++)
    printf(" %s ", edge_string(ep[i]));
}

void print_ep_int(int ep) {
  int aux[12];
  ep_int_to_array(ep, aux);
  print_ep_array(aux);
}

void print_cp_array(int cp[8]) {
  for (int i = 0; i < 8; i++)
    printf(" %s ", corner_string(cp[i]));
}

void print_cp_int(int cp) {
  int aux[8];
  cp_int_to_array(cp, aux);
  print_cp_array(aux);
}

void print_eo_array(int eo[12]) {
  for (int i = 0; i < 12; i++) {
    if (eo[i])
      printf("  x ");
    else
      printf("    ");
  }
}

void print_eo_int(int eo) {
  int aux[12];
  eo_11bits_to_array(eo, aux);
  print_eo_array(aux);
}

void print_co_array(int co[8]) {
  for (int i = 0; i < 8; i++) {
    if (co[i] == 0)
      printf("     ");
    if (co[i] == 1)
      printf("  cw ");
    if (co[i] == 2)
      printf(" ccw ");
  }
}

void print_co_int(int co) {
  int aux[8];
  co_7trits_to_array(co, aux);
  print_co_array(aux);
}

void print_cube_scram(int *scram) {
  int ep = 0, cp = 0, eofb = 0, coud = 0;
  for (int i = 0; scram[i]; i++) {
    ep = apply_move_ep_int(scram[i], ep);
    cp = cp_transition_table[cp][scram[i]];
    eofb = eofb_transition_table[eofb][scram[i]];
    coud = coud_transition_table[coud][scram[i]];
  }
  printf("\t\t");         print_ep_int(0);    printf("\n");
  printf("EP:\t\t");      print_ep_int(ep);   printf("\n");
  printf("EO(F/B):\t"); print_eo_int(eofb); printf("\n");
                                            printf("\n");
  printf("\t\t");         print_cp_int(0);    printf("\n");
  printf("CP:\t\t");      print_cp_int(cp);   printf("\n");
  printf("CO(U/D):\t"); print_co_int(coud); printf("\n");
}


void copy_moves(int *src, int *dst) {
  for (int i = 0; (dst[i] = src[i]); i++);
}

void append_moves(int *src, int *dst) {
  int n = 0;
  for (; dst[n]; n++);
  copy_moves(src, dst+n);
}

/* Parse a string and saves the move in a. Supports NISS notation.
 * Returns the number of moves, or -1 in case of error. */
int read_moves(char *str, int *a) {
  int count = 0;
  int niss = 0;
  for (int i = 0; str[i] && str[i] != '\n'; i++) {
    while (str[i] == ' ' || str[i] == '\t') i++;
    switch (str[i]) {
      case 'U':
        a[count++] = niss ? -U : U;
        break;
      case 'D':
        a[count++] = niss ? -D : D;
        break;
      case 'R':
        a[count++] = niss ? -R : R;
        break;
      case 'L':
        a[count++] = niss ? -L : L;
        break;
      case 'F':
        a[count++] = niss ? -F : F;
        break;
      case 'B':
        a[count++] = niss ? -B : B;
        break;
      case '(':
        if (niss)
          return -1;
        else
          niss = 1;
        break;
      case ')':
        if (!niss)
          return -1;
        else
          niss = 0;
        break;
      default:
        return -1;
    }
    switch (str[++i]) {
      case '2':
        a[count-1] += niss ? -1 : 1;
        break;
      case '\'':
      case '3':
        a[count-1] += niss ? -2 : 2;
        break;
      case '1':
      default:
        --i;
    }
  }
  a[count] = 0;
  return count;
}

/* Read moves from standard input, after a prompt. */
int read_moves_from_prompt(int *a) {
  char str[1000];
  printf("Enter moves: ");
  if (fgets(str, 1000, stdin) == NULL)
    return -1;
  return read_moves(str, a);
}

/* Read moves from a list of token, each containing one or more moves. */
int read_moves_from_tok(int n, char tok[][100], int *a) {
  char str[1000] = "";
  for (int i = 0; i < n; i++)
    strcat(str, tok[i]);
  return read_moves(str, a);
}

/* Checks if a sequence of moves uses NISS */
int uses_niss(int *str) {
  for (int i = 0; str[i]; i++)
    if (str[i] < 0)
      return 1;
  return 0;
}

/* A (B) -> B' A */
int unniss(int *src, int *dst) {
  int n = 0;
  for (int i = 0; src[i]; i++)
    if (src[i] < 0)
      n++;

  int norm_count = n, inv_count = n-1;
  for (int i = 0; src[i]; i++)
    if (src[i] > 0)
      dst[norm_count++] = src[i];
    else
      dst[inv_count--] = inverse_move[-src[i]];

  dst[norm_count] = 0;

  return n;
}

int invert(int *src, int *dst) {
  int aux[255];
  for (int i = 0; (aux[i] = -src[i]); i++);
  return unniss(aux, dst);
}

int len(int *scram) {
  int m;
  for (m = 0; scram[m]; m++);
  return m;
}

void print_moves(int moves_list[]) {
  int niss = 0;
  for (int i = 0; moves_list[i]; i++) {
    if (!niss && moves_list[i] < 0) {
      printf("(");
      niss = 1;
    }
    printf("%s", move_string_list[abs(moves_list[i])]);
    if (niss && moves_list[i+1] >= 0) {
      niss = 0;
      printf(")");
    }
    printf(" ");
  }
}
