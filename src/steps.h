#ifndef STEPS_H
#define STEPS_H

#include "pruning.h"

bool                    check_centers(Cube *cube);
bool                    check_coud_HTM(Cube *cube);
bool                    check_coud_URF(Cube *cube);
bool                    check_cp_HTM(Cube *cube);
bool                    check_corners_HTM(Cube *cube);
bool                    check_corners_URF(Cube *cube);
bool                    check_cornershtr(Cube *cube);
bool                    check_eofb(Cube *cube);
bool                    check_drud(Cube *cube);
bool                    check_htr(Cube *cube);
void                    compute_ind(Step *a, Cube *cube, Movable *ind);
void                    prepare_cs(ChoiceStep *cs, SolveOptions *opts);
bool                    always_valid(Alg *alg);
bool                    validate_singlecw_ending(Alg *alg);

#ifndef STEPS_C

extern char check_centers_msg[100];
extern char check_eo_msg[100];
extern char check_dr_msg[100];
extern char check_htr_msg[100];
extern char check_drany_msg[100];

extern Step step_eofb_HTM;
extern Step step_drud_HTM;
extern Step step_drfin_drud;

extern ChoiceStep optimal_HTM;
extern ChoiceStep eoany_HTM;
extern ChoiceStep eofb_HTM;
extern ChoiceStep eorl_HTM;
extern ChoiceStep eoud_HTM;
extern ChoiceStep drany_HTM;
extern ChoiceStep drud_HTM;
extern ChoiceStep drrl_HTM;
extern ChoiceStep drfb_HTM;
extern ChoiceStep dranyfin_DR;
extern ChoiceStep drudfin_drud;
extern ChoiceStep drrlfin_drrl;
extern ChoiceStep drfbfin_drfb;

extern ChoiceStep *csteps[];

#else

char check_centers_msg[100] = "cube must be oriented (centers solved)";
char check_eo_msg[100]      = "EO must be solved on given axis";
char check_dr_msg[100]      = "DR must be solved on given axis";
char check_htr_msg[100]     = "HTR must be solved";
char check_drany_msg[100]   = "DR must be solved on at least one axis";

/* Optimal after EO ******************/
/* TODO: eofin_eo (generic), eofbfin_eofb, eorlfin_eorl, eoudfin_eoud */

/* EO steps **************************/
/* TODO: eoany_HTM (generic), eofb_HTM, eorl_HTM, eoud_HTM */

Step
step_eofb_HTM = {
	.ready          = check_centers,
	.final          = false,
	.moveset        = &moveset_HTM,
	.n_coord        = 1,
	.coord          = {&coord_eofb},
	.coord_trans    = {uf},
	.is_valid       = validate_singlecw_ending,
};
ChoiceStep
eoany_HTM = {
	.shortname = "eo",
	.name      = "EO on any axis",
	.step      = {&step_eofb_HTM, &step_eofb_HTM, &step_eofb_HTM, NULL},
	.t         = {uf, ur, fd},
	.ready_msg = check_centers_msg,
};
ChoiceStep
eofb_HTM = {
	.shortname = "eofb",
	.name      = "EO on F/B",
	.step      = {&step_eofb_HTM, NULL},
	.t         = {uf},
	.ready_msg = check_centers_msg,
};
ChoiceStep
eorl_HTM = {
	.shortname = "eorl",
	.name      = "EO on R/L",
	.step      = {&step_eofb_HTM, NULL},
	.t         = {ur},
	.ready_msg = check_centers_msg,
};
ChoiceStep
eoud_HTM = {
	.shortname = "eoud",
	.name      = "EO on U/D",
	.step      = {&step_eofb_HTM, NULL},
	.t         = {fd},
	.ready_msg = check_centers_msg,
};

/* CO steps **************************/
/* TODO: coany_HTM (generic), cofb_HTM, corl_HTM, coud_HTM */
/* TODO: coany_URF (generic), cofb_URF, corl_URF, coud_URF */

/* Misc corner steps *****************/
/* TODO: cornershtr_HTM, cornershtr_URF, corners_HTM, corners_URF */
/* TODO (new): corners_drud */

/* DR steps **************************/
/* TODO: dr_eo (generic) */
/* TODO: dr_eofb (generic), dr_eorl (generic), dr_eoud (generic) */
/* TODO: drud_eofb, drrl_eofb, drud_eorl, drfb_eorl, drrl_eoud, drfb_eoud */

