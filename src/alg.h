#ifndef ALG_H
#define ALG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cubetypes.h"
#include "utils.h"

bool        moveset_HTM(Move m);
bool        moveset_URF(Move m);
bool        moveset_eofb(Move m);
bool        moveset_drud(Move m);
bool        moveset_htr(Move m);

void        append_alg(AlgList *l, Alg *alg);
void        append_move(Alg *alg, Move m, bool inverse);
void        compose_alg(Alg *alg1, Alg *alg2);
Move        base_move(Move m);
void        free_alg(Alg *alg);
void        free_alglist(AlgList *l);
Alg *       inverse_alg(Alg *alg);
Move        inverse_move(Move m);
char *      move_string(Move m);
void        movelist_to_position(Move *ml, int *pos);
void        moveset_to_list(Moveset ms, Move *lst);
Alg *       new_alg(char *str);
AlgList *   new_alglist();
Alg *       on_inverse(Alg *alg);
void        print_alg(Alg *alg, bool l);
void        print_alglist(AlgList *al, bool l);
void        swapmove(Move *m1, Move *m2);
void        unniss(Alg *alg);

#endif

