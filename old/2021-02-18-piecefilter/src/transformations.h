#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "cube.h"
#include "moves.h"
#include "utils.h"

#define NROTATIONS (bl+1)

/* Letters indicate top and front centers
 * Lowercase letter to distinguish from pieces */
typedef enum {
  uf, ur, ub, ul,
  df, dr, db, dl,
  rf, rd, rb, ru,
  lf, ld, lb, lu,
  fu, fr, fd, fl,
  bu, br, bd, bl,
} Rotation;

/* Mirror is always on fb axis and applied after rotation */
typedef struct {
  Rotation rotation;
  bool mirror;
} Transformation;

void print_transformation(Transformation t);

Cube transform_cube(Transformation t, Cube cube);
void transform_alg(Transformation t, NissMove *alg); /* Applied in-place */

void init_transformations(bool read, bool write);

#endif
