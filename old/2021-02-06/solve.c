#include "solve.h"

/* Data for creating a pruning table:
   - compressed: if set to true, each entry occupies only 4 bits, but values
     larger than 15 cannot be stored.
   - available[] is the list of availabel moves, as above.
   - *ptable is the actual table to fill.
   - n is the number of states (size of ptable).
   - index must "linearize" the cube, i.e. return its index in ptable.
   - fname is the name of the file where to store the table */
typedef struct {
  bool compressed, *available;
  int max_moves;
  uint8_t *ptable;
  uint64_t n;
  uint64_t (*index)(Cube);
  char *fname;
} PruneData;

/* TODO: comment this */
typedef struct {
  bool niss;
  int m, d;
  uint64_t *n;
  Move last1, last2;
} DfsData;

void solve_dfs(Cube cube, SolveData *sd, DfsData dd);
void init_ptable(PruneData *pd, bool read, bool write);

/* Search solutions of lenght exactly d */
void solve_dfs(Cube cube, SolveData *sd, DfsData dd) {
  if (*dd.n >= sd->max_solutions ||
     ((!sd->can_niss || dd.niss) && dd.m + sd->f(cube) > dd.d))
    return;

  (sd->solutions[*dd.n][dd.m]).inverse = dd.niss;
  (sd->solutions[*dd.n][dd.m]).m = NULLMOVE;

  if (!sd->f(cube)) { /* Solved */
    if (dd.m == dd.d) {
      (*dd.n)++;
      if (*dd.n < sd->max_solutions)
        copy_alg(sd->solutions[*dd.n-1], sd->solutions[*dd.n]);
    }
    return;
  }

  for (int i = 0; i < NMOVES && sd->sorted_moves[i] != NULLMOVE; i++) {
    Move move = sd->sorted_moves[i];
    if (possible_next[dd.last2][dd.last1][move]) {
      sd->solutions[*dd.n][dd.m].inverse = dd.niss;
      sd->solutions[*dd.n][dd.m].m = move;
      DfsData nn = { .niss = dd.niss, .m = dd.m+1, .d = dd.d, .n = dd.n,
                     .last1 = move, .last2 = dd.last1 };
      solve_dfs(move_cube(move, cube), sd, nn);
    }
  }
    
  if (sd->can_niss && !dd.niss &&
     (!dd.m || (dd.m && sd->f(move_cube(dd.last1, blank_cube()))))) {
    DfsData nn = { .niss = true, .m = dd.m, .d = dd.d, .n = dd.n };
    solve_dfs(inverse_cube(cube), sd, nn);
  }
}

/* Iterative deepening depth-first search: for i running from the minimum
   to the maximum number of moves allowed, looks for solutions of length i. */
int solve(Cube cube, SolveData *sd) {
  if (sd->precondition != NULL && !sd->precondition(cube))
    return -1;

  /* If not given, generate sorted list of moves */
  if (sd->sorted_moves[0] == NULLMOVE) {
    int a[NMOVES], b[NMOVES], ia = 0, ib = 0;
    for (int i = 0; i < NMOVES; i++) {
      if (sd->available[i]) {
        if (sd->f(move_cube(i, blank_cube())))
          a[ia++] = i;
        else
          b[ib++] = i;
      }
    }
    intarrcopy(a, (int *)sd->sorted_moves, ia);
    intarrcopy(b, (int *)sd->sorted_moves+ia, ib);
    sd->sorted_moves[ia+ib] = NULLMOVE;
  }

  sd->max_solutions = min(sd->max_solutions, MAXS);
  Cube rotated = apply_alg(sd->pre_rotation, blank_cube());
  cube = apply_alg(inverse_cube(rotated), compose(cube, rotated));

  uint64_t ret = 0;
  for (int i=sd->min_moves; i<=sd->max_moves&&!(ret&&sd->optimal_only); i++) {
    DfsData dd = { .d = i, .n = &ret };
    solve_dfs(cube, sd, dd);
  }

  for (uint64_t i = 0; i < ret; i++) {
    /* TODO: transform solutions with inverse of pre_rotation */
    if (sd->cleanup)
      cleanup(sd->solutions[i], sd->max_moves*3);
  }
  
  return ret;
}

void prune_dfs(Cube cube, PruneData *pd, DfsData dd) {
  uint64_t ind = pd->index(cube);
  if ((!ind || pd->ptable[ind]) && pd->ptable[ind] != dd.m)
    return;
  if (dd.m == dd.d) {
    if (ind && !pd->ptable[ind]) {
      pd->ptable[ind] = dd.m;
      (*dd.n)++;
    }
    return;
  }

  for (int i = 0; i < NMOVES; i++) {
    if (dd.m<20)
    if (possible_next[dd.last2][dd.last1][i] && pd->available[i]) {
      DfsData nn = { .m = dd.m+1, .d = dd.d, .n = dd.n,
                     .last1 = i, .last2 = dd.last1 };
      prune_dfs(move_cube(i, cube), pd, nn);
    }
  }
}

void init_ptable(PruneData *pd, bool read, bool write) {
  if (read) {
    FILE *ptf;
    if ((ptf = fopen(pd->fname, "rb")) != NULL) {
      fread(pd->ptable, sizeof(uint8_t), pd->n, ptf);
      fclose(ptf);
      return;
    }
  }

  /* TODO: for now it behaves always as if copressed = false */
  for (uint64_t i = 0; i < pd->n; i++)
    pd->ptable[i] = 0;

  uint64_t s = 1;
  for (int i = 1; i < pd->max_moves && s < pd->n; i++) {
    DfsData dd = { .d = i, .n = &s };
    prune_dfs(blank_cube(), pd, dd);
  }

  if (write) {
    FILE *ptf;
    if ((ptf = fopen(pd->fname, "wb")) != NULL) {
      fwrite(pd->ptable, sizeof(uint8_t), pd->n, ptf);
      fclose(ptf);
      return;
    }
  }
}

/* Solving steps (and indexing functions) */

uint64_t index_eofb(Cube cube) { return cube.eofb; }
uint16_t f_eofb(Cube cube) {
  static bool initialized_ptable;
  static uint8_t pt_eofb[pow2to11];
  if (!initialized_ptable) {
    PruneData pd = {
      .compressed = false, .available = standard_moveset, .max_moves = 13,
      .ptable = pt_eofb, .n = pow2to11, .index = index_eofb,
      .fname = "ptable_eofb"
    };
    init_ptable(&pd, false, true);
    initialized_ptable = true;
  }
  return cube.eofb ? pt_eofb[cube.eofb] : 0;
}

