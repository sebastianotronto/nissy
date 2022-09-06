#ifndef CUBE_H
#define CUBE_H

#include <stdio.h>

#include "cubetypes.h"
#include "env.h"
#include "utils.h"

void        compose(Cube *c2, Cube *c1); /* Use c2 as an alg on c1 */
void        copy_cube(Cube *src, Cube *dst);
bool        equal(Cube *c1, Cube *c2);
void        invert_cube(Cube *cube);
bool        is_admissible(Cube *cube);
bool        is_solved(Cube *cube);
void        make_solved(Cube *cube);
void        print_cube(Cube *cube);

#endif

