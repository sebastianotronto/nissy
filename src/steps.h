#ifndef STEPS_H
#define STEPS_H

#include "pruning.h"

#define NSTEPS          50

extern Step *           steps[NSTEPS];

void                    copy_estimatedata(EstimateData *s, EstimateData *d);
void                    free_estimatedata(EstimateData *ed);
void                    invert_estimatedata(EstimateData *ed);
EstimateData *          new_estimatedata();
void                    prepare_step(Step *step, SolveOptions *opts);

#endif
