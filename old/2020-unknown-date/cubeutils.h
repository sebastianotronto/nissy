/* Cube-specific utility functions */

#ifndef CUBEUTILS_H
#define CUBEUTILS_H

#include <stdbool.h>
#include "pieces.h"

void cube_to_ep_array(Cube *cube, int ep[12]);
void ep_array_to_epos(int ep[12], Cube *cube);

Cube *solved_cube();
bool solvable(Cube *cube);
bool solved(Cube *cube);

void print(Cube *cube);

#endif
