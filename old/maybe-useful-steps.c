#include "steps.h"

#define UPDATECHECKSTOP(a, b, c)     if ((a=(MAX((a),(b))))>(c)) return (a);

static int              estimate_stepalt(StepAlt *a, uint64_t *ind);

/* Checkers and validators ***************************************************/

/* TODO: these should be with Cube *cube (and all need to change) */
/* Maybe move them to cube.c */
static bool             check_centers(Cube cube);
static bool             check_coud_HTM(Cube cube);
static bool             check_coud_URF(Cube cube);
static bool             check_corners_HTM(Cube cube);
static bool             check_corners_URF(Cube cube);
static bool             check_cornershtr(Cube cube);
static bool             check_eofb(Cube cube);
static bool             check_drud(Cube cube);
static bool             check_htr(Cube cube);

static bool             always_valid(Alg *alg);
static bool             validate_singlecw_ending(Alg *alg);

/* Messages for when cube is not ready ***************************************/

static char check_centers_msg[100] = "cube must be oriented (centers solved)";
static char check_eo_msg[100]      = "EO must be solved on given axis";
static char check_dr_msg[100]      = "DR must be solved on given axis";
static char check_htr_msg[100]     = "HTR must be solved";
static char check_drany_msg[100]   = "DR must be solved on at least one axis";

/* Steps *********************************************************************/

/* Optimal solvers *******************/

Step
optimal_HTM = {
	.shortname = "optimal",
	.name      = "Optimal solve (in HTM)",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_nxopt31_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_nxopt31_HTM, &pd_corners_HTM},
	.ntables   = 2,
};

Step
optimal_light_HTM = {
	.shortname = "light",
	.name      = "Optimal solve (in HTM), small table (500Mb RAM total)",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_light_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_drud_sym16_HTM, &pd_corners_HTM},
	.ntables   = 2,
};

/* Optimal after EO ******************/

Step
eofin_eo = {
	.shortname = "eofin",
	.name      = "Optimal solve after EO without breaking EO (detected)",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_nxopt31_HTM,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_eofb,

	.detect    = detect_pretrans_eofb,

	.tables    = {&pd_nxopt31_HTM, &pd_corners_HTM},
	.ntables   = 2,
};

Step
eofbfin_eofb = {
	.shortname = "eofbfin",
	.name      = "Optimal after EO on F/B without breaking EO",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_nxopt31_HTM,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_eofb,

	.pre_trans = uf,

	.tables    = {&pd_nxopt31_HTM, &pd_corners_HTM},
	.ntables   = 2,
};

Step
eorlfin_eorl = {
	.shortname = "eorlfin",
	.name      = "Optimal after EO on R/L without breaking EO",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_nxopt31_HTM,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_eofb,

	.pre_trans = ur,

	.tables    = {&pd_nxopt31_HTM, &pd_corners_HTM},
	.ntables   = 2,
};

Step
eoudfin_eoud = {
	.shortname = "eoudfin",
	.name      = "Optimal after EO on U/D without breaking EO",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_nxopt31_HTM,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_eofb,

	.pre_trans = fd,

	.tables    = {&pd_nxopt31_HTM, &pd_corners_HTM},
	.ntables   = 2,
};

/* EO steps **************************/
Step
eoany_HTM = {
	.shortname = "eo",
	.name      = "EO on any axis",

	.final     = false,
	.is_done   = check_eofb,
	.estimate  = estimate_eofb_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.detect    = detect_pretrans_void_3axis,

	.tables    = {&pd_eofb_HTM},
	.ntables   = 1,
};

Step
eofb_HTM = {
	.shortname = "eofb",
	.name      = "EO on F/B",

	.final     = false,
	.is_done   = check_eofb,
	.estimate  = estimate_eofb_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_eofb_HTM},
	.ntables   = 1,
};

Step
eorl_HTM = {
	.shortname = "eorl",
	.name      = "EO on R/L",

	.final     = false,
	.is_done   = check_eofb,
	.estimate  = estimate_eofb_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.pre_trans = ur,

	.tables    = {&pd_eofb_HTM},
	.ntables   = 1,
};

Step
eoud_HTM = {
	.shortname = "eoud",
	.name      = "EO on U/D",

	.final     = false,
	.is_done   = check_eofb,
	.estimate  = estimate_eofb_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.pre_trans = fd,

	.tables    = {&pd_eofb_HTM},
	.ntables   = 1,
};

