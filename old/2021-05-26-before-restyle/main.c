#include <stdio.h>
#include "cube.h"
#include "moves.h"
#include "solve.h"
#include "transformations.h"

int main() {
  init_ttables(true, true);
  init_aux_tables();
  init_transformations(true, true);


  char moves[100] = "R' D2 F2 U2 R F2 R D2 L' R2 D2 F D' L' U' B R' D' U R' B";
  NissMove alg[100];
  read_moves(moves, alg, 100);
  Cube cube = apply_alg(alg, (Cube){0});
  print_cube(transform_cube(rd, cube));

  transform_alg(rd, alg);
  print_alg(alg);


  return 0;
}
