#ifndef CUBE_H
#define CUBE_H

#include <stdio.h>
#include <time.h>

#include "pf.h"
#include "utils.h"

Cube        admissible_ep(Cube cube, PieceFilter f); /* TODO: move? */
Cube        arrays_to_cube(CubeArray *arr, PieceFilter f); /* TODO: remove */
Cube        compose(Cube c2, Cube c1); /* Use c2 as an alg on c1 */
Cube        compose_filtered(Cube c2, Cube c1, PieceFilter f);
void        cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f);
int         edge_slice(Edge e); /* E=0, S=1, M=2 */
bool        equal(Cube c1, Cube c2);
Cube        inverse_cube(Cube cube);
bool        is_admissible(Cube cube);
bool        is_solved(Cube cube);
bool        block_solved(Cube cube, Block); /*TODO: rename to is_solved_block()*/
bool        is_solved_center(Cube cube, Center c);
bool        is_solved_corner(Cube cube, Corner c);
bool        is_solved_edge(Cube cube, Edge e);
void        epos_to_partial_ep(int epos, int *ep, int *ss);
void        free_cubearray(CubeArray *arr, PieceFilter f); /* TODO: remove */
Cube        move_via_arrays(CubeArray *arr, Cube c, PieceFilter pf);
CubeArray * new_cubearray(Cube cube, PieceFilter f); /* TODO: remove */
void        print_cube(Cube cube);
Cube        random_cube();
Center      what_center_at(Cube cube, Center c);
Corner      what_corner_at(Cube cube, Corner c);
Edge        what_edge_at(Cube cube, Edge e);
int         what_orientation_corner(int co, Corner c);
int         what_orientation_edge(int eo, Edge e);
Center      where_is_center(Cube cube, Center c);
Corner      where_is_corner(Cube cube, Corner c);
Edge        where_is_edge(Cube cube, Edge e);

#endif

