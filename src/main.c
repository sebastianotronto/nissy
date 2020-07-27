/* blabla */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"
#include "coordinates.h"
#include "io.h"
#include "moves.h"
#include "solver.h"
#include "string.h"

char *commands[][10] = {
  {"help", "[COMMAND]",
   "Print this help, or a help page for COMMAND."},
  {"scramble", "[OPTIONS]",
   "Prints a random-state scramble."},
  {"save", "[MOVES|@ID|$ID]",
   "Save or copy a scramble."},
  {"change", "$ID1 [MOVES|$ID2|@ID2]",
   "Change a memorized scramble."},
  {"print", "[$ID|@ID]",
   "Print memorized sequences."},
  {"add", "[MOVES|$ID1|@ID1] $ID2",
   "Add moves at the end of a memorized scramble."},
  {"invert", "[MOVES|$ID|@ID]",
   "Inverts the given sequence of moves."},
  {"unniss", "[MOVES|$ID|@ID]}",
   "Removes NISS: A (B) -> B\' A."},
  {"pic",   "[MOVES|$ID|@ID]",
   "Show a text description of the scrambled cube."},
  {"solve", "[MOVES|$ID|@ID]",
   "Solves a scramble."},
  {"replace", "[MOVES|$ID|@ID]",
   "Find non-optimal subsequences."},
  {"clear", "",
   "Delete saved scrambles and output sequences."},
  {"eo", "[MOVES|$ID|@ID]",
   "Solves EO."},
  {"dr", "[MOVES|$ID|@ID]",
   "Solves DR, either directly or from eo."},
  {"htr", "[MOVES|$ID|@ID]",
   "Solves HTR from DR."},
  {"drfinish", "[MOVES|$ID|@ID]",
   "Solves the cube after DR."},
  {"htrfinish", "[MOVES|$ID|@ID]",
   "Solves the cube using only half turns."},
  {"drcorners", "[MOVES|$ID|@ID]",
   "Solves corners after DR."},
  {"exit", "",
   "Exit nissy."},
  {"quit", "",
   "Exit nissy."},
  {"", "", ""}
};

/* Saved sequences of moves */
int scr_count=1, tmp_count=1, max_tmp=999;
int scrambles[255][255], tmp[1000][255];

int read_moves_from_variable(char *id, int *dst) {
  char c = id[0];
  if (c != '$' && c != '@')
    return -1;
  int n = atoi(id+1);
  if (n <= 0 || n >= (c == '$' ? scr_count : tmp_count))
    return -1;
  copy_moves(c == '$' ? scrambles[n] : tmp[n], dst);
  return n;
}

int read_moves_from_argument(int n, char tok[][100], int *dst) {
  int r = read_moves_from_variable(tok[0], dst);
  return  (r != -1) ? r : read_moves_from_tok(n, tok, dst);
}

void print_results(int n, int res[][30]) {
  if (n == -1)
    printf("Pre-conditions not satisfied (or other error).\n");

  if (n == 0)
    printf("No result found (try different bounds).\n");

  if (n > 1)
    printf("Found %d results.\n", n);
  tmp_count = 1; /* Reset temporary count */
  for (int i = 0; i < n; i++) {
    if (i < max_tmp) {
      copy_moves(res[i], tmp[tmp_count]);
      printf("@%d:\t", tmp_count++);
    } else {
      printf("  \t");
    }
    print_moves(res[i]);
    printf("(%d)\n", len(res[i]));
  }
}

/* Removes extra white spaces from the input string */
int parsecmd(char *cmd, char cmdtok[][100])  {
  char *i = cmd, *j = cmd;
  while (*j != '\n' && *j != EOF) {
    *i = *j;
    if (*i == ' ' || *i == '\t')
      *i = ' ';
    ++j;
    if (*i == ' ' || *i == '\t')
      while (*j == ' ' || *j == '\t')
        ++j;
    ++i;
  }
  if (*(i-1) == ' ')
    *(i-1) = 0;
  else
    *i = 0;
  
  int n = 0;
  char *s = strtok(cmd, " ");
  while (s != NULL) {
    strcpy(cmdtok[n++], s);
    s = strtok(NULL, " ");
  }
  return n;
}

