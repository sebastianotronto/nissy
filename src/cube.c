#define CUBE_C

#include "cube.h"

static int where_is_piece(int piece, int *arr, int n);

void
compose_centers(Cube *c2, Cube *c1)
{
	apply_permutation(c2->xp, c1->xp, 6);
}

void
compose_corners(Cube *c2, Cube *c1)
{
	apply_permutation(c2->cp, c1->cp, 8);
	apply_permutation(c2->cp, c1->co, 8);
	sum_arrays_mod(c2->co, c1->co, 8, 3);
}

void
compose_edges(Cube *c2, Cube *c1)
{
	apply_permutation(c2->ep, c1->ep, 12);
	apply_permutation(c2->ep, c1->eo, 12);
	sum_arrays_mod(c2->eo, c1->eo, 12, 2);
}

void
compose(Cube *c2, Cube *c1)
{
	compose_centers(c2, c1);
	compose_corners(c2, c1);
	compose_edges(c2, c1);
}

void
copy_cube_centers(Cube *src, Cube *dst)
{
	memcpy(dst->xp, src->xp,  6 * sizeof(int));
}

void
copy_cube_corners(Cube *src, Cube *dst)
{
	memcpy(dst->cp, src->cp,  8 * sizeof(int));
	memcpy(dst->co, src->co,  8 * sizeof(int));
}

void
copy_cube_edges(Cube *src, Cube *dst)
{
	memcpy(dst->ep, src->ep, 12 * sizeof(int));
	memcpy(dst->eo, src->eo, 12 * sizeof(int));
}

void
copy_cube(Cube *src, Cube *dst)
{
	copy_cube_centers(src, dst);
	copy_cube_corners(src, dst);
	copy_cube_edges(src, dst);
}

bool
equal(Cube *c1, Cube *c2)
{
	int i;

	for (i = 0; i < 12; i++)
		if (c1->ep[i] != c2->ep[i] || c1->eo[i] != c2->eo[i])
			return false;

	for (i = 0; i < 8; i++)
		if (c1->cp[i] != c2->cp[i] || c1->co[i] != c2->co[i])
			return false;

	for (i = 0; i < 6; i++)
		if (c1->xp[i] != c2->xp[i])
			return false;

	return true;
}

void
invert_cube_centers(Cube *cube)
{
	int i;
	Cube aux;

	copy_cube_centers(cube, &aux);

	for (i = 0; i < 6; i++)
		cube->xp[aux.xp[i]] = i;
}

void
invert_cube_corners(Cube *cube)
{
	int i;
	Cube aux;

	copy_cube_corners(cube, &aux);

	for (i = 0; i < 8; i++) {
		cube->cp[aux.cp[i]] = i;
		cube->co[aux.cp[i]] = (3 - aux.co[i]) % 3;
	}
}

void
invert_cube_edges(Cube *cube)
{
	int i;
	Cube aux;

	copy_cube_edges(cube, &aux);

	for (i = 0; i < 12; i++) {
		cube->ep[aux.ep[i]] = i;
		cube->eo[aux.ep[i]] = aux.eo[i];
	}
}

void
invert_cube(Cube *cube)
{
	invert_cube_centers(cube);
	invert_cube_corners(cube);
	invert_cube_edges(cube);
}

bool
is_admissible(Cube *c) {
	bool perm;
	int sign, i;
	int sum_e, sum_c;

	perm = is_perm(c->ep, 12) && is_perm(c->cp, 8) && is_perm(c->xp, 6);

	sign = perm_sign(c->ep,12) + perm_sign(c->cp,8) + perm_sign(c->xp,6);

	for (i = 0, sum_e = 0; i < 12; i++)
		if (c->eo[i] > 1)
			return false;
		else
			sum_e += c->eo[i];

	for (i = 0, sum_c = 0; i < 8; i++)
		if (c->co[i] > 2)
			return false;
		else
			sum_c += c->co[i];

	return (perm && sign % 2 == 0 && sum_e % 2 == 0 && sum_c % 2 == 0);
}

bool
is_solved(Cube *cube)
{
	Cube solved_cube;
	make_solved(&solved_cube);

	return equal(cube, &solved_cube);
}

void
make_solved_centers(Cube *cube)
{
	static int sorted[6] = {0, 1, 2, 3, 4, 5};

	memcpy(cube->xp, sorted,  6 * sizeof(int));
}

void
make_solved_corners(Cube *cube)
{
	static int sorted[8] = {0, 1, 2, 3, 4, 5, 6, 7};

	memcpy(cube->cp, sorted,  8 * sizeof(int));
	memset(cube->co, 0,       8 * sizeof(int));
}

void
make_solved_edges(Cube *cube)
{
	static int sorted[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

	memcpy(cube->ep, sorted, 12 * sizeof(int));
	memset(cube->eo, 0,      12 * sizeof(int));
}

void
make_solved(Cube *cube)
{
	make_solved_centers(cube);
	make_solved_corners(cube);
	make_solved_edges(cube);
}

void
print_cube(Cube *cube)
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
		printf(" %s ", edge_string[cube->ep[i]]);
	printf("\n");

	for (int i = 0; i < 12; i++)
		printf("  %" PRIu8 " ", cube->eo[i]);
	printf("\n");

	for (int i = 0; i < 8; i++)
		printf("%s ", corner_string[cube->cp[i]]);
	printf("\n");

	for (int i = 0; i < 8; i++)
		printf("  %" PRIu8 " ", cube->co[i]);
	printf("\n");

	for (int i = 0; i < 6; i++)
		printf("  %s ", center_string[cube->xp[i]]);
	printf("\n");
}

int
where_is_center(Center x, Cube *c)
{
	return where_is_piece(x, c->xp, 6);
}

int
where_is_corner(Corner k, Cube *c)
{
	return where_is_piece(k, c->cp, 8);
}

int
where_is_edge(Edge e, Cube *c)
{
	return where_is_piece(e, c->ep, 12);
}

static int
where_is_piece(int piece, int *arr, int n)
{
	int i;

	for (i = 0; i < n; i++)
		if (arr[i] == piece)
			return i;

	return -1;
}
