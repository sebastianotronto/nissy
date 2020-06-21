/* General rule for piece numbering (visually nicer):
 *
 *   0   1   2   3   4   5   6   7   8   9  10  11
 *  UF  UL  UB  UR  DF  DL  DB  DR  FR  FL  BL  BR
 * UFR UFL UBL UBR DFR DFL DBL DBR
 *                                                   
 * The order of moves is
 *  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 16 17 18
 *  -  U U2 U'  D D2 D'  R R2 R'  L L2 L'  F F2 F'  B B2 B'
 *  (0 is reserved for no move)                              */

#define UF 0
#define UL 1
#define UB 2
#define UR 3
#define DF 4
#define DL 5
#define DB 6
#define DR 7
#define FR 8
#define FL 9
#define BL 10
#define BR 11

#define UFR 0
#define UFL 1
#define UBL 2
#define UBR 3
#define DFR 4
#define DFL 5
#define DBL 6
#define DBR 7

#define U  1
#define U2 2
#define U3 3
#define D  4
#define D2 5
#define D3 6
#define R  7
#define R2 8
#define R3 9
#define L  10
#define L2 11
#define L3 12
#define F  13
#define F2 14
#define F3 15
#define B  16
#define B2 17
#define B3 18

extern char edge_string_list[12][5];
extern char corner_string_list[8][5];
extern char move_string_list[19][5];
extern int inverse_move[19];

/* Convert piece representation from integer to array.
 * Come convertions are not "perfect": for example, and epud type of piece
 * is represented by a permutation index in 8! elements, but it as an array
 * it is converted to the first 8 elements of a 12 elements ep array (with
 * meaningless values for the other 4 elements). */

void ep_int_to_array(int ep, int a[12]);
void epud_int_to_array(int epud, int a[12]);
void epfb_int_to_array(int epfb, int a[12]);
void eprl_int_to_array(int eprl, int a[12]);
void epose_int_to_array(int epos, int a[12]);
void eposs_int_to_array(int epos, int a[12]);
void eposm_int_to_array(int epos, int a[12]);
void epe_int_to_array(int epe, int a[12]);
void epm_int_to_array(int epe, int a[12]);
void eps_int_to_array(int epe, int a[12]);
void emslices_int_to_array(int emslices, int a[12]);
void cp_int_to_array(int cp, int a[8]);
void eo_11bits_to_array(int eo, int a[12]);
void co_7trits_to_array(int co, int a[8]);

int ep_array_to_int(int ep[12]);
int epud_array_to_int(int ep[12]);
int epfb_array_to_int(int ep[12]);
int eprl_array_to_int(int ep[12]);
int epose_array_to_int(int ep[12]);
int eposs_array_to_int(int ep[12]);
int eposm_array_to_int(int ep[12]);
int epe_array_to_int(int epe[12]);
int epm_array_to_int(int epe[12]);
int eps_array_to_int(int epe[12]);
int emslices_array_to_int(int ep[12]);
int cp_array_to_int(int cp[8]);
int eo_array_to_11bits(int a[12]);
int co_array_to_7trits(int a[8]);
