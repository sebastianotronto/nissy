#ifndef STEPS_H
#define STEPS_H

#include "pruning.h"

#define NSTEPS          50

extern Step *           steps[NSTEPS];

void                    free_localinfo(LocalInfo *li);
LocalInfo *             new_localinfo();
void                    prepare_step(Step *step, int nthreads);

#endif
