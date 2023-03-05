#ifndef SOLVE_H
#define SOLVE_H

#include "movesets.h"

AlgList *   solve(Cube *cube, ChoiceStep *cs, SolveOptions *opts);
Alg *       solve_2phase(Cube *cube, int nthreads);

#endif
