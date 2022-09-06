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
void                    compute_ind(StepAlt *a, Cube *cube, Movable *ind);
int                     estimate_stepalt(StepAlt *a, Movable *ind, int goal);
void                    prepare_step(Step *step, SolveOptions *opts);
bool                    always_valid(Alg *alg);
bool                    validate_singlecw_ending(Alg *alg);

#ifndef STEPS_C

extern char check_centers_msg[100];
extern char check_eo_msg[100];
extern char check_dr_msg[100];
extern char check_htr_msg[100];
extern char check_drany_msg[100];

extern StepAlt sa_nxopt31_HTM;
extern StepAlt sa_eofb_HTM;
extern StepAlt sa_drud_HTM;
extern StepAlt sa_drfin_drud;

extern Step optimal_HTM;
extern Step eoany_HTM;
extern Step eofb_HTM;
extern Step eorl_HTM;
extern Step eoud_HTM;
extern Step drany_HTM;
extern Step drud_HTM;
extern Step drrl_HTM;
extern Step drfb_HTM;
extern Step dranyfin_DR;
extern Step drudfin_drud;
extern Step drrlfin_drrl;
extern Step drfbfin_drfb;

extern Step *steps[];

#else

char check_centers_msg[100] = "cube must be oriented (centers solved)";
char check_eo_msg[100]      = "EO must be solved on given axis";
char check_dr_msg[100]      = "DR must be solved on given axis";
char check_htr_msg[100]     = "HTR must be solved";
char check_drany_msg[100]   = "DR must be solved on at least one axis";

/* Optimal solvers *******************/
/* TODO: build options for smaller optimal solvers */

StepAlt
sa_nxopt31_HTM = {
	.ready          = check_centers,
	.final          = true,
	.moveset        = &moveset_HTM,
	.n_coord        = 6,
	.coord          = {&coord_nxopt31, &coord_nxopt31, &coord_nxopt31,
			   &coord_eposepe, &coord_eposepe, &coord_eposepe},
			  /* TODO: use khuge as fallback? */
	.coord_trans    = {uf, rd, bl, uf, rd, bl},
	.compact_pd     = {true, true, true, false, false, false},
	.fallback_coord = {&coord_drud_sym16, &coord_drud_sym16,
	                   &coord_drud_sym16},
	.fbmod          = {BINOM8ON4, BINOM8ON4, BINOM8ON4},
	.n_dbtrick      = 1, /* TODO: change to 2 when khuge */
	.dbtrick        = {{0, 1, 2}},
	.is_valid       = NULL,
};
Step
optimal_HTM = {
	.shortname = "optimal",
	.name      = "Optimal solve (in HTM)",
	.alt       = {&sa_nxopt31_HTM, NULL},
	.t         = {uf},
	.ready_msg = check_centers_msg,
};

/* Optimal after EO ******************/
/* TODO: eofin_eo (generic), eofbfin_eofb, eorlfin_eorl, eoudfin_eoud */

/* EO steps **************************/
/* TODO: eoany_HTM (generic), eofb_HTM, eorl_HTM, eoud_HTM */

