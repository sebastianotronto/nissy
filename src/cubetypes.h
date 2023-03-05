#ifndef CUBETYPES_H
#define CUBETYPES_H

#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>

#define NMOVES               55 /* Actually 54, but one is NULLMOVE */
#define NTRANS               48
#define NROTATIONS           24
#define entry_group_t        uint8_t /* For pruning tables */

#define MAX_N_COORD          6

/* Enums *********************************************************************/

typedef enum
center
{
	U_center, D_center,
	R_center, L_center,
	F_center, B_center
} Center;

typedef enum
corner
{
	UFR, UFL, UBL, UBR,
	DFR, DFL, DBL, DBR
} Corner;

typedef enum
coordtype
{
	COMP_COORD, SYM_COORD, SYMCOMP_COORD
} CoordType;

typedef enum
edge
{
	UF, UL, UB, UR,
	DF, DL, DB, DR,
	FR, FL, BL, BR
} Edge;

typedef enum
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
} Move;

typedef enum
trans
{
	uf, ur, ub, ul,
	df, dr, db, dl,
	rf, rd, rb, ru,
	lf, ld, lb, lu,
	fu, fr, fd, fl,
	bu, br, bd, bl,
	uf_mirror, ur_mirror, ub_mirror, ul_mirror,
	df_mirror, dr_mirror, db_mirror, dl_mirror,
	rf_mirror, rd_mirror, rb_mirror, ru_mirror,
	lf_mirror, ld_mirror, lb_mirror, lu_mirror,
	fu_mirror, fr_mirror, fd_mirror, fl_mirror,
	bu_mirror, br_mirror, bd_mirror, bl_mirror,
} Trans;


/* Typedefs ******************************************************************/

typedef struct alg                Alg;
typedef struct alglist            AlgList;
typedef struct alglistnode        AlgListNode;
typedef struct choicestep         ChoiceStep;
typedef struct command            Command;
typedef struct commandargs        CommandArgs;
typedef struct coordinate         Coordinate;
typedef struct cube               Cube;
/*typedef struct dfsarg             DfsArg;*/
typedef struct fstcube            FstCube;
typedef struct indexer            Indexer;
typedef struct movable            Movable;
typedef struct moveset            Moveset;
typedef struct prunedata          PruneData;
typedef struct solveoptions       SolveOptions;
typedef struct step               Step;
typedef struct symdata            SymData;
typedef struct threaddatasolve    ThreadDataSolve;
typedef struct threaddatagenpt    ThreadDataGenpt;
typedef struct transgroup         TransGroup;

typedef bool                 (*Checker)          (Cube *);
typedef bool                 (*CubeTester)       (Cube *, Alg *);
/*typedef bool                 (*DfsMover)         (DfsArg *);*/
typedef void                 (*DfsExtraCopier)   (void *, void *);
typedef Alg *                (*Validator)        (Alg *);
typedef void                 (*Exec)             (CommandArgs *);
typedef CommandArgs *        (*ArgParser)        (int, char **);
typedef bool                 (*Tester)           (void);
typedef int                  (*TransFinder)      (uint64_t, Trans *);


/* Structs *******************************************************************/

struct
alg
{
	Move *                    move;
	bool *                    inv;
	int                       len;
	int                       allocated;
	Move *                    move_normal;
	int                       len_normal;
	Move *                    move_inverse;
	int                       len_inverse;
};

struct
alglist
{
	AlgListNode *             first;
	AlgListNode *             last;
	int                       len;
};

struct
alglistnode
{
	Alg *                     alg;
	AlgListNode *             next;
};

struct
choicestep
{
	char *                    shortname;
	char *                    name;
	Step *                    step[99];
	Trans                     t[99];
	char *                    ready_msg;
};

struct
command
{
	char *                    name;
	char *                    usage;
	char *                    description;
	ArgParser                 parse_args;
	Exec                      exec;
};

