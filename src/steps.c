#include "steps.h"

#define UPDATECHECKSTOP(a, b, c)     if ((a=(MAX((a),(b))))>(c)) return (a);

/* Checkers, estimators and validators ***************************************/

static bool             check_centers(Cube cube);
static bool             check_eofb(Cube cube);
static bool             check_drud(Cube cube);
static bool             check_htr(Cube cube);

static int              estimate_eoany_HTM(EstimateData *ed);
static int              estimate_eofb_HTM(EstimateData *ed);
static int              estimate_coany_HTM(EstimateData *ed);
static int              estimate_coud_HTM(EstimateData *ed);
static int              estimate_coany_URF(EstimateData *ed);
static int              estimate_coud_URF(EstimateData *ed);
static int              estimate_corners_HTM(EstimateData *ed);
static int              estimate_cornershtr_HTM(EstimateData *ed);
static int              estimate_corners_URF(EstimateData *ed);
static int              estimate_cornershtr_URF(EstimateData *ed);
static int              estimate_drany_HTM(EstimateData *ed);
static int              estimate_drud_HTM(EstimateData *ed);
static int              estimate_drud_eofb(EstimateData *ed);
static int              estimate_dr_eofb(EstimateData *ed);
static int              estimate_drudfin_drud(EstimateData *ed);
static int              estimate_htr_drud(EstimateData *ed);
static int              estimate_htrfin_htr(EstimateData *ed);
static int              estimate_optimal_HTM(EstimateData *ed);

static bool             always_valid(Alg *alg);
static bool             validate_singlecw_ending(Alg *alg);

/* Pre-transformation detectors **********************************************/

static Trans            detect_pretrans_eofb(Cube cube);
static Trans            detect_pretrans_drud(Cube cube);

/* Messages for when cube is not ready ***************************************/

static char check_centers_msg[100] = "cube must be oriented (centers solved)";
static char check_eo_msg[100]      = "EO must be solved on given axis";
static char check_dr_msg[100]      = "DR must be solved on given axis";
static char check_htr_msg[100]     = "HTR must be solved";
static char check_drany_msg[100]   = "DR must be solved on at least one axis";

/* Steps *********************************************************************/

Step
optimal_HTM = {
	.shortname = "optimal",
	.name      = "Optimal solve (in HTM)",

	.estimate  = estimate_optimal_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = always_valid,
	.moveset   = moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_khuge_HTM, &pd_corners_HTM},
	.ntables   = 2,
};

/* EO steps **************************/
Step
eoany_HTM = {
	.shortname = "eo",
	.name      = "EO on any axis",

	.estimate  = estimate_eoany_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_eofb_HTM},
	.ntables   = 1,
};

Step
eofb_HTM = {
	.shortname = "eofb",
	.name      = "EO on F/B",

	.estimate  = estimate_eofb_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_eofb_HTM},
	.ntables   = 1,
};

Step
eorl_HTM = {
	.shortname = "eorl",
	.name      = "EO on R/L",

	.estimate  = estimate_eofb_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = ur,

	.tables    = {&pd_eofb_HTM},
	.ntables   = 1,
};

Step
eoud_HTM = {
	.shortname = "eoud",
	.name      = "EO on U/D",

	.estimate  = estimate_eofb_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = fd,

	.tables    = {&pd_eofb_HTM},
	.ntables   = 1,
};

