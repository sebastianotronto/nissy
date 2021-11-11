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

/* Constants and macros *****************************************************/

#define POW2TO11            2048ULL
#define POW2TO12            4096ULL
#define POW3TO7             2187ULL
#define POW3TO8             6561ULL
#define FACTORIAL4          24ULL
#define FACTORIAL6          720ULL
#define FACTORIAL8          40320ULL
#define FACTORIAL12         479001600ULL
#define BINOM12ON4          495ULL
#define BINOM8ON4           70ULL
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))

#define NMOVES              (z3+1)
#define NTRANS              (mirror+1)
#define NROTATIONS          (NTRANS-1)
#define PTABLESIZE(n)       ((n+1) / 2)
#define PTABLEVAL(t,i)      (((i)%2) ? (t[(i)/2] / 16) : (t[(i)/2] % 16))

/* Type definitions **********************************************************/

#include "cubetypes.h"

/* Public functions **********************************************************/

Cube        apply_alg(Alg *alg, Cube cube);
Cube        apply_move(Move m, Cube cube);
Cube        apply_trans(Trans t, Cube cube);
bool        block_solved(Cube cube, Block);
Center      center_at(Cube cube, Center c);
Cube        compose(Cube c2, Cube c1); /* Use c2 as an alg on c1 */
Corner      corner_at(Cube cube, Corner c);
Edge        edge_at(Cube cube, Edge e);
uint64_t    epos_dependent_cube(Cube cube);
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

Move        base_move(Move m);
void        free_alg(Alg *alg);
void        free_alglist(AlgList *l);
Alg *       inverse_alg(Alg *alg);
Alg *       new_alg(char *str);
Alg *       on_inverse(Alg *alg);
void        print_alg(Alg *alg, bool l);
void        print_alglist(AlgList *al, bool l);
Alg *       trans_alg(Trans i);
void        transform_alg(Trans t, Alg *alg);

void        generate_ctable(CacheData *cd);
void        generate_ptable(PruneData *pd);
void        print_cachedata(CacheData *cd, bool printalgs);
void        print_ptable(PruneData *pd);

void        init();

#endif

