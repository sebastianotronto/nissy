#include "cube.h"

/* Public functions implementation *******************************************/

int
array_ep_to_epos(int *ep, int *ss)
{
	int epos[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int eps[4];
	int i, j, is;

	for (i = 0, is = 0; i < 12; i++) {
		for (j = 0; j < 4; j++) {
			if (ep[i] == ss[j]) {
				eps[is++] = j;
				epos[i] = 1;
			}
		}
	}

	for (i = 0; i < 4; i++)
		swap(&epos[ss[i]], &epos[i+8]);

	return 24 * subset_to_index(epos, 12, 4) + perm_to_index(eps, 4);
}

Cube
arrays_to_cube(CubeArray *arr, PieceFilter f)
{
	Cube ret = {0};

	static int epe_solved[4] = {FR, FL, BL, BR};
	static int eps_solved[4] = {UL, UR, DL, DR};
	static int epm_solved[4] = {UF, UB, DF, DB};

	if (f.epose)
		ret.epose = array_ep_to_epos(arr->ep, epe_solved);
	if (f.eposs)
		ret.eposs = array_ep_to_epos(arr->ep, eps_solved);
	if (f.eposm)
		ret.eposm = array_ep_to_epos(arr->ep, epm_solved);
	if (f.eofb)
		ret.eofb = digit_array_to_int(arr->eofb, 11, 2);
	if (f.eorl)
		ret.eorl = digit_array_to_int(arr->eorl, 11, 2);
	if (f.eoud)
		ret.eoud = digit_array_to_int(arr->eoud, 11, 2);
	if (f.cp)
		ret.cp = perm_to_index(arr->cp, 8);
	if (f.coud)
		ret.coud = digit_array_to_int(arr->coud, 7, 3);
	if (f.corl)
		ret.corl = digit_array_to_int(arr->corl, 7, 3);
	if (f.cofb)
		ret.cofb = digit_array_to_int(arr->cofb, 7, 3);
	if (f.cpos)
		ret.cpos = perm_to_index(arr->cpos, 6);

	return ret;
}

Cube
compose_filtered(Cube c2, Cube c1, PieceFilter f)
{
	CubeArray *arr = new_cubearray(c2, f);
	Cube ret;

	ret = move_via_arrays(arr, c1, f);
	free_cubearray(arr, f);

	return ret;
}

void
cube_to_arrays(Cube cube, CubeArray *arr, PieceFilter f)
{
	int i;

	static int epe_solved[4] = {FR, FL, BL, BR};
	static int eps_solved[4] = {UL, UR, DL, DR};
	static int epm_solved[4] = {UF, UB, DF, DB};

	if (f.epose || f.eposs || f.eposm)
		for (i = 0; i < 12; i++)
			arr->ep[i] = -1;

	if (f.epose) 
		epos_to_partial_ep(cube.epose, arr->ep, epe_solved); 
	if (f.eposs) 
		epos_to_partial_ep(cube.eposs, arr->ep, eps_solved); 
	if (f.eposm)
		epos_to_partial_ep(cube.eposm, arr->ep, epm_solved); 
	if (f.eofb)
		int_to_sum_zero_array(cube.eofb, 2, 12, arr->eofb);
	if (f.eorl)
		int_to_sum_zero_array(cube.eorl, 2, 12, arr->eorl);
	if (f.eoud)
		int_to_sum_zero_array(cube.eoud, 2, 12, arr->eoud);
	if (f.cp)
		index_to_perm(cube.cp, 8, arr->cp);
	if (f.coud)
		int_to_sum_zero_array(cube.coud, 3, 8, arr->coud);
	if (f.corl)
		int_to_sum_zero_array(cube.corl, 3, 8, arr->corl);
	if (f.cofb)
		int_to_sum_zero_array(cube.cofb, 3, 8, arr->cofb);
	if (f.cpos)
		index_to_perm(cube.cpos, 6, arr->cpos);
}

void
epos_to_partial_ep(int epos, int *ep, int *ss)
{
	int i, is, eposs[12], eps[4];

	index_to_perm(epos % FACTORIAL4, 4, eps);
	index_to_subset(epos / FACTORIAL4, 12, 4, eposs);

	for (i = 0; i < 4; i++)
		swap(&eposs[ss[i]], &eposs[i+8]);

	for (i = 0, is = 0; i < 12; i++)
		if (eposs[i])
			ep[i] = ss[eps[is++]];
}

void
free_cubearray(CubeArray *arr, PieceFilter f)
{
	if (f.epose || f.eposs || f.eposm)
		free(arr->ep);
	if (f.eofb)
		free(arr->eofb);
	if (f.eorl)
		free(arr->eorl);
	if (f.eoud)
		free(arr->eoud);
	if (f.cp)
		free(arr->cp);
	if (f.coud)
		free(arr->coud);
	if (f.corl)
		free(arr->corl);
	if (f.cofb)
		free(arr->cofb);
	if (f.cpos)
		free(arr->cpos);

	free(arr);
}

Cube
move_via_arrays(CubeArray *arr, Cube c, PieceFilter f)
{
	CubeArray *arrc = new_cubearray(c, f);
	Cube ret;

	if (f.epose || f.eposs || f.eposm)
		apply_permutation(arr->ep, arrc->ep, 12);

	if (f.eofb) {
		apply_permutation(arr->ep, arrc->eofb, 12);
		sum_arrays_mod(arr->eofb, arrc->eofb, 12, 2);
	}

	if (f.eorl) {
		apply_permutation(arr->ep, arrc->eorl, 12);
		sum_arrays_mod(arr->eorl, arrc->eorl, 12, 2);
	}

	if (f.eoud) {
		apply_permutation(arr->ep, arrc->eoud, 12);
		sum_arrays_mod(arr->eoud, arrc->eoud, 12, 2);
	}

	if (f.cp)
		apply_permutation(arr->cp, arrc->cp, 8);

	if (f.coud) {
		apply_permutation(arr->cp, arrc->coud, 8);
		sum_arrays_mod(arr->coud, arrc->coud, 8, 3);
	}

	if (f.corl) {
		apply_permutation(arr->cp, arrc->corl, 8);
		sum_arrays_mod(arr->corl, arrc->corl, 8, 3);
	}

	if (f.cofb) {
		apply_permutation(arr->cp, arrc->cofb, 8);
		sum_arrays_mod(arr->cofb, arrc->cofb, 8, 3);
	}

	if (f.cpos)
		apply_permutation(arr->cpos, arrc->cpos, 6);

	ret = arrays_to_cube(arrc, f);
	free_cubearray(arrc, f);

	return ret;
}

CubeArray *
new_cubearray(Cube cube, PieceFilter f)
{
	CubeArray *arr = malloc(sizeof(CubeArray));

	if (f.epose || f.eposs || f.eposm)
		arr->ep   = malloc(12 * sizeof(int));
	if (f.eofb)
		arr->eofb = malloc(12 * sizeof(int));
	if (f.eorl)
		arr->eorl = malloc(12 * sizeof(int));
	if (f.eoud)
		arr->eoud = malloc(12 * sizeof(int));
	if (f.cp)
		arr->cp   = malloc(8  * sizeof(int));
	if (f.coud)
		arr->coud = malloc(8  * sizeof(int));
	if (f.corl)
		arr->corl = malloc(8  * sizeof(int));
	if (f.cofb)
		arr->cofb = malloc(8  * sizeof(int));
	if (f.cpos)
		arr->cpos = malloc(6  * sizeof(int));

	cube_to_arrays(cube, arr, f);

	return arr;
}

Cube
admissible_ep(Cube cube, PieceFilter f)
{
	CubeArray *arr = new_cubearray(cube, f);
	Cube ret;
	bool used[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int i, j;

	for (i = 0; i < 12; i++)
		if (arr->ep[i] != -1)
			used[arr->ep[i]] = true;

	for (i = 0, j = 0; i < 12; i++) {
		for ( ; j < 11 && used[j]; j++);
		if (arr->ep[i] == -1)
			arr->ep[i] = j++;
	}

	ret = arrays_to_cube(arr, pf_ep);
	free_cubearray(arr, f);

	return ret;
}

Cube
compose(Cube c2, Cube c1)
{
	return compose_filtered(c2, c1, pf_all);
}

int
edge_slice(Edge e) {
	if (e < 0 || e > 11)
		return -1;

	if (e == FR || e == FL || e == BL || e == BR)
		return 0;
	if (e == UR || e == UL || e == DR || e == DL)
		return 1;

	return 2;
}

bool
equal(Cube c1, Cube c2)
{
	return c1.eofb  == c2.eofb  &&
	       c1.epose == c2.epose &&
	       c1.eposs == c2.eposs &&
	       c1.eposm == c2.eposm &&
	       c1.coud  == c2.coud  &&
	       c1.cp    == c2.cp    &&
	       c1.cpos  == c2.cpos;
}

Cube
inverse_cube(Cube cube)
{
	CubeArray *arr, *inv;
	Cube ret;
	int i;

	arr = new_cubearray((Cube){0}, pf_all);
	inv = new_cubearray((Cube){0}, pf_all);

	cube_to_arrays(cube, arr, pf_all);

	for (i = 0; i < 12; i++) {
		inv->ep[arr->ep[i]]   = i;
		inv->eofb[arr->ep[i]] = arr->eofb[i];
		inv->eorl[arr->ep[i]] = arr->eorl[i];
		inv->eoud[arr->ep[i]] = arr->eoud[i];
	}

	for (i = 0; i < 8; i++) {
		inv->cp[arr->cp[i]]   = i;
		inv->coud[arr->cp[i]] = (3 - arr->coud[i]) % 3;
		inv->corl[arr->cp[i]] = (3 - arr->corl[i]) % 3;
		inv->cofb[arr->cp[i]] = (3 - arr->cofb[i]) % 3;
	}

	for (int i = 0; i < 6; i++)
		inv->cpos[arr->cpos[i]] = i;

	ret = arrays_to_cube(inv, pf_all);
	free_cubearray(arr, pf_all);
	free_cubearray(inv, pf_all);

	return ret;
}

bool
is_admissible(Cube cube) {

	/* TODO: this should check consistency of different orientations */
	/* check also that centers are opposite and admissible */

	CubeArray *a = new_cubearray(cube, pf_all);
	int parity;
	bool perm;

	perm   = is_perm(a->ep,  12)  &&
	         is_perm(a->cp,   8)  &&
	         is_perm(a->cpos, 6);
	parity = perm_sign(a->ep,  12) +
	         perm_sign(a->cp,   8) +
	         perm_sign(a->cpos, 6);

	return perm && parity % 2 == 0;
}

bool
is_solved(Cube cube)
{
	return equal(cube, (Cube){0});
}

bool
is_block_solved(Cube cube, Block block)
{
	int i;

	for (i = 0; i < 12; i++)
		if (block.edge[i] && !is_solved_edge(cube, i))
			return false;
	for (i = 0; i < 8; i++)
		if (block.corner[i] && !is_solved_corner(cube, i))
			return false;
	for (i = 0; i < 6; i++)
		if (block.center[i] && !is_solved_center(cube, i))
			return false;

	return true;
}

bool
is_solved_center(Cube cube, Center c)
{
	return what_center_at(cube, c) == c;
}

bool
is_solved_corner(Cube cube, Corner c)
{
	return what_corner_at(cube, c) == c &&
	       what_orientation_corner(cube.coud, c);
}

bool
is_solved_edge(Cube cube, Edge e)
{
	return what_edge_at(cube, e) == e &&
	       what_orientation_edge(cube.eofb, e);
}

int
piece_orientation(Cube cube, int piece, char *orientation)
{
	int arr[12], n, b, x;

	if (!strcmp(orientation, "eofb")) {
		x = cube.eofb;
		n = 12;
		b = 2;
	} else if (!strcmp(orientation, "eorl")) {
		x = cube.eorl;
		n = 12;
		b = 2;
	} else if (!strcmp(orientation, "eoud")) {
		x = cube.eoud;
		n = 12;
		b = 2;
	} else if (!strcmp(orientation, "coud")) {
		x = cube.coud;
		n = 8;
		b = 3;
	} else if (!strcmp(orientation, "corl")) {
		x = cube.corl;
		n = 8;
		b = 3;
	} else if (!strcmp(orientation, "cofb")) {
		x = cube.cofb;
		n = 8;
		b = 3;
	} else {
		return -1;
	}

	int_to_sum_zero_array(x, b, n, arr);
	if (piece < n)
		return arr[piece];

	return -1;
}

void
print_cube(Cube cube)
{
	static char edge_string[12][7] = {
		[UF] = "UF", [UL] = "UL", [UB] = "UB", [UR] = "UR",
		[DF] = "DF", [DL] = "DL", [DB] = "DB", [DR] = "DR",
		[FR] = "FR", [FL] = "FL", [BL] = "BL", [BR] = "BR"
	};

	static char corner_string[8][7] = {
		[UFR] = "UFR", [UFL] = "UFL", [UBL] = "UBL", [UBR] = "UBR",
		[DFR] = "DFR", [DFL] = "DFL", [DBL] = "DBL", [DBR] = "DBR"
	};

	static char center_string[6][7] = {
		[U_center] = "U", [D_center] = "D",
		[R_center] = "R", [L_center] = "L", 
		[F_center] = "F", [B_center] = "B"
	};

	for (int i = 0; i < 12; i++)
		printf(" %s ", edge_string[what_edge_at(cube, i)]);
	printf("\n");

	for (int i = 0; i < 12; i++)
		printf("  %d ", what_orientation_edge(cube.eofb, i));
	printf("\n");

	for (int i = 0; i < 8; i++)
		printf("%s ", corner_string[what_corner_at(cube, i)]);
	printf("\n");

	for (int i = 0; i < 8; i++)
		printf("  %d ", what_orientation_corner(cube.coud, i));
	printf("\n");

	for (int i = 0; i < 6; i++)
		printf("  %s ", center_string[what_center_at(cube, i)]);
	printf("\n");
}

Cube
random_cube()
{
	CubeArray *arr = new_cubearray((Cube){0}, pf_4val);
	Cube ret;
	int ep, cp, eo, co;

	ep = rand() % FACTORIAL12;
	cp = rand() % FACTORIAL8;
	eo = rand() % POW2TO11;
	co = rand() % POW3TO7;

	index_to_perm(ep, 12, arr->ep);
	index_to_perm(cp,  8, arr->cp);
	int_to_sum_zero_array(eo, 2, 12, arr->eofb);
	int_to_sum_zero_array(co, 3,  8, arr->coud);

	if (perm_sign(arr->ep, 12) != perm_sign(arr->cp, 8))
		swap(&(arr->ep[0]), &(arr->ep[1]));

	ret = arrays_to_cube(arr, pf_4val);
	free_cubearray(arr, pf_4val);

	return ret;
}

Center
what_center_at(Cube cube, Center c)
{
	static bool initialized = false;
	static Center aux[FACTORIAL6][6];
	static int i;
	static unsigned int ui;
	static CubeArray *arr;

	if (!initialized) {
		for (ui = 0; ui < FACTORIAL6; ui++) {
			arr = new_cubearray((Cube){.cpos = ui}, pf_cpos);
			for (i = 0; i < 6; i++)
				aux[ui][i] = arr->cpos[i];
			free_cubearray(arr, pf_cpos);
		}

		initialized = true;
	}

	return aux[cube.cpos][c];
}

Corner
what_corner_at(Cube cube, Corner c)
{
	static bool initialized = false;
	static Corner aux[FACTORIAL8][8];
	static int i;
	static unsigned int ui;
	static CubeArray *arr;

	if (!initialized) {
		for (ui = 0; ui < FACTORIAL8; ui++) {
			arr = new_cubearray((Cube){.cp = ui}, pf_cp);
			for (i = 0; i < 8; i++)
				aux[ui][i] = arr->cp[i];
			free_cubearray(arr, pf_cp);
		}

		initialized = true;
	}

	return aux[cube.cp][c];
}

Edge
what_edge_at(Cube cube, Edge e)
{
	Edge ret;
	CubeArray *arr = new_cubearray(cube, pf_ep);

	ret = arr->ep[e];

	free_cubearray(arr, pf_ep);
	return ret;
}

int
what_orientation_corner(int co, Corner c)
{
	static bool initialized = false;
	static int auxlast[POW3TO7];
	static int auxarr[8];
	static unsigned int ui;

	if (!initialized) {
		for (ui = 0; ui < POW3TO7; ui++) {
			int_to_sum_zero_array(ui, 3, 8, auxarr);
			auxlast[ui] = auxarr[7];
		}

		initialized = true;
	}

	if (c < 7)
		return (co / powint(3, c)) % 3;
	else
		return auxlast[co];
}

int
what_orientation_edge(int eo, Edge e)
{
	static bool initialized = false;
	static int auxlast[POW2TO11];
	static int auxarr[12];
	static unsigned int ui;

	if (!initialized) {
		for (ui = 0; ui < POW2TO11; ui++) {
			int_to_sum_zero_array(ui, 2, 12, auxarr);
			auxlast[ui] = auxarr[11];
		}

		initialized = true;
	}

	if (e < 11)
		return (eo & (1 << e)) ? 1 : 0;
	else
		return auxlast[eo];
}

Center
where_is_center(Cube cube, Center c)
{
	static bool initialized = false;
	static Center aux[FACTORIAL6][6];
	static int i;
	static unsigned int ui;
	static CubeArray *arr;

	if (!initialized) {
		for (ui = 0; ui < FACTORIAL6; ui++) {
			arr = new_cubearray((Cube){.cpos = ui}, pf_cpos);
			for (i = 0; i < 6; i++)
				aux[ui][arr->cpos[i]] = i;
			free_cubearray(arr, pf_cpos);
		}

		initialized = true;
	}

	return aux[cube.cpos][c];
}

Corner
where_is_corner(Cube cube, Corner c)
{
	static bool initialized = false;
	static Corner aux[FACTORIAL8][8];
	static int i;
	static unsigned int ui;
	static CubeArray *arr;

	if (!initialized) {
		for (ui = 0; ui < FACTORIAL8; ui++) {
			arr = new_cubearray((Cube){.cp = ui}, pf_cp);
			for (i = 0; i < 8; i++)
				aux[ui][arr->cp[i]] = i;
			free_cubearray(arr, pf_cp);
		}

		initialized = true;
	}
	return aux[cube.cp][c];
}

Edge
where_is_edge(Cube cube, Edge e)
{
	/* TODO: when I wrote this code I forgot to add the final
		 part, and now I can't remember how it was supposed to
		 work (i.e. how to recover the location of the edge
		 from these tables. I think it is either very easy or
		 wrong, in any case it is not a priority now.
		 Future Seba can deal with it.

	static bool initialized = false;
	static Edge aux[3][FACTORIAL12/FACTORIAL8][12];
	static int i;
	static unsigned int ui;
	static CubeArray *arr;

	if (!initialized) {
		for (ui = 0; ui < FACTORIAL12/FACTORIAL8; ui++) {
			arr = new_cubearray((Cube){.epose = ui}, pf_e);
			for (i = 0; i < 12; i++)
				if (edge_slice(arr->ep[i]) == 0)
					aux[0][ui][arr->ep[i]] = i;
			free_cubearray(arr, pf_e);

			arr = new_cubearray((Cube){.eposs = ui}, pf_s);
			for (i = 0; i < 12; i++)
				if (edge_slice(arr->ep[i]) == 1)
					aux[1][ui][arr->ep[i]] = i;
			free_cubearray(arr, pf_s);

			arr = new_cubearray((Cube){.eposm = ui}, pf_m);
			for (i = 0; i < 12; i++)
				if (edge_slice(arr->ep[i]) == 2)
					aux[2][ui][arr->ep[i]] = i;
			free_cubearray(arr, pf_m);
		}

		initialized = true;
	}
	*/

	int i;
	CubeArray *arr = new_cubearray(cube, pf_ep);

	for (i = 0; i < 12; i++)
		if ((Edge)arr->ep[i] == e)
			return i;

	return -1;
}
