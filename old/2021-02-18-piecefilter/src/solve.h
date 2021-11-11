#ifndef SOLVE_H
#define SOLVE_H

#include <stdlib.h>
#include "cube.h"
#include "moves.h"

/* Maximum number of moves per solution and of solutions */
#define MAXM 30
#define MAXS 999

/* Data for solving a step:
   - can_niss is true niss can be used, false otherwise.
   - optimal_only if true, dynamically updates max_moves so non-optimal
     solutions are discarded.
   - cleanup determines whether the cleaunup() function should be used on
     the found solutions before returning.
   - available[m] is true if the move m can be used, false otherwise.
   - min_moves and max_moves are the minimum and maximum number of moves that
     can be used.
   - max_solution is the maximum number of solutions that can be returned.
   - precondition can be used to check wheter the step can actually be applied
     to the cube. If it returns false, solve() stops immediately returning -1.
   - f must return 0 if and only if the step is solve, otherwise it must return
     a lower bound for the number of moves required (without niss).
   - sorted_moves[] can be used to specify in which order moves are tried
     by the solving algorithm (for example if one wants to always try F' before
     F). If sorted_moves[0] == NULLMOVE, the list is generated automatically.
     It is advised to list first all the moves that actually influence the
     solved state of the step (this is the default choice). This is in order to
     avoid cases like B2 F for EO and to NISS only when it makes sense.
   - start_moves [Currently unused, REMOVE]
     are the moves that will be used as first moves of all
     solutions. For example giving R' U' F (F' U R) will generate FMC scrambles
     and y (y) will solve the step on another axis.
   - pre_rotation are the rotations to apply before the scamble to solve
     the step wrt a different orientation
   - pre_rotation are the rotations to apply before the scamble to solve
     the step wth respect to a different orientation.
   - solutions[][] is the array where to store the found solutions. */
typedef struct {
  bool can_niss, optimal_only, cleanup, *available;
  int min_moves, max_moves;
  uint64_t max_solutions;
  bool (*precondition)(Cube);
  uint16_t (*f)(Cube);
  Move sorted_moves[NMOVES];
  NissMove pre_rotation[3], solutions[MAXS][MAXM];
} SolveData;

int solve(Cube cube, SolveData *data); /* Returns the number of solutions. */

/* Steps */
uint16_t f_eofb(Cube cube);

#endif
