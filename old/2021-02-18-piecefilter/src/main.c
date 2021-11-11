#include <stdio.h>
#include "cube.h"
#include "moves.h"
#include "solve.h"

int main() {
  init_ttables(true, true);
  init_aux_tables();


  char moves[100] = "MR U'   B2 Bw F z xE2 M' x Dw' y Fw2 y2";
  NissMove alg[100];
  read_moves(moves, alg, 100);
  Cube cube = apply_alg(alg, blank_cube());

  /*f_eofb(cube);*/


/*  NissMove sol[MAXS][MAXM];*/
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

  NissMove a[5], b[5];
  read_moves("R", a, 5);
  read_moves("U", b, 5);
  Cube c1 = apply_alg(a,blank_cube()), c2 = apply_alg(b,blank_cube());
  print_cube(compose(c2,c1));
  print_cube(compose(c1,c2));
  /*print_cube(compose(c2,blank_cube()));*/

  NissMove nm[10];
  read_moves("y(y)RU", nm, 10);
  
  print_moves(nm);
  cleanup(nm, 10); 
  print_moves(nm);

  return 0;
}
