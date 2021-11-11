#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "cube.h"
#include "moves.h"
#include "utils.h"

#define NROTATIONS (mirror+1)

/* Letters indicate top and front centers
 * Mirror is wrt rl
 * Lowercase letter to distinguish from pieces */

typedef enum {
  uf, ur, ub, ul,
  df, dr, db, dl,
  rf, rd, rb, ru,
  lf, ld, lb, lu,
  fu, fr, fd, fl,
  bu, br, bd, bl,
  mirror,
} Transformation;

void print_transformation(Transformation t);

Cube transform_cube(Transformation t, Cube cube);
void transform_alg(Transformation t, NissMove *alg); /* Applied in-place */

void init_transformations(bool read, bool write);

#endif