struct
commandargs
{
	bool                      success;
	Alg *                     scramble;
	SolveOptions *            opts;
	ChoiceStep *              cs;
	Command *                 command; /* For help */
	int                       n;
	char                      scrtype[20];
	bool                      scrstdin;
	bool                      header;
};

struct
coordinate
{
	char *                    name;
	CoordType                 type;
	bool                      generated;
	Indexer *                 i[99];
	uint64_t                  max;
	uint64_t *                mtable[NMOVES];
	uint64_t *                ttable[NTRANS];
	TransGroup *              tgrp;
	Coordinate *              base[2];
	uint64_t *                symclass;
	uint64_t *                symrep;
	Trans *                   transtorep;
	Trans *                   ttrep_move[NMOVES];
	uint64_t *                selfsim;
};

struct
cube
{
	int                       ep[12];
	int                       eo[12];
	int                       cp[8];
	int                       co[8];
	int                       xp[6];
};

/*
struct
movable
{
	uint64_t                  val;
	Trans                     t;
};
*/

/*
struct
dfsarg
{
	Cube *                    cube;
	Movable                   ind[MAX_N_COORD];
	Trans                     t;
	Step *                    s;
	SolveOptions *            opts;
	int                       d;
	int                       bound;
	bool                      niss;
	AlgList *                 sols;
	pthread_mutex_t *         sols_mutex;
	Alg *                     current_alg;
	void *                    extra;
};
*/

/*
struct
dfsarg
{
	void *                    cube_data;
	SolveOptions *            opts;
	int                       d;
	int                       bound;
	bool                      niss;
	AlgList *                 sols;
	Alg *                     current_alg;
	Solver *                  solver;
	Threader *                threader;
};
*/

struct
fstcube
{
	uint16_t                  uf_eofb;
	uint16_t                  uf_eposepe;
	uint16_t                  uf_coud;
	uint16_t                  uf_cp;
	uint16_t                  fr_eofb;
	uint16_t                  fr_eposepe;
	uint16_t                  fr_coud;
	uint16_t                  rd_eofb;
	uint16_t                  rd_eposepe;
	uint16_t                  rd_coud;
};

struct
indexer
{
	int                       n;
	uint64_t                  (*index)(Cube *);
	void                      (*to_cube)(uint64_t, Cube *);
};

struct
moveset
{
	char *                    name;
	bool                      (*allowed)(Move);
	bool                      (*can_append)(Alg *, Move, bool);
	bool                      (*cancel_niss)(Alg *);
	Move                      sorted_moves[NMOVES+1];
};

struct
prunedata
{
	entry_group_t *           ptable;
	uint64_t                  n;
	Coordinate *              coord;
	Moveset *                 moveset;
	uint64_t                  count[16];
	bool                      compact;
	int                       base;
};

struct
solveoptions
{
	int                       min_moves;
	int                       max_moves;
	int                       max_solutions;
	int                       nthreads;
	int                       optimal;
	bool                      can_niss;
	bool                      verbose;
	bool                      all;
	bool                      print_number;
	bool                      count_only;
};

struct
step
{
	Checker                   ready;
	bool                      final;
	Moveset *                 moveset;
	int                       n_coord;
	Coordinate *              coord[MAX_N_COORD];
	Trans                     coord_trans[MAX_N_COORD];
	PruneData *               pd[MAX_N_COORD];
	bool                      pd_compact[MAX_N_COORD];
	Validator                 is_valid;
	/*DfsMover                  custom_move_checkstop;*/
	DfsExtraCopier            copy_extra;
};

/*
struct
threaddatasolve
{
	DfsArg                    arg;
	int                       thid;
	AlgList *                 start;
	AlgListNode **            node;
	pthread_mutex_t *         start_mutex;
};
*/

struct
threaddatagenpt
{
	int                       thid;
	int                       nthreads;
	PruneData *               pd;
	int                       d;
	int                       nchunks;
	pthread_mutex_t **        mutex;
	pthread_mutex_t *         upmutex;
};

struct
transgroup
{
	int                       n;
	Trans                     t[NTRANS];
};

#endif
