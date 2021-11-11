#ifndef CUBE_H
#define CUBE_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include "cubetypes.h"

/* Steps *********************************************************************/

extern Step             eofb_HTM;
extern Step             eorl_HTM;
extern Step             eoud_HTM;
extern Step             coud_HTM;
extern Step             corl_HTM;
extern Step             cofb_HTM;
extern Step             coud_URF;
extern Step             corl_URF;
extern Step             cofb_URF;
extern Step             corners_HTM;
extern Step             corners_URF;
extern Step             edges_HTM;
extern Step             drud_HTM;
extern Step             optimal_HTM;

extern PruneData        pd_eofb_HTM;
extern PruneData        pd_coud_HTM;
extern PruneData        pd_corners_HTM;
extern PruneData        pd_ep_HTM;
extern PruneData        pd_drud_HTM;

/* Public functions **********************************************************/

Cube        apply_alg(Alg *alg, Cube cube);
Cube        apply_move(Move m, Cube cube);
Cube        apply_trans(Trans t, Cube cube);
bool        block_solved(Cube cube, Block);
Center      center_at(Cube cube, Center c);
Cube        compose(Cube c2, Cube c1); /* Use c2 as an alg on c1 */
Corner      corner_at(Cube cube, Corner c);
Edge        edge_at(Cube cube, Edge e);
bool        equal(Cube c1, Cube c2);
Cube        inverse_cube(Cube cube);
Move        inverse_move(Move m);
Trans       inverse_trans(Trans t);
bool        is_admissible(Cube cube);
bool        is_solved(Cube cube, bool reorient);
int         piece_orientation(Cube cube, int piece, char *orientation);
void        print_cube(Cube cube);
Cube        random_cube();
AlgList *   solve(Cube cube, Step step, SolveOptions *opts);

void        free_alg(Alg *alg);
void        free_alglist(AlgList *l);
Alg *       inverse_alg(Alg *alg);
Alg *       new_alg(char *str);
Alg *       on_inverse(Alg *alg);
void        print_alg(Alg *alg, bool l);
void        print_alglist(AlgList *al, bool l);
void        remove_last_move(Alg *alg);
void        transform_alg(Trans t, Alg *alg);

void        print_ptable(PruneData *pd);

void        init();

#endif

