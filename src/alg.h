#ifndef ALG_H
#define ALG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cubetypes.h"
#include "utils.h"

bool        allowed_all(Move m);
bool        allowed_HTM(Move m);
bool        allowed_URF(Move m);
bool        allowed_eofb(Move m);
bool        allowed_drud(Move m);
bool        allowed_htr(Move m);
bool        allowed_next_all(Move l2, Move l1, Move m);

void        moveset_to_list(Moveset ms, Move *lst);
void        init_moveset(Moveset *ms);
bool        possible_next(Move m, Moveset *ms, Move l0, Move l1);

void        append_alg(AlgList *l, Alg *alg);
void        append_move(Alg *alg, Move m, bool inverse);
Move        base_move(Move m);
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
void        swapmove(Move *m1, Move *m2);
char *      trans_string(Trans t); /* Here because similar to move_string, move? */
Alg *       unniss(Alg *alg);

/* Movesets ******************************************************************/

#ifndef ALG_C

extern Moveset moveset_HTM;
extern Moveset moveset_URF;
extern Moveset moveset_eofb;
extern Moveset moveset_drud;
extern Moveset moveset_htr;

extern Moveset * all_ms[];

#else

Moveset
moveset_HTM = {
	.name         = "HTM",
	.allowed      = allowed_HTM,
	.allowed_next = allowed_next_all,
};

Moveset
moveset_URF = {
	.name         = "URF",
	.allowed      = allowed_URF,
	.allowed_next = allowed_next_all,
};

Moveset
moveset_eofb = {
	.name         = "eofb",
	.allowed      = allowed_eofb,
	.allowed_next = allowed_next_all,
};

Moveset
moveset_drud = {
	.name         = "drud",
	.allowed      = allowed_drud,
	.allowed_next = allowed_next_all,
};

Moveset
moveset_htr = {
	.name         = "htr",
	.allowed      = allowed_htr,
	.allowed_next = allowed_next_all,
};

Moveset *
all_ms[] = {
	&moveset_HTM,
	&moveset_URF,
	&moveset_eofb,
	&moveset_drud,
	&moveset_htr,
	NULL
};

#endif

#endif