/* CO steps **************************/
Step
coany_HTM = {
	.shortname = "co",
	.name      = "CO on any axis",

	.final     = false,
	.is_done   = check_coud_HTM,
	.estimate  = estimate_coud_HTM,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.detect    = detect_pretrans_void_3axis,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
coud_HTM = {
	.shortname = "coud",
	.name      = "CO on U/D",

	.final     = false,
	.is_done   = check_coud_HTM,
	.estimate  = estimate_coud_HTM,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
corl_HTM = {
	.shortname = "corl",
	.name      = "CO on R/L",

	.final     = false,
	.is_done   = check_coud_HTM,
	.estimate  = estimate_coud_HTM,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.pre_trans = rf,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
cofb_HTM = {
	.shortname = "cofb",
	.name      = "CO on F/B",

	.final     = false,
	.is_done   = check_coud_HTM,
	.estimate  = estimate_coud_HTM,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.pre_trans = fd,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
coany_URF = {
	.shortname = "co-URF",
	.name      = "CO any axis (URF moveset)",

	.final     = false,
	.is_done   = check_coud_URF,
	.estimate  = estimate_coud_URF,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_URF,

	.detect    = detect_pretrans_void_3axis,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
coud_URF = {
	.shortname = "coud-URF",
	.name      = "CO on U/D (URF moveset)",

	.final     = false,
	.is_done   = check_coud_URF,
	.estimate  = estimate_coud_URF,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_URF,

	.pre_trans = uf,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
corl_URF = {
	.shortname = "corl-URF",
	.name      = "CO on R/L (URF moveset)",

	.final     = false,
	.is_done   = check_coud_URF,
	.estimate  = estimate_coud_URF,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_URF,

	.pre_trans = rf,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
cofb_URF = {
	.shortname = "cofb-URF",
	.name      = "CO on F/B (URF moveset)",

	.final     = false,
	.is_done   = check_coud_URF,
	.estimate  = estimate_coud_URF,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_URF,

	.pre_trans = fd,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

/* Misc corner steps *****************/
Step
cornershtr_HTM = {
	.shortname = "chtr",
	.name      = "Solve corners to HTR state",

	.final     = false,
	.is_done   = check_cornershtr,
	.estimate  = estimate_cornershtr_HTM,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_cornershtr_HTM},
	.ntables   = 1,
};

Step
cornershtr_URF = {
	.shortname = "chtr-URF",
	.name      = "Solve corners to HTR state (URF moveset)",

	.final     = false,
	.is_done   = check_cornershtr,
	.estimate  = estimate_cornershtr_URF,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_URF,

	.pre_trans = uf,

	.tables    = {&pd_cornershtr_HTM},
	.ntables   = 1,
};

Step
corners_HTM = {
	.shortname = "corners",
	.name      = "Solve corners",

	.final     = true,
	.is_done   = check_corners_HTM,
	.estimate  = estimate_corners_HTM,
	.ready     = NULL,
	.is_valid  = always_valid,
	.moveset   = &moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_corners_HTM},
	.ntables   = 1,
};

Step
corners_URF = {
	.shortname = "corners-URF",
	.name      = "Solve corners (URF moveset)",

	.final     = true, /* TODO: check if this works with reorient */
	.is_done   = check_corners_URF,
	.estimate  = estimate_corners_URF,
	.ready     = NULL,
	.is_valid  = always_valid,
	.moveset   = &moveset_URF,

	.pre_trans = uf,

	.tables    = {&pd_corners_HTM},
	.ntables   = 1,
};

/* DR steps **************************/
Step
drany_HTM = {
	.shortname = "dr",
	.name      = "DR on any axis",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_drud_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.detect    = detect_pretrans_void_3axis,

	.tables    = {&pd_drud_sym16_HTM},
	.ntables   = 1,
};

Step
drud_HTM = {
	.shortname = "drud",
	.name      = "DR on U/D",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_drud_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_drud_sym16_HTM},
	.ntables   = 1,
};

Step
drrl_HTM = {
	.shortname = "drrl",
	.name      = "DR on R/L",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_drud_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.pre_trans = rf,

	.tables    = {&pd_drud_sym16_HTM},
	.ntables   = 1,
};

Step
drfb_HTM = {
	.shortname = "drfb",
	.name      = "DR on F/B",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_drud_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_HTM,

	.pre_trans = fd,

	.tables    = {&pd_drud_sym16_HTM},
	.ntables   = 1,
};

/* DR from EO */
Step
dr_eo = {
	.shortname = "dr-eo",
	.name      = "DR without breaking EO (automatically detected)",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_dr_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_eofb,

	.detect    = detect_pretrans_eofb,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
dr_eofb = {
	.shortname = "dr-eofb",
	.name      = "DR on U/D or R/L without breaking EO on F/B",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_dr_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_eofb,

	.pre_trans = uf,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
dr_eorl = {
	.shortname = "dr-eorl",
	.name      = "DR on U/D or F/B without breaking EO on R/L",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_dr_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_eofb,

	.pre_trans = ur,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
dr_eoud = {
	.shortname = "dr-eoud",
	.name      = "DR on R/L or F/B without breaking EO on U/D",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_dr_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_eofb,

	.pre_trans = fd,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drud_eofb = {
	.shortname = "drud-eofb",
	.name      = "DR on U/D without breaking EO on F/B",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_eofb,

	.pre_trans = uf,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drrl_eofb = {
	.shortname = "drrl-eofb",
	.name      = "DR on R/L without breaking EO on F/B",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_eofb,

	.pre_trans = rf,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drud_eorl = {
	.shortname = "drud-eorl",
	.name      = "DR on U/D without breaking EO on R/L",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_eofb,

	.pre_trans = ur,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drfb_eorl = {
	.shortname = "drfb-eorl",
	.name      = "DR on F/B without breaking EO on R/L",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_eofb,

	.pre_trans = fr,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drfb_eoud = {
	.shortname = "drfb-eoud",
	.name      = "DR on F/B without breaking EO on U/D",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_eofb,

	.pre_trans = fd,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drrl_eoud = {
	.shortname = "drrl-eoud",
	.name      = "DR on R/L without breaking EO on U/D",

	.final     = false,
	.is_done   = check_drud,
	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_eofb,

	.pre_trans = rd,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

/* DR finish steps */
Step
dranyfin_DR = {
	.shortname = "drfin",
	.name      = "DR finish on any axis without breaking DR",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_drudfin_drud,
	.ready     = check_drud,
	.ready_msg = check_drany_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_drud,

	.detect    = detect_pretrans_drud,

	.tables    = {&pd_drudfin_noE_sym16_drud},
	.ntables   = 1,
};

Step
drudfin_drud = {
	.shortname = "drudfin",
	.name      = "DR finish on U/D without breaking DR",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_drudfin_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_drud,

	.pre_trans = uf,

	.tables    = {&pd_drudfin_noE_sym16_drud},
	.ntables   = 1,
};

Step
drrlfin_drrl = {
	.shortname = "drrlfin",
	.name      = "DR finish on R/L without breaking DR",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_drudfin_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_drud,

	.pre_trans = rf,

	.tables    = {&pd_drudfin_noE_sym16_drud},
	.ntables   = 1,
};

Step
drfbfin_drfb = {
	.shortname = "drfbfin",
	.name      = "DR finish on F/B without breaking DR",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_drudfin_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_drud,

	.pre_trans = fd,

	.tables    = {&pd_drudfin_noE_sym16_drud},
	.ntables   = 1,
};

/* HTR from DR */
Step
htr_any = {
	.shortname = "htr",
	.name      = "HTR from DR",

	.final     = false,
	.is_done   = check_htr,
	.estimate  = estimate_htr_drud,
	.ready     = check_drud,
	.ready_msg = check_drany_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_drud,

	.detect    = detect_pretrans_drud,

	.tables    = {&pd_htr_drud},
	.ntables   = 1,
};

Step
htr_drud = {
	.shortname = "htr-drud",
	.name      = "HTR from DR on U/D",

	.final     = false,
	.is_done   = check_htr,
	.estimate  = estimate_htr_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_drud,

	.pre_trans = uf,

	.tables    = {&pd_htr_drud},
	.ntables   = 1,
};

Step
htr_drrl = {
	.shortname = "htr-drrl",
	.name      = "HTR from DR on R/L",

	.final     = false,
	.is_done   = check_htr,
	.estimate  = estimate_htr_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_drud,

	.pre_trans = rf,

	.tables    = {&pd_htr_drud},
	.ntables   = 1,
};

Step
htr_drfb = {
	.shortname = "htr-drfb",
	.name      = "HTR from DR on F/B",

	.final     = false,
	.is_done   = check_htr,
	.estimate  = estimate_htr_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = &moveset_drud,

	.pre_trans = fd,

	.tables    = {&pd_htr_drud},
	.ntables   = 1,
};

/* HTR finish */
Step
htrfin_htr = {
	.shortname = "htrfin",
	.name      = "HTR finish without breaking HTR",

	.final     = true,
	.is_done   = is_solved,
	.estimate  = estimate_htrfin_htr,
	.ready     = check_htr,
	.ready_msg = check_htr_msg,
	.is_valid  = always_valid,
	.moveset   = &moveset_htr,

	.pre_trans = uf,

	.tables    = {&pd_htrfin_htr},
	.ntables   = 1,
};

Step *steps[] = {
	&optimal_HTM, /* first is default */
	&optimal_light_HTM,

	&eofin_eo,
	&eofbfin_eofb,
	&eorlfin_eorl,
	&eoudfin_eoud,

	&eoany_HTM,
	&eofb_HTM,
	&eorl_HTM,
	&eoud_HTM,

	&coany_HTM,
	&coud_HTM,
	&corl_HTM,
	&cofb_HTM,

	&coany_URF,
	&coud_URF,
	&corl_URF,
	&cofb_URF,

	&drany_HTM,
	&drud_HTM,
	&drrl_HTM,
	&drfb_HTM,

	&dr_eo,
	&dr_eofb,
	&dr_eorl,
	&dr_eoud,
	&drud_eofb,
	&drrl_eofb,
	&drud_eorl,
	&drfb_eorl,
	&drfb_eoud,
	&drrl_eoud,

	&dranyfin_DR,
	&drudfin_drud,
	&drrlfin_drrl,
	&drfbfin_drfb,

	&htr_any,
	&htr_drud,
	&htr_drrl,
	&htr_drfb,

	&htrfin_htr,

	&cornershtr_HTM,
	&cornershtr_URF,
	&corners_HTM,
	&corners_URF,

	NULL
};

/* Checkers and validators ***************************************************/

static bool
check_centers(Cube cube)
{
	return cube.cpos == 0;
}

static bool
check_coud_HTM(Cube cube)
{
	return cube.coud == 0;
}

static bool
check_coud_URF(Cube cube)
{
	Cube c2, c3;

	c2 = apply_move(z, cube);
	c3 = apply_move(x, cube);

	return cube.coud == 0 || c2.coud == 0 || c3.coud == 0;
}

static bool
check_corners_URF(Cube cube)
{
	Cube c;
	Trans i;

	for (i = 0; i < NROTATIONS; i++) {
		c = apply_alg(rotation_alg(i), cube);
		if (c.cp && c.coud)
			return true;
	}

	return false;
}

static bool
check_corners_HTM(Cube cube)
{
	return cube.cp == 0 && cube.coud == 0;
}

static bool
check_cornershtr(Cube cube)
{
	return coord_cornershtr.index(cube) == 0;
}

static bool
check_eofb(Cube cube)
{
	return cube.eofb == 0;
}

static bool
check_drud(Cube cube)
{
	return cube.eofb == 0 && cube.eorl == 0 && cube.coud == 0;
}

static bool
check_htr(Cube cube)
{
	return check_drud(cube) && coord_htr_drud.index(cube) == 0;
}

static bool
always_valid(Alg *alg)
{
	return true;
}

static bool
validate_singlecw_ending(Alg *alg)
{
	int i;
	bool nor, inv;
	Move l2 = NULLMOVE, l1 = NULLMOVE, l2i = NULLMOVE, l1i = NULLMOVE;

	for (i = 0; i < alg->len; i++) {
		if (alg->inv[i]) {
			l2i = l1i;
			l1i = alg->move[i];
		} else {
			l2 = l1;
			l1 = alg->move[i];
		}
	}

	nor = l1 ==base_move(l1)  && (!commute(l1, l2) ||l2 ==base_move(l2));
	inv = l1i==base_move(l1i) && (!commute(l1i,l2i)||l2i==base_move(l2i));

	return nor && inv;
}

/* Public functions **********************************************************/

void
compute_ind(StepAlt *a, Cube *cube, uint64_t *ind)
{

}

int
estimate_stepalt(StepAlt *a, uint64_t *ind)
{
	int i, ret, est[a->n_coord];

	for (i = 0; i < a->n_coord; i++) {
		est[i] = ptableval(a->pd[i], ind[i]);
		if (est[i] == 0 && a->compact_pd[i])
			est[i] = ptableval(a->fallback_pd, ind[i] / a->fbmod);
	}

	for (i = 0; i < a->n_dbtrick; i++)
		if (est[a->dbtrick[i][0]] == est[a->dbtrick[i][1]] &&
		    est[a->dbtrick[i][0]] == est[a->dbtrick[i][2]])
			est[a->dbtrick[i][0]] += 1;

	for (i = 0, ret = -1; i < a->n_coord; i++)
		ret = max(ret, est[i]);

	return ret;
}

void
prepare_step(Step *step, SolveOptions *opts)
{
	int i, j;
	PDGenData pdg;
	StepAlt *a;

	init_moveset(step->moveset);
	pdg.moveset = step->moveset;

	for (i = 0; step->alt[i] != NULL; i++) {
		a = step->alt[i];
		for (j = 0; j < a->n_coord; j++) {
			gen_coord(a->coord[j]);

			pdg.coord   = a->coord[j];
			pdg.compact = a->compact[j];
			pdg.pd      = NULL;

			a->pd[j] = genptable(&pdg, opts->nthreads);

			if (a->compact_pd[j]) {
				gen_coord(a->fallback_coord[j]);

				pdg.coord   = a->fallback_coord[j];
				pdg.compact = false;
				pdg.pd      = NULL;

				a->fallback_pd[j] =
					step->genptable(&pdg, opts->nthreads);
			}
		}
	}
}
