#include "threader_single.h"

static void             append_sol(Alg *, void *);
static void             dispatch(DfsArg *, AlgList *, Solver *, Threader *);
static int              get_nsol(void *);

Threader threader_single = {
	.append_sol      = append_sol,
	.dispatch        = dispatch,
	.get_nsol        = get_nsol,
};

static void
append_sol(Alg *alg, void *threaddata)
{
	append_alg((AlgList *)threaddata, alg);
}

static void
dispatch(DfsArg *arg, AlgList *sols, Solver *solver, Threader *threader)
{
	arg->threaddata  = sols;
	arg->niss        = false;
	arg->current_alg = new_alg("");

	dfs(arg, solver, threader);

	free_alg(arg->current_alg);
}

static int
get_nsol(void *threaddata)
{
	return ((AlgList *)threaddata)->len;
}