void help_cmd(int n, char cmdtok[][100]) {
  if (n == 1) {
    printf("\n");
    for (int i = 0; commands[i][0][0]; i++)
      printf("%-10s%-25s%s\n", commands[i][0], commands[i][1], commands[i][2]);
    printf("\n");
    printf("Type \'help\' followed by a command for a detailed help page.\n");
    printf("Type \'help nissy\' for a general user guide.\n");
  } else if (n == 2) {
    char fname[255], line[255] = "";
    FILE *file;
    sprintf(fname, "docs/%s.txt", cmdtok[1]);
    file = fopen(fname, "r");
    if (file == NULL) {
      printf("No help file for %s.\n", cmdtok[1]);
      return;
    }
    while (fgets(line, 255, file) != NULL)
      printf("%s", line);
  } else {
    printf("help: wrong syntax.\n");
  }
}

void scramble_cmd(int n, char cmdtok[][100]) {
  int c = 0, e = 0, dr = 0;

  if (n > 2 || cmdtok[0][0] != 's') { /* Second case avoids warning */
    printf("scramble: wrong syntax\n");
    return;
  } else if (n == 2) {
    if (!strcmp(cmdtok[1], "c")) {
      c = 1;
    } else if (!strcmp(cmdtok[1], "e")) {
      e = 1;
    } else if (!strcmp(cmdtok[1], "dr")) {
      dr = 1;
    } else {
      printf("scramble: wrong syntax\n");
      return;
    }
  }

  int scram[2][30];
  
  srand(time(NULL));
  int eofb = rand() % pow2to11;
  int coud = rand() % pow3to7;
  int ep   = rand() % factorial12;
  int cp   = rand() % factorial8;
  
  if (c) {
    eofb = ep = 0;
  } else if (e) {
    coud = cp = 0;
  } else if (dr) {
    eofb = coud = 0;
    int epud = rand() % factorial8;
    int epe  = rand() % factorial4;
    int ep_arr[12];
    epud_int_to_array(epud, ep_arr);
    epe_int_to_array(epe, ep_arr);
    ep = ep_array_to_int(ep_arr);
    while (perm_sign_int(ep, 12) != perm_sign_int(cp, 8))
      cp = (cp+1) % factorial8;
  } else {
    while (perm_sign_int(ep, 12) != perm_sign_int(cp, 8))
      cp = (cp+1) % factorial8;
  }

  reach_state(eofb, coud, ep, cp, scram);
  /* Debug */
  /* printf("State: %d %d %d %d\n", eofb, coud, ep, cp); */
  print_results(1, scram);
}

void save_cmd(int n, char cmdtok[][100]) {
  int scram[255];
  if (n == 1) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("save: error reading moves. Not saved.\n");
      return;
    }
  } else if (read_moves_from_argument(n-1, cmdtok+1, scram) == -1) {
    printf("save: error reading moves or ID. Not saved.\n");
    return;
  }

  copy_moves(scram, scrambles[scr_count]);

  printf("$%d:\t", scr_count);
  print_moves(scrambles[scr_count]);
  printf("\n");
  scr_count++;
}

void change_cmd(int n, char cmdtok[][100]) {
  int id, scram[255];
  if (n == 1) {
    printf("change: you must specify an $ID.\n");
    return;
  } else if (cmdtok[1][0] != '$') {
    printf("change: invalid $ID.\n");
    return;
  } else {
    id = atoi(cmdtok[1]+1);
    if (id <= 0 || id >= scr_count) {
      printf("change: invalid $ID.\n");
      return;
    }
    if (n == 2) {
      if (read_moves_from_prompt(scram) == -1) {
        printf("change: error reading moves.\n");
        return;
      }
    } else if (read_moves_from_argument(n-2, cmdtok+2, scram) == -1 ) {
      printf("change: error reading moves or ID.\n");
      return;
    }
  }

  copy_moves(scram, scrambles[id]);

  printf("$%d:\t", id);
  print_moves(scrambles[id]);
  printf("\n");
}

