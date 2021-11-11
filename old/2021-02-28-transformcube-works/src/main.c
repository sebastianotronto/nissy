#include <stdio.h>
#include "cube.h"
#include "moves.h"
#include "solve.h"
#include "transformations.h"

int main() {
  init_ttables(true, true);
  init_aux_tables();
  init_transformations(true, true);


  /*char moves[100] = "MR U'   B2 Bw F z xE2 M' x Dw' y Fw2";*/

  /*char moves[100] = "M'U2MU2";*/
  char moves[100] = "R' D2 F2 U2 R F2 R D2 L' R2 D2 F D' L' U' B R' D' U R' B";
  NissMove alg[100];
  read_moves(moves, alg, 100);
  Cube cube = apply_alg(alg, (Cube){0});
  print_cube(cube);
  cube = transform_cube(rd, cube);
  print_cube(cube);

  /*f_eofb(cube);*/


  /*
  SolveData d = { .optimal_only = true, .available = standard_moveset,
                  .max_moves = 10,
                  .cleanup = true,
                  .max_solutions = 10,
                  .f = f_eofb };
  read_moves("y", d.pre_rotation, 2);
  int n = solve(cube, &d);
  printf("%d solutions found:\n", n);
  for (int i = 0; i < n; i++)
    print_moves(d.solutions[i]);
  */

/*
  NissMove a[5], b[5];
  read_moves("R", a, 5);
  read_moves("U", b, 5);
  Cube c1 = apply_alg(a,(Cube){0}), c2 = apply_alg(b,(Cube){0});
  print_cube(compose(c2,c1));
  print_cube(compose(c1,c2));

  NissMove nm[10];
  read_moves("y(y)RU", nm, 10);
  
  print_moves(nm);
  cleanup(nm, 10); 
  print_moves(nm);
  */

  return 0;
}
