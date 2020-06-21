#include "utils.h"

/* Bitmask that define certain movesets. */
#define move_mask_all  524287   /* Reverse 1111111111111111111 */
#define move_mask_eofb 155647   /* Reverse 1111111111111010010 */
#define move_mask_eorl 518527   /* Reverse 1111111010101111111 */
#define move_mask_eoud 524197   /* Reverse 1010010111111111111 */
#define move_mask_drud 149887   /* Reverse 1111111010010010010 */
#define move_mask_drfb 518437   /* Reverse 1010010010010111111 */
#define move_mask_drrl 155557   /* Reverse 1010010111111010010 */
#define move_mask_htr  149797   /* Reverse 1010010010010010010 */

extern int possible_next[19][19];

int parallel(int m1, int m2);
void init_possible_next();

/* Transition tables */
extern int eofb_transition_table[pow2to11][19];
extern int eorl_transition_table[pow2to11][19];
extern int eoud_transition_table[pow2to11][19];
extern int coud_transition_table[pow3to7][19];
extern int cofb_transition_table[pow3to7][19];
extern int corl_transition_table[pow3to7][19];
extern int epud_transition_table[factorial8][19];
extern int epfb_transition_table[factorial8][19];
extern int eprl_transition_table[factorial8][19];
extern int epose_transition_table[binom12on4][19];
extern int eposs_transition_table[binom12on4][19];
extern int eposm_transition_table[binom12on4][19];
extern int epe_transition_table[factorial4][19];
extern int eps_transition_table[factorial4][19];
extern int epm_transition_table[factorial4][19];
extern int emslices_transition_table[binom12on4*binom8on4][19];
extern int cp_transition_table[factorial8][19];


/* Functions for permuting pieces (given in array format) */

void apply_move_ep_array(int move, int ep[12]);
void apply_move_cp_array(int move, int cp[8]);

/* Functions for permuting pieces (given in integer format) */

int apply_move_ep_int(int move, int ep);
int apply_move_epud_int(int move, int ep);
int apply_move_epfb_int(int move, int ep);
int apply_move_eprl_int(int move, int ep);
int apply_move_epose_int(int move, int ep);
int apply_move_eposs_int(int move, int ep);
int apply_move_eposm_int(int move, int ep);
int apply_move_epe_int(int move, int ep);
int apply_move_eps_int(int move, int ep);
int apply_move_epm_int(int move, int ep);
int apply_move_cp_int(int move, int cp);
int apply_move_eofb_int(int move, int eo);
int apply_move_eorl_int(int move, int eo);
int apply_move_eoud_int(int move, int eo);
int apply_move_coud_int(int move, int co);
int apply_move_cofb_int(int move, int co);
int apply_move_corl_int(int move, int co);

/* Initialize transition tables */

void init_epud_transition_table();
void init_epfb_transition_table();
void init_eprl_transition_table();
void init_epose_transition_table();
void init_eposs_transition_table();
void init_eposm_transition_table();
void init_epe_transition_table();
void init_eps_transition_table();
void init_epm_transition_table();
void init_cp_transition_table();
void init_eofb_transition_table();
void init_eorl_transition_table();
void init_eoud_transition_table();
void init_coud_transition_table();
void init_cofb_transition_table();
void init_corl_transition_table();

void init_transition_table();

