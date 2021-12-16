#ifndef COORD_H
#define COORD_H

#include "trans.h"

extern Coordinate       coord_eofb;
extern Coordinate       coord_eofbepos;
extern Coordinate       coord_coud;
extern Coordinate       coord_cp;
extern Coordinate       coord_cphtr;
extern Coordinate       coord_corners;
extern Coordinate       coord_cornershtr;
extern Coordinate       coord_cornershtrfin;
extern Coordinate       coord_epud;
extern Coordinate       coord_drud;
extern Coordinate       coord_drud_eofb;
extern Coordinate       coord_htr_drud;
extern Coordinate       coord_htrfin;
extern Coordinate       coord_cpud_separate;

void        test_coord(Coordinate *coord);
void        init_coord();

#endif

