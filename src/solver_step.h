#ifndef SOLVER_STEP_H
#define SOLVER_STEP_H

#include "cube.h"
#include "solve.h"
#include "steps.h"

Solver *new_stepsolver_eager(Step *);
Solver *new_stepsolver_lazy(Step *);
void    free_stepsolver(Solver *);

#endif
