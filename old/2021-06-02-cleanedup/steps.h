/* Pre-conditions, i.e. "ready(cube)" functions ******************************/

bool always_ready(Cube cube) { return true; }

/* Main functions, i.e. "f(cube)" ********************************************/

int f_eofb(Cube cube) { return (cube.eofb != 0) ? 1 : 0; }

/* Steps *********************************************************************/

Step step_eofb = { .f = f_eofb, .ready = always_ready };

/* Movesets ******************************************************************/

bool standard_moveset[NMOVES] = {
	[U] = true, [U2] = true, [U3] = true,
	[D] = true, [D2] = true, [D3] = true,
	[R] = true, [R2] = true, [R3] = true,
	[L] = true, [L2] = true, [L3] = true,
	[F] = true, [F2] = true, [F3] = true,
	[B] = true, [B2] = true, [B3] = true,
};