void print_cmd(int n, char cmdtok[][100]) {
  if (n == 1) {
    for (int i = 1; i < scr_count; i++) {
      printf("$%d:\t", i);
      print_moves(scrambles[i]);
      printf("\n");
    }
  } else if (n == 2) {
    int i = atoi(cmdtok[1]+1);
    char sign = cmdtok[1][0];
    if (sign != '$' && sign != '@') {
      printf("print: invalid ID (must start with $ or @).\n");
      return;
    }
    if (i > 0 && i < (sign == '$' ? scr_count : tmp_count)) {
      printf("%c%d:\t", sign, i);
      print_moves(sign == '$' ? scrambles[i] : tmp[i]);
      printf("\n");
    } else {
      printf("print: invalid ID.\n");
      return;
    }
  } else {
    printf("print: wrong syntax.\n");
  }
}

void add_cmd(int n, char cmdtok[][100]) {
  int id, scram[255];
  if (n == 1) {
    printf("add: you must specify a destination $ID.\n");
    return;
  } else if (cmdtok[n-1][0] != '$') {
    printf("add: invalid destination $ID.\n");
    return;
  } else {
    id = atoi(cmdtok[n-1]+1);
    if (id <= 0 || id >= scr_count) {
      printf("add: invalid destination $ID.\n");
      return;
    }
    if (n == 2) {
      if (read_moves_from_prompt(scram) == -1) {
        printf("add: error reading moves.\n");
        return;
      }
    } else {
      if (read_moves_from_argument(n-2, cmdtok+1, scram) == -1) {
        printf("add: error reading moves or ID.\n");
        return;
      }
    }
  }

  append_moves(scram, scrambles[id]);

  printf("$%d:\t", id);
  print_moves(scrambles[id]);
  printf("\n");
}

void invert_cmd(int n, char cmdtok[][100]) {
  int scram[255];
  if (n == 1) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("invert: error reading moves.\n");
      return;
    }
  } else if (read_moves_from_argument(n-1, cmdtok+1, scram) == -1) {
    printf("invert: error reading moves or ID.\n");
    return;
  }

  if (uses_niss(scram)) {
    printf("invert: cannot invert NISS.\n");
    return;
  }

  invert(scram, tmp[1]);
  tmp_count = 2;

  printf("@1:\t");
  print_moves(tmp[1]);
  printf("\n");
}

void unniss_cmd(int n, char cmdtok[][100]) {
  int scram[255];
  if (n == 1) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("unniss: error reading moves.\n");
      return;
    }
  } else if (read_moves_from_argument(n-1, cmdtok+1, scram) == -1) {
    printf("unniss: error reading moves or ID.\n");
    return;
  }
  
  unniss(scram, tmp[1]);
  tmp_count = 2;

  printf("@1:\t");
  print_moves(tmp[1]);
  printf("\n");
}

void pic_cmd(int n, char cmdtok[][100]) {
  int scram[255];
  if (n == 1) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("pic: error reading moves.\n");
      return;
    }
  } else if (read_moves_from_argument(n-1, cmdtok+1, scram) == -1) {
    printf("pic: error reading moves or ID.\n");
    return;
  }
  print_cube_scram(scram);
}

