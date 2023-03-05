#ifndef SOLVE_H
#define SOLVE_H

#include "moves.h"

#define MAX_SOLVERS 99

typedef struct dfsarg DfsArg;
typedef struct threader Threader;
typedef struct solver Solver;

/* TODO: add solver and threader in DfsData, remove from dispatch args and similar */

struct dfsarg {
	void *                    cubedata;
	void *                    threaddata;
	SolveOptions *            opts;
	int                       d;
	bool                      niss;
	Alg *                     current_alg;
};

struct threader {
	void    (*append_sol)(Alg *, void *);
	void    (*dispatch)(DfsArg *, AlgList *, Solver *, Threader *);
	int     (*get_nsol)(void *);
/* TODO: threader should have param, like solver? */
};

struct solver {
	Moveset *  moveset;
	bool       (*move_check_stop)(void *, DfsArg *, Threader *);
	Alg *      (*validate_solution)(void *, Alg *);
	bool       (*niss_makes_sense)(void *, void *, Alg *);
/* TODO: move param to somewhere where it makes more sense */
	void *     param;
/* TODO: the following should be part of a generic cube description */
/* TODO: remove alloc? */
/* TODO: revisit apply_move, maybe apply_alg? or both? */
	void *     (*alloc_cubedata)(void *);
	void       (*copy_cubedata)(void *, void *, void *);
	void       (*free_cubedata)(void *, void *);
	void       (*invert_cube)(void *, void *);
	bool       (*is_solved)(void *, void *);
	void       (*apply_move)(void *, void *, Move);
/* TODO: remove dependence on Cube, preparation should be done before */
	void *     (*prepare_cube)(void *, Cube *);
};

void      dfs(DfsArg *, Solver *, Threader *);
/* TODO: remove dependence on Cube, preparation should be done before */
AlgList * solve(Cube *, SolveOptions *, Solver **, Threader *);

#endif
