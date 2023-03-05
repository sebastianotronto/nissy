#ifndef ALG_H
#define ALG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cubetypes.h"
#include "utils.h"

void        append_alg(AlgList *l, Alg *alg);
void        append_move(Alg *alg, Move m, bool inverse);
Move        base_move(Move m);
int         compare(Move m1, Move m2); /* Return 1 (m1<m2), 0 or -1 (m1>m2) */
int         compare_last(Alg *alg, Move m, bool inverse);
void        compose_alg(Alg *alg1, Alg *alg2);
bool        commute(Move m1, Move m2);
void        copy_alg(Alg *src, Alg *dst);
void        free_alg(Alg *alg);
void        free_alglist(AlgList *l);
Alg *       inverse_alg(Alg *alg);
Move        inverse_move(Move m);
char *      move_string(Move m);
Alg *       new_alg(char *str);
AlgList *   new_alglist();
Alg *       on_inverse(Alg *alg);
void        print_alg(Alg *alg, bool l);
void        print_alglist(AlgList *al, bool l);
void        remove_last_move(Alg *alg);
void        swapmove(Move *m1, Move *m2);
char *      trans_string(Trans t); /* Here because similar to move_string, move? */
Alg *       unniss(Alg *alg);

#endif

