#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Constants ****************************************************************/

#define NMOVES      (z3+1)
#define NTRANS      (mirror+1)
#define NROTATIONS  (NTRANS-1)

/* Typedefs *****************************************************************/

typedef enum center         Center;
typedef enum corner         Corner;
typedef enum edge           Edge;
typedef enum move           Move;
typedef enum trans          Trans;

typedef struct nissmove *   Alg;
typedef struct alglist      AlgList;
typedef struct alglistnode  AlgListNode;
typedef struct block        Block;
typedef struct cube         Cube;
typedef struct nissmove     NissMove;
typedef struct solveoptions SolveOptions;
typedef struct step         Step;

/* Type specifications *******************************************************/

enum
center
{
	U_center, D_center,
	R_center, L_center,
	F_center, B_center
};

enum
corner
{
	UFR, UFL, UBL, UBR,
	DFR, DFL, DBL, DBR
};

enum
edge
{
	UF, UL, UB, UR,
	DF, DL, DB, DR,
	FR, FL, BL, BR
};

enum
move
{
	NULLMOVE,
	U, U2, U3, D, D2, D3,
	R, R2, R3, L, L2, L3,
	F, F2, F3, B, B2, B3,
	Uw, Uw2, Uw3, Dw, Dw2, Dw3,
	Rw, Rw2, Rw3, Lw, Lw2, Lw3,
	Fw, Fw2, Fw3, Bw, Bw2, Bw3,
	M, M2, M3,
	S, S2, S3,
	E, E2, E3,
	x, x2, x3,
	y, y2, y3,
	z, z2, z3,
};

enum
trans
{
	uf, ur, ub, ul,
	df, dr, db, dl,
	rf, rd, rb, ru,
	lf, ld, lb, lu,
	fu, fr, fd, fl,
	bu, br, bd, bl,
	mirror, /* R|L */
};

struct
alglist
{
	AlgListNode *first;
	AlgListNode *last;
	int          len;
};

struct
alglistnode
{
	Alg          alg;
	AlgListNode *next;
};

struct
block
{
	bool edge[12];
	bool corner[8];
	bool center[6];
};

struct
cube
{
	uint16_t epose;
	uint16_t eposs;
	uint16_t eposm;
	uint16_t eofb;
	uint16_t eorl;
	uint16_t eoud;
	uint16_t cp;
	uint16_t coud;
	uint16_t cofb;
	uint16_t corl;
	uint16_t cpos;
};

struct
nissmove
{
	Move m;
	bool inverse;
};

struct
solveoptions
{
	int     min_moves;
	int     max_moves;
	int     max_solutions;
	bool    optimal_only;
	bool    can_niss;
	bool   *moveset;
	Move   *sorted_moves;
	Trans   pre_trans;
};

struct
step
{
	int  (*f)(Cube);
	bool (*ready)(Cube);
};

/* Public functions **********************************************************/

Cube      apply_alg(Alg alg, Cube cube);
Cube      apply_move(Move m, Cube cube);
Cube      apply_trans(Trans t, Cube cube);
bool      block_solved(Cube cube, Block);
Center    center_at(Cube cube, Center c);
Cube      compose(Cube c2, Cube c1); /* Use c2 as an alg on c1 */
Corner    corner_at(Cube cube, Corner c);
Edge      edge_at(Cube cube, Edge e);
bool      equal(Cube c1, Cube c2);
Cube      inverse_cube(Cube cube);
Move      inverse_move(Move m);
Trans     inverse_trans(Trans t);
bool      is_solved(Cube cube, bool reorient);
int       piece_orientation(Cube cube, int piece, char *orientation);
void      print_cube(Cube cube);
AlgList * solve(Cube cube, Step step, SolveOptions opts);

void      concat(Alg src1, Alg src2, Alg dest);
void      copy_alg(Alg src, Alg dest);
void      invert_alg(Alg src, Alg dest);
int       len(Alg alg);
Alg       new_alg(char *str);
void      print_alg(Alg alg);
void      remove_last_moves(Alg alg, int k);
void      transform_alg(Trans t, Alg alg);

void      init();