void solve_cmd(int n, char cmdtok[][100]) {
  int m = 1, b = 25, optimal = 0;
  int scram[255] = {[0] = 0};
  int scram_unnissed[255];

  /* Parse options */
  for (int i = 1; i < n && scram[0] == 0; i++) {
    if (!strncmp(cmdtok[i], "b=", 2)) {
      b = atoi(cmdtok[i]+2);
      if (b <= 0) {
        printf("solve: bad option b.\n");
        return;
      }
    } else if (!strncmp(cmdtok[i], "n=", 2)) {
      m = atoi(cmdtok[i]+2);
      if (m <= 0) {
          printf("solve: bad option n.\n");
        return;
      }
    } else if (!strcmp(cmdtok[i], "o")) {
      optimal = 1;
    } else if (read_moves_from_argument(n-i, cmdtok+i, scram) == -1) {
      printf("solve: error reading moves or ID.\n");
      return;
    }
  }

  if (scram[0] == 0) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("solve: error reading moves.\n");
      return;
    }
  }

  /* Call solver and print results */
  unniss(scram, scram_unnissed);
  int sol[m+2][30];
  int s = solve_scram(scram_unnissed, sol, m, b, optimal);
  print_results(s, sol);
}

void replace_cmd(int n, char cmdtok[][100]) {
  int m = 10; /* max length */
  int scram[255] = {[0] = 0};
  int scram_unnissed[255];

  /* Parse options */
  for (int i = 1; i < n && scram[0] == 0; i++) {
    if (!strncmp(cmdtok[i], "b=", 2)) {
      m = atoi(cmdtok[i]+2);
      if (m <= 0) {
          printf("replace: bad option n.\n");
        return;
      }
    } else if (read_moves_from_argument(n-i, cmdtok+i, scram) == -1) {
      printf("replace: error reading moves or ID.\n");
      return;
    }
  }

  if (scram[0] == 0) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("replace: error reading moves.\n");
      return;
    }
  }

  unniss(scram, scram_unnissed);
  int l = len(scram_unnissed);
  int aux1[255], aux2[15][30], aux3[30];
  for (int i = 0; i < l; i++) {
    for (int j = 2; j <= m && i + j <= l; j++) {
      copy_moves(scram_unnissed+i, aux1);
      aux1[j] = 0;
      int s = solve_scram(aux1, aux2, 10, j-1, 1);
      for (int k = 0; k < s; k++) {
        invert(aux2[k], aux3);
        /* TODO: the following part should also chek for the case when
         * the last moves are R L or similar. */
        if (aux3[0] != aux1[0] && aux3[len(aux3)-1] != aux1[len(aux1)-1]) {
          printf("Replace [ ");
          print_moves(aux1);
          printf("] (moves %d-%d) with: [ ", i+1, i+j);
          print_moves(aux3);
          printf("] (-%d+%d)\n", j, len(aux3));
        }
      }
    }
  }
}     

void clear_cmd(int n, char cmdtok[][100]) {
  if (n > 1 || cmdtok[0][0] != 'c') {  /* Avoid unused variable warning */
    printf("clear: syntax error.\n");
    return;
  }
  scr_count = tmp_count = 1;
}

void eo_cmd(int n, char cmdtok[][100]) {
  
  /* Default values */
  int m = 1, b = 20;
  int niss = 0, hide = 1;
  int fb = 1, rl = 1, ud = 1;
  int scram[255] = {[0] = 0};
  int scram_unnissed[255];
  
  /* Parse options */
  for (int i = 1; i < n && scram[0] == 0; i++) {
    if (!strcmp(cmdtok[i], "h")) {
      hide = 0;
    } else if (!strcmp(cmdtok[i], "niss")) {
      niss = 1;
    } else if (!strncmp(cmdtok[i], "axis=", 5)) {
      fb = rl = ud = 0;
      if (strstr(cmdtok[i], "fb") != NULL)
        fb = 1;
      if (strstr(cmdtok[i], "rl") != NULL)
        rl = 1;
      if (strstr(cmdtok[i], "ud") != NULL)
        ud = 1;
      if (fb + rl + ud == 0) {
        printf("eo: bad axis option.\n");
        return;
      }
    } else if (!strncmp(cmdtok[i], "n=", 2)) {
      m = atoi(cmdtok[i]+2);
      if (m <= 0) {
        printf("eo: bad option n.\n");
        return;
      }
    } else if (!strncmp(cmdtok[i], "b=", 2)) {
      b = atoi(cmdtok[i]+2);
      if (b <= 0) {
        printf("eo: bad option b.\n");
        return;
      }
    } else if (read_moves_from_argument(n-i, cmdtok+i, scram) == -1) {
      printf("eo: error reading moves or ID.\n");
      return;
    }
  }

  if (scram[0] == 0) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("eo: error reading moves.\n");
      return;
    }
  }

  unniss(scram, scram_unnissed);

  /* Call solver and print results */
  int eo_list[m+5][30];
  int neo = eo_scram_spam(scram_unnissed, eo_list, fb, rl, ud, m, b, niss,
                          hide);
  print_results(neo, eo_list);
}

