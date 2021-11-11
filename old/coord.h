#ifndef COORD_H
#define COORD_H

#include "trans.h"

extern Coordinate       coord_eofb;
extern Coordinate       coord_eofbepos;
extern Coordinate       coord_coud;
extern Coordinate       coord_cp;
extern Coordinate       coord_corners;
extern Coordinate       coord_cornershtr;
extern Coordinate       coord_drud;
extern Coordinate       coord_coud_sym16;
extern Coordinate       coord_drud_eofb;
extern Coordinate       coord_cp_sym16;
extern Coordinate       coord_eofbepos_sym16;
extern Coordinate       coord_drud_sym16;
extern Coordinate       coord_drudfin_noE_sym16;
extern Coordinate       coord_htrfin;
extern Coordinate       coord_khuge;

uint64_t    cphtr(Cube cube); /* TODO: rename (something with cosets) */
Cube        anti_cphtr(uint64_t ind); /*TODO also this */
uint64_t    epos_dependent(Cube cube); /* TODO: rename and turn into an indexer */

bool        check_centers(Cube cube);
bool        check_corners(Cube cube);
bool        check_cp(Cube cube);
bool        check_cornershtr(Cube cube);
bool        check_coud(Cube cube);
bool        check_drud(Cube cube);
bool        check_htr(Cube cube);
bool        check_htrfin(Cube cube);
bool        check_drudfin_noE(Cube cube);
bool        check_eofb(Cube cube);
bool        check_eofbepos(Cube cube);
bool        check_epose(Cube cube);
bool        check_ep(Cube cube);
bool        check_khuge(Cube cube);
bool        check_nothing(Cube cube);

void        init_coord();

#endif