/* CO steps **************************/
Step
coany_HTM = {
	.shortname = "co",
	.name      = "CO on any axis",

	.estimate  = estimate_coany_HTM,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
coud_HTM = {
	.shortname = "coud",
	.name      = "CO on U/D",

	.estimate  = estimate_coud_HTM,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
corl_HTM = {
	.shortname = "corl",
	.name      = "CO on R/L",

	.estimate  = estimate_coud_HTM,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = rf,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
cofb_HTM = {
	.shortname = "cofb",
	.name      = "CO on F/B",

	.estimate  = estimate_coud_HTM,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = fd,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
coany_URF = {
	.shortname = "co-URF",
	.name      = "CO any axis (URF moveset)",

	.estimate  = estimate_coany_URF,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_URF,

	.pre_trans = uf,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
coud_URF = {
	.shortname = "coud-URF",
	.name      = "CO on U/D (URF moveset)",

	.estimate  = estimate_coud_URF,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_URF,

	.pre_trans = uf,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
corl_URF = {
	.shortname = "corl-URF",
	.name      = "CO on R/L (URF moveset)",

	.estimate  = estimate_coud_URF,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_URF,

	.pre_trans = rf,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

Step
cofb_URF = {
	.shortname = "cofb-URF",
	.name      = "CO on F/B (URF moveset)",

	.estimate  = estimate_coud_URF,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_URF,

	.pre_trans = fd,

	.tables    = {&pd_coud_HTM},
	.ntables   = 1,
};

/* Misc corner steps *****************/
Step
cornershtr_HTM = {
	.shortname = "chtr",
	.name      = "Solve corners to HTR state",

	.estimate  = estimate_cornershtr_HTM,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_cornershtr_HTM},
	.ntables   = 1,
};

Step
cornershtr_URF = {
	.shortname = "chtr-URF",
	.name      = "Solve corners to HTR state (URF moveset)",

	.estimate  = estimate_cornershtr_URF,
	.ready     = NULL,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_URF,

	.pre_trans = uf,

	.tables    = {&pd_cornershtr_HTM},
	.ntables   = 1,
};

Step
corners_HTM = {
	.shortname = "corners",
	.name      = "Solve corners",

	.estimate  = estimate_corners_HTM,
	.ready     = NULL,
	.is_valid  = always_valid,
	.moveset   = moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_corners_HTM},
	.ntables   = 1,
};

Step
corners_URF = {
	.shortname = "corners-URF",
	.name      = "Solve corners (URF moveset)",

	.estimate  = estimate_corners_URF,
	.ready     = NULL,
	.is_valid  = always_valid,
	.moveset   = moveset_URF,

	.pre_trans = uf,

	.tables    = {&pd_corners_HTM},
	.ntables   = 1,
};

/* DR steps **************************/
Step
drany_HTM = {
	.shortname = "dr",
	.name      = "DR on any axis",

	.estimate  = estimate_drany_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_drud_sym16_HTM},
	.ntables   = 1,
};

Step
drud_HTM = {
	.shortname = "drud",
	.name      = "DR on U/D",

	.estimate  = estimate_drud_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = uf,

	.tables    = {&pd_drud_sym16_HTM},
	.ntables   = 1,
};

Step
drrl_HTM = {
	.shortname = "drrl",
	.name      = "DR on R/L",

	.estimate  = estimate_drud_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = rf,

	.tables    = {&pd_drud_sym16_HTM},
	.ntables   = 1,
};

Step
drfb_HTM = {
	.shortname = "drfb",
	.name      = "DR on F/B",

	.estimate  = estimate_drud_HTM,
	.ready     = check_centers,
	.ready_msg = check_centers_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_HTM,

	.pre_trans = fd,

	.tables    = {&pd_drud_sym16_HTM},
	.ntables   = 1,
};

/* DR from EO */
Step
dr_eo = {
	.shortname = "dr-eo",
	.name      = "DR without breaking EO (automatically detected)",

	.estimate  = estimate_dr_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_eofb,

	.detect    = detect_pretrans_eofb,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
dr_eofb = {
	.shortname = "dr-eofb",
	.name      = "DR on U/D or R/L without breaking EO on F/B",

	.estimate  = estimate_dr_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_eofb,

	.pre_trans = uf,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
dr_eorl = {
	.shortname = "dr-eorl",
	.name      = "DR on U/D or F/B without breaking EO on R/L",

	.estimate  = estimate_dr_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_eofb,

	.pre_trans = ur,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
dr_eoud = {
	.shortname = "dr-eoud",
	.name      = "DR on R/L or F/B without breaking EO on U/D",

	.estimate  = estimate_dr_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_eofb,

	.pre_trans = fd,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drud_eofb = {
	.shortname = "drud-eofb",
	.name      = "DR on U/D without breaking EO on F/B",

	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_eofb,

	.pre_trans = uf,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drrl_eofb = {
	.shortname = "drrl-eofb",
	.name      = "DR on R/L without breaking EO on F/B",

	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_eofb,

	.pre_trans = rf,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drud_eorl = {
	.shortname = "drud-eorl",
	.name      = "DR on U/D without breaking EO on R/L",

	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_eofb,

	.pre_trans = ur,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drfb_eorl = {
	.shortname = "drfb-eorl",
	.name      = "DR on F/B without breaking EO on R/L",

	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_eofb,

	.pre_trans = fr,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drfb_eoud = {
	.shortname = "drfb-eoud",
	.name      = "DR on F/B without breaking EO on U/D",

	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_eofb,

	.pre_trans = fd,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

Step
drrl_eoud = {
	.shortname = "drrl-eoud",
	.name      = "DR on R/L without breaking EO on U/D",

	.estimate  = estimate_drud_eofb,
	.ready     = check_eofb,
	.ready_msg = check_eo_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_eofb,

	.pre_trans = rd,

	.tables    = {&pd_drud_eofb},
	.ntables   = 1,
};

/* DR finish steps */
Step
dranyfin_DR = {
	.shortname = "drfin",
	.name      = "DR finish on any axis without breaking DR",

	.estimate  = estimate_drudfin_drud,
	.ready     = check_drud,
	.ready_msg = check_drany_msg,
	.is_valid  = always_valid,
	.moveset   = moveset_drud,

	.detect    = detect_pretrans_drud,

	.tables    = {&pd_drudfin_noE_sym16_drud},
	.ntables   = 1,
};

Step
drudfin_drud = {
	.shortname = "drudfin",
	.name      = "DR finish on U/D without breaking DR",

	.estimate  = estimate_drudfin_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = always_valid,
	.moveset   = moveset_drud,

	.pre_trans = uf,

	.tables    = {&pd_drudfin_noE_sym16_drud},
	.ntables   = 1,
};

Step
drrlfin_drrl = {
	.shortname = "drrlfin",
	.name      = "DR finish on R/L without breaking DR",

	.estimate  = estimate_drudfin_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = always_valid,
	.moveset   = moveset_drud,

	.pre_trans = rf,

	.tables    = {&pd_drudfin_noE_sym16_drud},
	.ntables   = 1,
};

Step
drfbfin_drfb = {
	.shortname = "drfbfin",
	.name      = "DR finish on F/B without breaking DR",

	.estimate  = estimate_drudfin_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = always_valid,
	.moveset   = moveset_drud,

	.pre_trans = fd,

	.tables    = {&pd_drudfin_noE_sym16_drud},
	.ntables   = 1,
};

/* HTR from DR */
Step
htr_any = {
	.shortname = "htr",
	.name      = "HTR from DR",

	.estimate  = estimate_htr_drud,
	.ready     = check_drud,
	.ready_msg = check_drany_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_drud,

	.detect    = detect_pretrans_drud,

	.tables    = {&pd_htr_drud},
	.ntables   = 1,
};

Step
htr_drud = {
	.shortname = "htr-drud",
	.name      = "HTR from DR on U/D",

	.estimate  = estimate_htr_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_drud,

	.pre_trans = uf,

	.tables    = {&pd_htr_drud},
	.ntables   = 1,
};

Step
htr_drrl = {
	.shortname = "htr-drrl",
	.name      = "HTR from DR on R/L",

	.estimate  = estimate_htr_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_drud,

	.pre_trans = rf,

	.tables    = {&pd_htr_drud},
	.ntables   = 1,
};

Step
htr_drfb = {
	.shortname = "htr-drfb",
	.name      = "HTR from DR on F/B",

	.estimate  = estimate_htr_drud,
	.ready     = check_drud,
	.ready_msg = check_dr_msg,
	.is_valid  = validate_singlecw_ending,
	.moveset   = moveset_drud,

	.pre_trans = fd,

	.tables    = {&pd_htr_drud},
	.ntables   = 1,
};

/* HTR finish */
Step
htrfin_htr = {
	.shortname = "htrfin",
	.name      = "HTR finish without breaking HTR",

	.estimate  = estimate_htrfin_htr,
	.ready     = check_htr,
	.ready_msg = check_htr_msg,
	.is_valid  = always_valid,
	.moveset   = moveset_htr,

	.pre_trans = uf,

	.tables    = {&pd_htrfin_htr},
	.ntables   = 1,
};

Step *steps[NSTEPS] = {
	&optimal_HTM, /* first is default */

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
};

/* Checkers, estimators and validators ***************************************/

static bool
check_centers(Cube cube)
{
	return cube.cpos == 0;
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

static int
estimate_eoany_HTM(EstimateData *ed)
{
	int r1, r2, r3;

	r1 = ptableval(&pd_eofb_HTM, ed->cube);
	r2 = ptableval(&pd_eofb_HTM, apply_trans(ur, ed->cube));
	r3 = ptableval(&pd_eofb_HTM, apply_trans(fd, ed->cube));

	return MIN(r1, MIN(r2, r3));
}

static int
estimate_eofb_HTM(EstimateData *ed)
{
	return ptableval(&pd_eofb_HTM, ed->cube);
}

static int
estimate_coany_HTM(EstimateData *ed)
{
	int r1, r2, r3;

	r1 = ptableval(&pd_coud_HTM, ed->cube);
	r2 = ptableval(&pd_coud_HTM, apply_trans(rf, ed->cube));
	r3 = ptableval(&pd_coud_HTM, apply_trans(fd, ed->cube));

	return MIN(r1, MIN(r2, r3));
}

static int
estimate_coud_HTM(EstimateData *ed)
{
	return ptableval(&pd_coud_HTM, ed->cube);
}

static int
estimate_coany_URF(EstimateData *ed)
{
	int r1, r2, r3;
	EstimateData *ed2, *ed3;

	ed2 = malloc(sizeof(EstimateData));
	ed3 = malloc(sizeof(EstimateData));

	ed2->cube = apply_trans(rf, ed->cube);
	ed2->target = ed->target;

	ed3->cube = apply_trans(fd, ed->cube);
	ed3->target = ed->target;

	r1 = estimate_coud_URF(ed);
	r2 = estimate_coud_URF(ed2);
	r3 = estimate_coud_URF(ed3);

	free(ed2);
	free(ed3);

	return MIN(r1, MIN(r2, r3));
}

static int
estimate_coud_URF(EstimateData *ed)
{
	/* TODO: I can improve this by checking first the orientation of
	 * the corner in DBL and use that as a reference */

	EstimateData *ed2, *ed3;

	ed2 = malloc(sizeof(EstimateData));
	ed2->cube = apply_move(z, ed->cube);
	ed2->target = ed->target;

	ed3 = malloc(sizeof(EstimateData));
	ed3->cube = apply_move(x, ed->cube);
	ed3->target = ed->target;

	int ud = estimate_coud_HTM(ed);
	int rl = estimate_coud_HTM(ed2);
	int fb = estimate_coud_HTM(ed3);

	free(ed2);
	free(ed3);

	return MIN(ud, MIN(rl, fb));
}

static int
estimate_corners_HTM(EstimateData *ed)
{
	return ptableval(&pd_corners_HTM, ed->cube);
}

static int
estimate_cornershtr_HTM(EstimateData *ed)
{
	return ptableval(&pd_cornershtr_HTM, ed->cube);
}

static int
estimate_cornershtr_URF(EstimateData *ed)
{
	/* TODO: I can improve this by checking first the corner in DBL
	 * and use that as a reference */

	int c, ret = 15;
	Trans i;

	for (i = 0; i < NROTATIONS; i++) {
		ed->cube = apply_alg(rotation_alg(i), ed->cube);
		c = estimate_cornershtr_HTM(ed);
		ret = MIN(ret, c);
	}

	return ret;
}

static int
estimate_corners_URF(EstimateData *ed)
{
	/* TODO: I can improve this by checking first the corner in DBL
	 * and use that as a reference */

	int c, ret = 15;
	Trans i;

	for (i = 0; i < NROTATIONS; i++) {
		ed->cube = apply_alg(rotation_alg(i), ed->cube);
		c = estimate_corners_HTM(ed);
		ret = MIN(ret, c);
	}

	return ret;
}

static int
estimate_drany_HTM(EstimateData *ed)
{
	int r1, r2, r3;

	r1 = ptableval(&pd_drud_sym16_HTM, ed->cube);
	r2 = ptableval(&pd_drud_sym16_HTM, apply_trans(rf, ed->cube));
	r3 = ptableval(&pd_drud_sym16_HTM, apply_trans(fd, ed->cube));

	return MIN(r1, MIN(r2, r3));
}

static int
estimate_drud_HTM(EstimateData *ed)
{
	return ptableval(&pd_drud_sym16_HTM, ed->cube);
}

static int
estimate_drud_eofb(EstimateData *ed)
{
	return ptableval(&pd_drud_eofb, ed->cube);
}

static int
estimate_dr_eofb(EstimateData *ed)
{
	int r1, r2;

	r1 = ptableval(&pd_drud_eofb, ed->cube);
	r2 = ptableval(&pd_drud_eofb, apply_trans(rf, ed->cube));

	return MIN(r1, r2);
}

static int
estimate_drudfin_drud(EstimateData *ed)
{
	int val = ptableval(&pd_drudfin_noE_sym16_drud, ed->cube);

	if (val != 0)
		return val;

	return ed->cube.epose % 24 == 0 ? 0 : 1;
}

static int
estimate_htr_drud(EstimateData *ed)
{
	return ptableval(&pd_htr_drud, ed->cube);
}

static int
estimate_htrfin_htr(EstimateData *ed)
{
	return ptableval(&pd_htrfin_htr, ed->cube);
}

static int
estimate_optimal_HTM(EstimateData *ed)
{
	int ret = -1;
	Move lbase;
	Cube cubeaux, inv;

	ed->li->corners = ptableval(&pd_corners_HTM, ed->cube);
	UPDATECHECKSTOP(ret, ed->li->corners, ed->target);

	ed->li->normal_ud = ptableval(&pd_khuge_HTM, ed->cube);
	UPDATECHECKSTOP(ret, ed->li->normal_ud, ed->target);

	cubeaux = apply_trans(fd, ed->cube);
	ed->li->normal_fb = ptableval(&pd_khuge_HTM, cubeaux);
	UPDATECHECKSTOP(ret, ed->li->normal_fb, ed->target);

	cubeaux = apply_trans(rf, ed->cube);
	ed->li->normal_rl = ptableval(&pd_khuge_HTM, cubeaux);
	UPDATECHECKSTOP(ret, ed->li->normal_rl, ed->target);

	if (ret == 0)
		return ret;

	if (ed->li->normal_ud == ed->li->normal_fb &&
	    ed->li->normal_fb == ed->li->normal_rl)
		UPDATECHECKSTOP(ret, ed->li->normal_ud + 1, ed->target);

	/* TODO: avoid computation of inverse if unnecessary */
	lbase = base_move(ed->lastmove);
	inv = inverse_cube(ed->cube);

	if ((lbase != U && lbase != D) ||
	    (ed->li->inverse_ud == -1)) {
		ed->li->inverse_ud = ptableval(&pd_khuge_HTM, inv);
	}
	UPDATECHECKSTOP(ret, ed->li->inverse_ud, ed->target);

	if ((lbase != F && lbase != B) ||
	    (ed->li->inverse_fb == -1)) {
		cubeaux = apply_trans(fd, inv);
		ed->li->inverse_fb = ptableval(&pd_khuge_HTM, cubeaux);
	}
	UPDATECHECKSTOP(ret, ed->li->inverse_fb, ed->target);

	if ((lbase != R && lbase != L) ||
	    (ed->li->inverse_rl == -1)) {
		cubeaux = apply_trans(rf, inv);
		ed->li->inverse_rl = ptableval(&pd_khuge_HTM, cubeaux);
	}
	UPDATECHECKSTOP(ret, ed->li->inverse_rl, ed->target);

	if (ed->li->inverse_ud == ed->li->inverse_fb &&
	    ed->li->inverse_fb == ed->li->inverse_rl)
		UPDATECHECKSTOP(ret, ed->li->inverse_ud + 1, ed->target);

	if (ed->li->inverse_ud == ed->target)
		ed->movebitmask |= (1<<U) | (1<<U2) | (1<<U3) |
				   (1<<D) | (1<<D2) | (1<<D3);
	if (ed->li->inverse_fb == ed->target)
		ed->movebitmask |= (1<<F) | (1<<F2) | (1<<F3) |
				   (1<<B) | (1<<B2) | (1<<B3);
	if (ed->li->inverse_rl == ed->target)
		ed->movebitmask |= (1<<R) | (1<<R2) | (1<<R3) |
				   (1<<L) | (1<<L2) | (1<<L3);

	return ret;
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

/* Pre-transformation detectors **********************************************/

static Trans
detect_pretrans_eofb(Cube cube)
{
	Trans i;

	for (i = 0; i < NROTATIONS; i++)
		if (check_eofb(apply_trans(i, cube)))
			return i;

	return 0;
}

static Trans
detect_pretrans_drud(Cube cube)
{
	Trans i;

	for (i = 0; i < NROTATIONS; i++)
		if (check_drud(apply_trans(i, cube)))
			return i;

	return 0;
}

/* Public functions **********************************************************/

void
free_localinfo(LocalInfo *li)
{
	free(li);
}

LocalInfo *
new_localinfo()
{
	LocalInfo *ret = malloc(sizeof(LocalInfo));

	ret->corners    = -1;
	ret->normal_ud  = -1;
	ret->normal_fb  = -1;
	ret->normal_rl  = -1;
	ret->inverse_ud = -1;
	ret->inverse_fb = -1;
	ret->inverse_rl = -1;
	ret->prev_ret   = -1;

	return ret;
}

void
prepare_step(Step *step)
{
	int i;

	for (i = 0; i < step->ntables; i++)
		genptable(step->tables[i]);
}
