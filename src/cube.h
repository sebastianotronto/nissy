#ifndef CUBE_H
#define CUBE_H

#include <stdio.h>

#include "cubetypes.h"
#include "env.h"
#include "utils.h"

void        compose(Cube *c2, Cube *c1); /* Use c2 as an alg on c1 */
void        compose_centers(Cube *c2, Cube *c1);
void        compose_corners(Cube *c2, Cube *c1);
void        compose_edges(Cube *c2, Cube *c1);
void        copy_cube(Cube *src, Cube *dst);
void        copy_cube_centers(Cube *src, Cube *dst);
void        copy_cube_corners(Cube *src, Cube *dst);
void        copy_cube_edges(Cube *src, Cube *dst);
bool        equal(Cube *c1, Cube *c2);
void        invert_cube(Cube *cube);
void        invert_cube_centers(Cube *cube);
void        invert_cube_corners(Cube *cube);
void        invert_cube_edges(Cube *cube);
bool        is_admissible(Cube *cube);
bool        is_solved(Cube *cube);
void        make_solved(Cube *cube);
void        make_solved_centers(Cube *cube);
void        make_solved_corners(Cube *cube);
void        make_solved_edges(Cube *cube);
void        print_cube(Cube *cube);
int         where_is_center(Center x, Cube *c);
int         where_is_corner(Corner k, Cube *c);
int         where_is_edge(Edge e, Cube *c);

#endif

