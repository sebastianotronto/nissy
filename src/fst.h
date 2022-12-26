#ifndef FST_H
#define FST_H

#include "coord.h"

FstCube     cube_to_fst(Cube *cube);
FstCube     fst_inverse(FstCube fst);
FstCube     fst_move(Move m, FstCube fst);
void        fst_to_cube(FstCube fst, Cube *cube);
int         fst_where_is_edge(int e, FstCube fst);
void        init_fst();

#endif

