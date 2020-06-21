#include <stdint.h>
#include "utils.h"

extern int eofb_pruning_table[pow2to11];
extern int eorl_pruning_table[pow2to11];
extern int eoud_pruning_table[pow2to11];
extern int coud_pruning_table[pow3to7];
extern int cofb_pruning_table[pow3to7];
extern int corl_pruning_table[pow3to7];
extern int cp_pruning_table[factorial8];

extern int eorl_from_eofb_pruning_table[pow2to11];
extern int eoud_from_eofb_pruning_table[pow2to11];
extern int eoud_from_eorl_pruning_table[pow2to11];
extern int eofb_from_eorl_pruning_table[pow2to11];
extern int eofb_from_eoud_pruning_table[pow2to11];
extern int eorl_from_eoud_pruning_table[pow2to11];

extern int coud_from_eofb_pruning_table[pow3to7];
extern int coud_from_eorl_pruning_table[pow3to7];
extern int cofb_from_eorl_pruning_table[pow3to7];
extern int cofb_from_eoud_pruning_table[pow3to7];
extern int corl_from_eoud_pruning_table[pow3to7];
extern int corl_from_eofb_pruning_table[pow3to7];

extern int cp_drud_pruning_table[factorial8];
extern int cp_drfb_pruning_table[factorial8];
extern int cp_drrl_pruning_table[factorial8];
extern int epud_pruning_table[factorial8];
extern int epfb_pruning_table[factorial8];
extern int eprl_pruning_table[factorial8];

extern int cp_htr_pruning_table[factorial8];
extern int cpud_to_htr_pruning_table[factorial8];
extern int cpfb_to_htr_pruning_table[factorial8];
extern int cprl_to_htr_pruning_table[factorial8];

/* About 1Mb each */
extern int8_t eofb_epose_pruning_table[pow2to11][binom12on4];
extern int8_t eorl_eposs_pruning_table[pow2to11][binom12on4];
extern int8_t eoud_eposm_pruning_table[pow2to11][binom12on4];

/* About 4.5Mb each */
extern int8_t eofb_coud_pruning_table[pow2to11][pow3to7];
extern int8_t eofb_corl_pruning_table[pow2to11][pow3to7];
extern int8_t eorl_coud_pruning_table[pow2to11][pow3to7];
extern int8_t eorl_cofb_pruning_table[pow2to11][pow3to7];
extern int8_t eoud_cofb_pruning_table[pow2to11][pow3to7];
extern int8_t eoud_corl_pruning_table[pow2to11][pow3to7];

/* About 1Mb each */
extern int8_t coud_epose_from_eofb_pruning_table[pow3to7][binom12on4];
extern int8_t cofb_eposs_from_eorl_pruning_table[pow3to7][binom12on4];
extern int8_t corl_eposm_from_eoud_pruning_table[pow3to7][binom12on4];
extern int8_t coud_epose_from_eorl_pruning_table[pow3to7][binom12on4];
extern int8_t cofb_eposs_from_eoud_pruning_table[pow3to7][binom12on4];
extern int8_t corl_eposm_from_eofb_pruning_table[pow3to7][binom12on4];

/* First one 88Mb, second one 21Mb */
extern int8_t cp_co_pruning_table[factorial8][pow3to7];
extern int8_t triple_eo_pruning_table[pow2to11][binom12on4*binom8on4];

void init_small_pruning_tables();
void init_directdr_pruning_tables();
void init_drfromeo_pruning_tables();
void init_huge_pruning_tables(); 
