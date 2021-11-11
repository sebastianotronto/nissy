#ifndef STEPS_H
#define STEPS_H

#include "cube.h"

/* Steps *********************************************************************/

extern Step             eofb_HTM;
extern Step             eorl_HTM;
extern Step             eoud_HTM;
extern Step             coud_HTM;
extern Step             corl_HTM;
extern Step             cofb_HTM;
extern Step             coud_URF;
extern Step             corl_URF;
extern Step             cofb_URF;
extern Step             corners_HTM;
extern Step             corners_URF;
extern Step             edges_HTM;
extern Step             drud_HTM;
extern Step             optimal_HTM;

/* Pruning tables ************************************************************/

extern PruneData        pd_eofb_HTM;
extern PruneData        pd_coud_HTM;
extern PruneData        pd_corners_HTM;
extern PruneData        pd_ep_HTM;
extern PruneData        pd_drud_HTM;

/* Movesets ******************************************************************/

bool                    moveset_HTM(Move m);
bool                    moveset_URF(Move m);

#endif
