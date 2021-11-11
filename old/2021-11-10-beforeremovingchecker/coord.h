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

bool        check_centers(Cube cube);
bool        check_corners(Cube cube);
bool        check_cp(Cube cube);
bool        check_cphtr(Cube cube);
bool        check_cornershtr(Cube cube);
bool        check_coud(Cube cube);
bool        check_drud(Cube cube);
bool        check_htr(Cube cube);
bool        check_drudfin_noE(Cube cube);
bool        check_eofb(Cube cube);
bool        check_eofbepos(Cube cube);
bool        check_epose(Cube cube);
bool        check_ep(Cube cube);
bool        check_epud(Cube cube);
bool        check_khuge(Cube cube);
bool        check_nothing(Cube cube);

void        init_coord();

#endif