void dr_cmd(int n, char cmdtok[][100]) {
  
  /* Default values */
  int m = 1, b = 20;
  int niss = 0, hide = 1;
  int from = 0; /* 0: direct dr; {1,2,3}: from {eofb,eorl,eoud} */
  int fb = 1, rl = 1, ud = 1;
  int scram[255] = {[0] = 0};
  int scram_unnissed[255];
  
  /* Parse options */
  for (int i = 1; i < n && scram[0] == 0; i++) {
    if (!strcmp(cmdtok[i], "h")) {
      hide = 0;
    } else if (!strcmp(cmdtok[i], "niss")) {
      niss = 1;
    } else if (!strncmp(cmdtok[i], "axis=", 5)) {
      fb = rl = ud = 0;
      if (strstr(cmdtok[i], "fb") != NULL)
        fb = 1;
      if (strstr(cmdtok[i], "rl") != NULL)
        rl = 1;
      if (strstr(cmdtok[i], "ud") != NULL)
        ud = 1;
      if (fb + rl + ud == 0) {
        printf("dr: bad axis option.\n");
        return;
      }
    } else if (!strncmp(cmdtok[i], "n=", 2)) {
      m = atoi(cmdtok[i]+2);
      if (m <= 0) {
        printf("dr: bad option n.\n");
        return;
      }
    } else if (!strncmp(cmdtok[i], "b=", 2)) {
      b = atoi(cmdtok[i]+2);
      if (b <= 0) {
        printf("dr: bad option b.\n");
        return;
      }
    } else if (!strcmp(cmdtok[i], "from")) {
      i++;
      char x[3][3] = {"fb", "rl", "ud"};
      for (int j = 0; j < 3; j++)
        if (!strcmp(cmdtok[i], x[j]))
          from = j+1;
      if (!from) {
        printf("dr: bad from option.\n");
        return;
      }
    } else if (read_moves_from_argument(n-i, cmdtok+i, scram) == -1) {
      printf("dr: error reading moves or ID.\n");
      return;
    }
  }

  if (scram[0] == 0) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("dr: error reading moves.\n");
      return;
    }
  }

  unniss(scram, scram_unnissed);

  /* Call solver */
  int dr_list[m+5][30], ndr;
  if (from) {
    ndr = drfrom_scram_spam(scram_unnissed, dr_list, from, fb, rl, ud,
                            m, b, niss, hide);
    if (ndr == -1) {
      printf("dr: from given, but EO not found (possibly other error).\n");
      return;
    }
  } else {
    if (niss)
      printf("Warning: not using NISS for direct DR.\n");
    ndr = dr_scram_spam(scram_unnissed, dr_list, fb, rl, ud, m, b, hide);
  }
  print_results(ndr, dr_list);
}

