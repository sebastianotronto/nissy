/* Typedefs ******************************************************************/

typedef enum center          Center;
typedef enum corner          Corner;
typedef enum edge            Edge;
typedef enum move            Move;
typedef enum trans           Trans;

typedef struct alg           Alg;
typedef struct alglist       AlgList;
typedef struct alglistnode   AlgListNode;
typedef struct block         Block;
typedef struct cube          Cube;
typedef struct cubearray     CubeArray;
typedef struct dfsdata       DfsData;
typedef struct piecefilter   PieceFilter;
typedef struct prunedata     PruneData;
typedef struct solveoptions  SolveOptions;
typedef struct step          Step;

/* Enums *********************************************************************/

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


/* Structs *******************************************************************/

struct
alg
{
	Move *          move;
	bool *          inv;
	int             len;
	int             allocated;
};

struct
alglist
{
	AlgListNode *   first;
	AlgListNode *   last;
	int             len;
};

struct
alglistnode
{
	Alg *           alg;
	AlgListNode *   next;
};

struct
block
{
	bool            edge[12];
	bool            corner[8];
	bool            center[6];
};

struct
cube
{
	uint16_t        epose;
	uint16_t        eposs;
	uint16_t        eposm;
	uint16_t        eofb;
	uint16_t        eorl;
	uint16_t        eoud;
	uint16_t        cp;
	uint16_t        coud;
	uint16_t        cofb;
	uint16_t        corl;
	uint16_t        cpos;
};

struct
cubearray
{
        int *           ep;
        int *           eofb;
        int *           eorl;
        int *           eoud;
        int *           cp;
        int *           coud;
        int *           corl;
        int *           cofb;
        int *           cpos;
};

struct
dfsdata
{
        int             d;
        int             m;
        bool            niss;
        Move            last1;
        Move            last2;
        AlgList *       sols;
        Alg *           current_alg;
        Move            sorted_moves[z3+1];
};

struct
piecefilter
{
        bool            epose;
        bool            eposs;
        bool            eposm;
        bool            eofb;
        bool            eorl;
        bool            eoud;
        bool            cp;
        bool            coud;
        bool            cofb;
        bool            corl;
        bool            cpos;
};

struct
prunedata
{
        char *          filename;
        uint8_t *       ptable;
	uint8_t *       reached;
	bool            generated;
        uint64_t        n;
        uint64_t        size;
        uint64_t        (*index)(Cube);
        bool            (*moveset)(Move);
};

struct
solveoptions
{
	int             min_moves;
	int             max_moves;
	int             max_solutions;
	bool            optimal_only;
	bool            can_niss;
	bool            feedback;
};

struct
step
{
	int             (*check)(Cube);
	int             (*ready)(Cube);
	bool            (*moveset)(Move);
	Trans           pre_trans;
};
