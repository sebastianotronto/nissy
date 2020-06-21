#include <stdint.h>

char *edge_string(int edge);
char *corner_string(int edge);
char *move_string(int move);

void print_cube_scram(int *scram);

void copy_moves(int *src, int *dst);
void append_moves(int *src, int *dst);

int read_moves(char *str, int *a);
int read_moves_from_prompt(int *a);
int read_moves_from_tok(int n, char tok[][100], int *a);

int uses_niss(int *str);
int unniss(int *src, int *dst);
int invert(int *src, int *dst);
int len(int *scram);

void print_moves(int move_list[]);