void htr_cmd(int n, char cmdtok[][100]) {

  /* Default values */
  int m = 1, b = 20;
  int niss = 0, hide = 1;
  int from = 0; /* 0: unspecified; {1,2,3}: from {ud,fb,rl} */
  int scram[255] = {[0] = 0};
  int scram_unnissed[255];

  /* Parse options */
  for (int i = 1; i < n && scram[0] == 0; i++) {
    if (!strcmp(cmdtok[i], "h")) {
      hide = 0;
    } else if (!strcmp(cmdtok[i], "niss")) {
      niss = 1;
    } else if (!strncmp(cmdtok[i], "n=", 2)) {
      m = atoi(cmdtok[i]+2);
      if (m <= 0) {
        printf("htr: bad option n.\n");
        return;
      }
    } else if (!strncmp(cmdtok[i], "b=", 2)) {
      b = atoi(cmdtok[i]+2);
      if (b <= 0) {
        printf("htr: bad option b.\n");
        return;
      }
    } else if (!strcmp(cmdtok[i], "from")) {
      i++;
      char x[3][3] = {"ud", "fb", "rl"};
      for (int j = 0; j < 3; j++)
        if (!strcmp(cmdtok[i], x[j]))
          from = j+1;
      if (!from) {
        printf("htr: bad from option.\n");
        return;
      }
    } else if (read_moves_from_argument(n-i, cmdtok+i, scram) == -1) {
      printf("htr: error reading moves or ID.\n");
      return;
    }
  }

  if (scram[0] == 0) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("htr: error reading moves.\n");
      return;
    }
  }

  unniss(scram, scram_unnissed);

  /* Call solver */
  int htr_list[m+5][30], nhtr;
  nhtr = htr_scram_spam(scram_unnissed, htr_list, from, m, b, niss, hide);
  print_results(nhtr, htr_list);
}

void drfinish_cmd(int n, char cmdtok[][100]) {
  /* Default values */
  int m = 1, b = 20;
  int from = 0; /* 0: unspecified; {1,2,3}: from {ud,fb,rl} */
  int scram[255] = {[0] = 0};
  int scram_unnissed[255];

  /* Parse options */
  for (int i = 1; i < n && scram[0] == 0; i++) {
    if (!strncmp(cmdtok[i], "n=", 2)) {
      m = atoi(cmdtok[i]+2);
      if (m <= 0) {
        printf("drfinish: bad option n.\n");
        return;
      }
    } else if (!strncmp(cmdtok[i], "b=", 2)) {
      b = atoi(cmdtok[i]+2);
      if (b <= 0) {
        printf("drfinish: bad option b.\n");
        return;
      }
    } else if (!strcmp(cmdtok[i], "from")) {
      i++;
      char x[3][3] = {"ud", "fb", "rl"};
      for (int j = 0; j < 3; j++)
        if (!strcmp(cmdtok[i], x[j]))
          from = j+1;
      if (!from) {
        printf("drfinish: bad from option.\n");
        return;
      }
    } else if (read_moves_from_argument(n-i, cmdtok+i, scram) == -1) {
      printf("drfinish: error reading moves or ID.\n");
      return;
    }
  }

  if (scram[0] == 0) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("drfinish: error reading moves.\n");
      return;
    }
  }

  unniss(scram, scram_unnissed);
  
  /* Call solver */
  int c_list[m+5][30], nc;
  nc = dr_finish_scram_spam(scram_unnissed, c_list, from, m, b);
  print_results(nc, c_list);
}

void htrfinish_cmd(int n, char cmdtok[][100]) {
  /* Default values */
  int m = 1, b = 20;
  int scram[255] = {[0] = 0};
  int scram_unnissed[255];

  /* Parse options */
  for (int i = 1; i < n && scram[0] == 0; i++) {
    if (!strncmp(cmdtok[i], "n=", 2)) {
      m = atoi(cmdtok[i]+2);
      if (m <= 0) {
        printf("htrfinish: bad option n.\n");
        return;
      }
    } else if (!strncmp(cmdtok[i], "b=", 2)) {
      b = atoi(cmdtok[i]+2);
      if (b <= 0) {
        printf("htrfinish: bad option b.\n");
        return;
      }
    } else if (read_moves_from_argument(n-i, cmdtok+i, scram) == -1) {
      printf("htrfinish: error reading moves or ID.\n");
      return;
    }
  }

  if (scram[0] == 0) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("htrfinish: error reading moves.\n");
      return;
    }
  }

  unniss(scram, scram_unnissed);
  
  /* Call solver */
  int c_list[m+5][30], nc;
  nc = htr_finish_scram_spam(scram_unnissed, c_list, m, b);
  print_results(nc, c_list);
}

