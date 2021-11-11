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

#include "macros.h"
#include "cubetypes.h"

extern Coordinate       coord_eofb;
extern Coordinate       coord_eofbepos;
extern Coordinate       coord_coud;
extern Coordinate       coord_corners;
extern Coordinate       coord_cornershtr;
extern Coordinate       coord_drud;
extern Coordinate       coord_coud_sym16;
extern Coordinate       coord_eofbepos_sym16;
extern Coordinate       coord_drud_sym16;
extern Coordinate       coord_khuge;

Cube        apply_alg(Alg *alg, Cube cube);
Cube        apply_move(Move m, Cube cube);
Cube        apply_trans(Trans t, Cube cube);
bool        block_solved(Cube cube, Block);
Cube        compose(Cube c2, Cube c1); /* Use c2 as an alg on c1 */
uint64_t    cphtr(Cube cube); /* TODO: rename (something with cosets) */
Cube        anti_cphtr(uint64_t ind); /*TODO also this */
uint64_t    epos_dependent(Cube cube); /* TODO: rename and turn into an indexer */
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

bool        check_centers(Cube cube);
bool        check_corners(Cube cube);
bool        check_cornershtr(Cube cube);
bool        check_coud(Cube cube);
bool        check_drud(Cube cube);
bool        check_eofb(Cube cube);
bool        check_eofbepos(Cube cube);
bool        check_epose(Cube cube);
bool        check_ep(Cube cube);
bool        check_khuge(Cube cube);
bool        check_nothing(Cube cube);

bool        moveset_HTM(Move m);
bool        moveset_URF(Move m);

Move        base_move(Move m);
void        free_alg(Alg *alg);
void        free_alglist(AlgList *l);
Alg *       inverse_alg(Alg *alg);
Alg *       new_alg(char *str);
Alg *       on_inverse(Alg *alg);
void        print_alg(Alg *alg, bool l);
void        print_alglist(AlgList *al, bool l);
Alg *       rotation_alg(Trans i);
void        transform_alg(Trans t, Alg *alg);

void        genptable(PruneData *pd);
void        print_ptable(PruneData *pd);
uint64_t    ptablesize(PruneData *pd);
int         ptableval(PruneData *pd, Cube cube);

void        init();

#endif