Step
step_drud_HTM = {
	.ready          = check_centers,
	.final          = false,
	.moveset        = &moveset_HTM,
	.n_coord        = 1,
	.coord          = {&coord_drud_sym16},
	.coord_trans    = {uf},
	.is_valid       = validate_singlecw_ending,
};
ChoiceStep
drany_HTM = {
	.shortname = "dr",
	.name      = "DR on any axis",
	.step      = {&step_drud_HTM, &step_drud_HTM, &step_drud_HTM, NULL},
	.t         = {uf, rf, fd},
	.ready_msg = check_centers_msg,
};
ChoiceStep
drud_HTM = {
	.shortname = "drud",
	.name      = "DR on U/D",
	.step      = {&step_drud_HTM, NULL},
	.t         = {uf},
	.ready_msg = check_centers_msg,
};
ChoiceStep
drrl_HTM = {
	.shortname = "drrl",
	.name      = "DR on R/L",
	.step      = {&step_drud_HTM, NULL},
	.t         = {rf},
	.ready_msg = check_centers_msg,
};
ChoiceStep
drfb_HTM = {
	.shortname = "drfb",
	.name      = "DR on F/B",
	.step      = {&step_drud_HTM, NULL},
	.t         = {fd},
	.ready_msg = check_centers_msg,
};

/* DR finish steps */
Step
step_drfin_drud = {
	.ready          = check_drud,
	.final          = true,
	.moveset        = &moveset_drud,
	.n_coord        = 1,
	.coord          = {&coord_drudfin_noE_sym16}, /* TODO: maybe no noE */
	.coord_trans    = {uf},
	.is_valid       = NULL,
};
ChoiceStep
dranyfin_DR = {
	.shortname = "drfin",
	.name      = "DR finish on any axis without breaking DR",
	.step      = {&step_drfin_drud, &step_drfin_drud,
	              &step_drfin_drud, NULL},
	.t         = {uf, rf, fd},
	.ready_msg = check_dr_msg,
};
ChoiceStep
drudfin_drud = {
	.shortname = "drudfin",
	.name      = "DR finis on U/D without breaking DR",
	.step      = {&step_drfin_drud, NULL},
	.t         = {uf},
	.ready_msg = check_dr_msg,
};
ChoiceStep
drrlfin_drrl = {
	.shortname = "drrlfin",
	.name      = "DR finish on R/L without breaking DR",
	.step      = {&step_drfin_drud, NULL},
	.t         = {rf},
	.ready_msg = check_dr_msg,
};
ChoiceStep
drfbfin_drfb = {
	.shortname = "drfbfin",
	.name      = "DR finish on F/B without breaking DR",
	.step      = {&step_drfin_drud, NULL},
	.t         = {fd},
	.ready_msg = check_dr_msg,
};

/* HTR from DR */
/* TODO: htr_any (generic), htr_drud, htr_drrl, htr_drfb */

/* HTR finish */
/* TODO: htrfin_htr */

ChoiceStep *csteps[] = {
/* TODO: re-implement optimal
	&optimal_HTM,
*/

	&eoany_HTM, &eofb_HTM, &eorl_HTM, &eoud_HTM,
	&drany_HTM, &drud_HTM, &drrl_HTM, &drfb_HTM,
	&dranyfin_DR, &drudfin_drud, &drrlfin_drrl, &drfbfin_drfb,

NULL
/* TODO:
	&optimal_light_HTM,

	&eofin_eo, &eofbfin_eofb, &eorlfin_eorl, &eoudfin_eoud,
	&coany_HTM, &coud_HTM, &corl_HTM, &cofb_HTM,
	&coany_URF, &coud_URF, &corl_URF, &cofb_URF,
	&dr_eo, &dr_eofb, &dr_eorl, &dr_eoud,
	&drud_eofb, &drrl_eofb,
	&drud_eorl, &drfb_eorl,
	&drfb_eoud, &drrl_eoud,
	&htr_any, &htr_drud, &htr_drrl, &htr_drfb,
	&htrfin_htr,
	&cornershtr_HTM, &cornershtr_URF, &corners_HTM, &corners_URF,
	NULL
*/

};

#endif

#endif
