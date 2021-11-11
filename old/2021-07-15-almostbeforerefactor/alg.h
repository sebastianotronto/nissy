#ifndef ALG_H
#define ALG_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "macros.h"
#include "cubetypes.h"

Move        base_move(Move m);
void        free_alg(Alg *alg);
void        free_alglist(AlgList *l);
Alg *       inverse_alg(Alg *alg);
Move        inverse_move(Move m);
bool        moveset_HTM(Move m);
bool        moveset_URF(Move m);
Alg *       new_alg(char *str);
Alg *       on_inverse(Alg *alg);
void        print_alg(Alg *alg, bool l);
void        print_alglist(AlgList *al, bool l);
Alg *       rotation_alg(Trans i);
void        transform_alg(Trans t, Alg *alg);

#endif

