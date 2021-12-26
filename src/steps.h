#ifndef STEPS_H
#define STEPS_H

#include "pruning.h"

#define NSTEPS          50

extern Step *           steps[NSTEPS];

extern Step             optimal_HTM;
extern Step             optimal_light_HTM;
extern Step             eofin_eo;
extern Step             eofbfin_eofb;
extern Step             eorlfin_eorl;
extern Step             eoudfin_eoud;
extern Step             eoany_HTM;
extern Step             eofb_HTM;
extern Step             eorl_HTM;
extern Step             eoud_HTM;
extern Step             coany_HTM;
extern Step             coud_HTM;
extern Step             corl_HTM;
extern Step             cofb_HTM;
extern Step             coany_URF;
extern Step             coud_URF;
extern Step             corl_URF;
extern Step             cofb_URF;
extern Step             drany_HTM;
extern Step             drud_HTM;
extern Step             drrl_HTM;
extern Step             drfb_HTM;
extern Step             dr_eo;
extern Step             dr_eofb;
extern Step             dr_eorl;
extern Step             dr_eoud;
extern Step             drud_eofb;
extern Step             drrl_eofb;
extern Step             drud_eorl;
extern Step             drfb_eorl;
extern Step             drfb_eoud;
extern Step             drrl_eoud;
extern Step             dranyfin_DR;
extern Step             drudfin_drud;
extern Step             drrlfin_drrl;
extern Step             drfbfin_drfb;
extern Step             htr_any;
extern Step             htr_drud;
extern Step             htr_drrl;
extern Step             htr_drfb;
extern Step             htrfin_htr;
extern Step             cornershtr_HTM;
extern Step             cornershtr_URF;
extern Step             corners_HTM;
extern Step             corners_URF;

void                    copy_estimatedata(EstimateData *s, EstimateData *d);
void                    invert_estimatedata(EstimateData *ed);
void                    reset_estimatedata(EstimateData *ed);
void                    prepare_step(Step *step, SolveOptions *opts);

#endif
