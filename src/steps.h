#ifndef STEPS_H
#define STEPS_H

#include "pruning.h"

#define NSTEPS          50

extern Step *           steps[NSTEPS];

void                    copy_estimatedata(EstimateData *s, EstimateData *d);
void                    invert_estimatedata(EstimateData *ed);
void                    reset_estimatedata(EstimateData *ed);
void                    prepare_step(Step *step, SolveOptions *opts);

#endif