StepAlt
sa_eofb_HTM = {
	.ready          = check_centers,
	.final          = false,
	.moveset        = &moveset_HTM,
	.n_coord        = 1,
	.coord          = {&coord_eofb},
	.coord_trans    = {uf},
	.compact_pd     = {false},
	.n_dbtrick      = 0,
	.is_valid       = validate_singlecw_ending,
};
Step
eoany_HTM = {
	.shortname = "eo",
	.name      = "EO on any axis",
	.alt       = {&sa_eofb_HTM, &sa_eofb_HTM, &sa_eofb_HTM, NULL},
	.t         = {uf, ur, fd},
	.ready_msg = check_centers_msg,
};
Step
eofb_HTM = {
	.shortname = "eofb",
	.name      = "EO on F/B",
	.alt       = {&sa_eofb_HTM, NULL},
	.t         = {uf},
	.ready_msg = check_centers_msg,
};
Step
eorl_HTM = {
	.shortname = "eorl",
	.name      = "EO on R/L",
	.alt       = {&sa_eofb_HTM, NULL},
	.t         = {ur},
	.ready_msg = check_centers_msg,
};
Step
eoud_HTM = {
	.shortname = "eoud",
	.name      = "EO on U/D",
	.alt       = {&sa_eofb_HTM, NULL},
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

StepAlt
sa_drud_HTM = {
	.ready          = check_centers,
	.final          = false,
	.moveset        = &moveset_HTM,
	.n_coord        = 1,
	.coord          = {&coord_drud_sym16},
	.coord_trans    = {uf},
	.compact_pd     = {false}, /* TODO: maybe compactify */
	.n_dbtrick      = 0,
	.is_valid       = validate_singlecw_ending,
};
Step
drany_HTM = {
	.shortname = "dr",
	.name      = "DR on any axis",
	.alt       = {&sa_drud_HTM, &sa_drud_HTM, &sa_drud_HTM, NULL},
	.t         = {uf, rf, fd},
	.ready_msg = check_centers_msg,
};
Step
drud_HTM = {
	.shortname = "drud",
	.name      = "DR on U/D",
	.alt       = {&sa_drud_HTM, NULL},
	.t         = {uf},
	.ready_msg = check_centers_msg,
};
Step
drrl_HTM = {
	.shortname = "drrl",
	.name      = "DR on R/L",
	.alt       = {&sa_drud_HTM, NULL},
	.t         = {rf},
	.ready_msg = check_centers_msg,
};
Step
drfb_HTM = {
	.shortname = "drfb",
	.name      = "DR on F/B",
	.alt       = {&sa_drud_HTM, NULL},
	.t         = {fd},
	.ready_msg = check_centers_msg,
};

/* DR finish steps */
StepAlt
sa_drfin_drud = {
	.ready          = check_drud,
	.final          = true,
	.moveset        = &moveset_drud,
	.n_coord        = 1,
	.coord          = {&coord_drudfin_noE_sym16}, /* TODO: maybe no noE */
	.coord_trans    = {uf},
	.compact_pd     = {false}, /* TODO: maybe compactify */
	.n_dbtrick      = 0,
	.is_valid       = NULL,
};
Step
dranyfin_DR = {
	.shortname = "drfin",
	.name      = "DR finish on any axis without breaking DR",
	.alt       = {&sa_drfin_drud, &sa_drfin_drud, &sa_drfin_drud, NULL},
	.t         = {uf, rf, fd},
	.ready_msg = check_dr_msg,
};
Step
drudfin_drud = {
	.shortname = "drudfin",
	.name      = "DR finis on U/D without breaking DR",
	.alt       = {&sa_drfin_drud, NULL},
	.t         = {uf},
	.ready_msg = check_dr_msg,
};
Step
drrlfin_drrl = {
	.shortname = "drrlfin",
	.name      = "DR finish on R/L without breaking DR",
	.alt       = {&sa_drfin_drud, NULL},
	.t         = {rf},
	.ready_msg = check_dr_msg,
};
Step
drfbfin_drfb = {
	.shortname = "drfbfin",
	.name      = "DR finish on F/B without breaking DR",
	.alt       = {&sa_drfin_drud, NULL},
	.t         = {fd},
	.ready_msg = check_dr_msg,
};

/* HTR from DR */
/* TODO: htr_any (generic), htr_drud, htr_drrl, htr_drfb */

/* HTR finish */
/* TODO: htrfin_htr */

Step *steps[] = {
	&optimal_HTM, /* first is default */

	&eoany_HTM, &eofb_HTM, &eorl_HTM, &eoud_HTM,
	&drany_HTM, &drud_HTM, &drrl_HTM, &drfb_HTM,
	&dranyfin_DR, &drudfin_drud, &drrlfin_drrl, &drfbfin_drfb,

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
