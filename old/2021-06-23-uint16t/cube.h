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

#define POW2TO6             64ULL
#define POW2TO11            2048ULL
#define POW2TO12            4096ULL
#define POW3TO7             2187ULL
#define POW3TO8             6561ULL
#define FACTORIAL4          24ULL
#define FACTORIAL6          720ULL
#define FACTORIAL7          5040ULL
#define FACTORIAL8          40320ULL
#define FACTORIAL12         479001600ULL
#define BINOM12ON4          495ULL
#define BINOM8ON4           70ULL
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))

#define NMOVES              (z3+1)
#define NTRANS              (mirror+1)
#define NROTATIONS          (NTRANS-1)

/* Type definitions **********************************************************/

#include "cubetypes.h"

/* Public functions **********************************************************/

Cube        apply_alg(Alg *alg, Cube cube);
Cube        apply_move(Move m, Cube cube);
Cube        apply_trans(Trans t, Cube cube);
bool        block_solved(Cube cube, Block);
Cube        compose(Cube c2, Cube c1); /* Use c2 as an alg on c1 */
uint64_t    epos_dependent_cube(Cube cube);
bool        equal(Cube c1, Cube c2);
Cube        inverse_cube(Cube cube);
Move        inverse_move(Move m);
Trans       inverse_trans(Trans t);
bool        is_admissible(Cube cube);
bool        is_solved(Cube cube, bool reorient);
bool        is_solved_center(Cube cube, Center c);
bool        is_solved_corner(Cube cube, Corner c);
bool        is_solved_edge(Cube cube, Edge e);
void        print_cube(Cube cube);
Cube        random_cube();
AlgList *   solve(Cube cube, Step step, SolveOptions *opts);
Center      what_center_at(Cube cube, Center c);
Corner      what_corner_at(Cube cube, Corner c);
Edge        what_edge_at(Cube cube, Edge e);
int         what_orientation_corner(int co, Corner c);
int         what_orientation_edge(int eo, Edge e);
Center      where_is_center(Cube cube, Center c);
Corner      where_is_corner(Cube cube, Corner c);
Edge        where_is_edge(Cube cube, Edge e);

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

void        genalgset(AlgSet *as);
void        genptable(PruneData *pd);
void        print_ptable(PruneData *pd);
uint64_t    ptablesize(PruneData *pd);
int         ptableval(PruneData *pd, uint64_t ind);

void        init();

#endif

