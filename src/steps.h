#ifndef STEPS_H
#define STEPS_H

#include "pruning.h"

#define NSTEPS          50

extern Step *           steps[NSTEPS];

void                    prepare_step(Step *step);

#endif