void drcorners_cmd(int n, char cmdtok[][100]) {
  /* Default values */
  int m = 1, b = 20, ignore=0;
  int from = 0; /* 0: unspecified; {1,2,3}: from {ud,fb,rl} */
  int scram[255] = {[0] = 0};
  int scram_unnissed[255];

  /* Parse options */
  for (int i = 1; i < n && scram[0] == 0; i++) {
    if (!strncmp(cmdtok[i], "n=", 2)) {
      m = atoi(cmdtok[i]+2);
      if (m <= 0) {
        printf("drcorners: bad option n.\n");
        return;
      }
    } else if (!strncmp(cmdtok[i], "b=", 2)) {
      b = atoi(cmdtok[i]+2);
      if (b <= 0) {
        printf("drcorners: bad option b.\n");
        return;
      }
    } else if (!strcmp(cmdtok[i], "i")) {
      ignore = 1;
    } else if (!strcmp(cmdtok[i], "from")) {
      i++;
      char x[3][3] = {"ud", "fb", "rl"};
      for (int j = 0; j < 3; j++)
        if (!strcmp(cmdtok[i], x[j]))
          from = j+1;
      if (!from) {
        printf("drcorners: bad from option.\n");
        return;
      }
    } else if (read_moves_from_argument(n-i, cmdtok+i, scram) == -1) {
      printf("drcorners: error reading moves or ID.\n");
      return;
    }
  }

  if (scram[0] == 0) {
    if (read_moves_from_prompt(scram) == -1) {
      printf("drcorners: error reading moves.\n");
      return;
    }
  }

  unniss(scram, scram_unnissed);
  
  /* Call solver */
  int c_list[m+5][30], nc;
  nc = dr_corners_scram_spam(scram_unnissed, c_list, from, m, b, ignore);
  print_results(nc, c_list);
}


void exit_quit_cmd(int n, char cmdtok[][100]) {
  if (n == 1)
    exit(0);
  else
    printf("%s: wrong synstax.\n", cmdtok[0]);
}

void (*cmd_list[])(int n, char cmdtok[][100]) = {
  help_cmd, scramble_cmd, save_cmd, change_cmd, print_cmd,
  add_cmd, invert_cmd, unniss_cmd, pic_cmd,
  solve_cmd, replace_cmd, clear_cmd,
  eo_cmd, dr_cmd, htr_cmd,
  drfinish_cmd, htrfinish_cmd, drcorners_cmd,
  exit_quit_cmd, exit_quit_cmd, NULL
};

void execcmd(int n, char cmdtok[][100]) {
  int i = 0;
  while (strcmp(commands[i][0], cmdtok[0]) && strcmp(commands[i][0], ""))
    i++;
  if (strcmp(commands[i][0], ""))
    (*cmd_list[i])(n, cmdtok);
  else
    printf("%s: not a command.\n", cmdtok[0]);
}

int main() {
  init_transition_table();
  init_possible_next();

  printf("Type help for a list of commands.\n");

  char cmd[1000] = "";
  while (1) {
    printf("nissy-# ");
    if (fgets(cmd, 1000, stdin) == NULL)
      break;
    char cmdtok[100][100];
    int n = parsecmd(cmd, cmdtok);
    if (n == 0)
      continue;
    execcmd(n, cmdtok);
  }

  return 0;
}
