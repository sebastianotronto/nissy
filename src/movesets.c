#define MOVESETS_C

#include "movesets.h"

static bool allowed_HTM(Move m);
static bool allowed_URF(Move m);
static bool allowed_eofb(Move m);
static bool allowed_drud(Move m);
static bool allowed_htr(Move m);
static bool can_append_HTM(Move l2, Move l1, Move m);
static bool can_append_HTM_cached(Alg *alg, Move m, bool inverse);
static bool cancel_niss_HTM_cached(Alg *alg);
static void init_can_append_HTM();

Moveset
moveset_HTM = {
	.name        = "HTM",
	.allowed     = allowed_HTM,
	.can_append  = can_append_HTM_cached,
	.cancel_niss = cancel_niss_HTM_cached,
};

Moveset
moveset_URF = {
	.name        = "URF",
	.allowed     = allowed_URF,
	.can_append  = can_append_HTM_cached,
	.cancel_niss = cancel_niss_HTM_cached,
};

Moveset
moveset_eofb = {
	.name        = "eofb",
	.allowed     = allowed_eofb,
	.can_append  = can_append_HTM_cached,
	.cancel_niss = cancel_niss_HTM_cached,
};

Moveset
moveset_drud = {
	.name        = "drud",
	.allowed     = allowed_drud,
	.can_append  = can_append_HTM_cached,
	.cancel_niss = cancel_niss_HTM_cached,
};

Moveset
moveset_htr = {
	.name        = "htr",
	.allowed     = allowed_htr,
	.can_append  = can_append_HTM_cached,
	.cancel_niss = cancel_niss_HTM_cached,
};

Moveset *
all_movesets[] = {
	&moveset_HTM,
	&moveset_URF,
	&moveset_eofb,
	&moveset_drud,
	&moveset_htr,
	NULL
};

static uint64_t can_append_HTM_mask[NMOVES][NMOVES];

static bool
allowed_HTM(Move m)
{
	return m >= U && m <= B3;
}

static bool
allowed_URF(Move m)
{
	Move b = base_move(m);

	return b == U || b == R || b == F;
}

static bool
allowed_eofb(Move m)
{
	Move b = base_move(m);

	return b == U || b == D || b == R || b == L ||
	       ((b == F || b == B) && m == b+1);
}

static bool
allowed_drud(Move m)
{
	Move b = base_move(m);

	return b == U || b == D ||
	       ((b == R || b == L || b == F || b == B) && m == b + 1);
}

static bool
allowed_htr(Move m)
{
	Move b = base_move(m);

	return moveset_HTM.allowed(m) && m == b + 1;
}

static bool
can_append_HTM(Move l2, Move l1, Move m)
{
	bool cancel, cancel_last, cancel_swap;

	cancel_last = l1 != NULLMOVE && base_move(l1) == base_move(m);
	cancel_swap = l2 != NULLMOVE && base_move(l2) == base_move(m);
	cancel = cancel_last || (commute(l1, l2) && cancel_swap);

	return !cancel;
}

static bool
can_append_HTM_cached(Alg *alg, Move m, bool inverse)
{
	Move *moves, l1, l2;
	uint64_t mbit;
	int n;

	if (inverse) {
		moves = alg->move_inverse;
		n = alg->len_inverse;
	} else {
		moves = alg->move_normal;
		n = alg->len_normal;
	}

	l1 = n > 0 ? moves[n-1] : NULLMOVE;
	l2 = n > 1 ? moves[n-2] : NULLMOVE;

	mbit = ((uint64_t)1) << m;

	return can_append_HTM_mask[l2][l1] & mbit;
}

static bool
cancel_niss_HTM_cached(Alg *alg)
{
	Move i1, i2;
	int n;
	bool can_first, can_swap;

	n = alg->len_inverse;
	i1 = n > 0 ? alg->move_inverse[n-1] : NULLMOVE;
	i2 = n > 1 ? alg->move_inverse[n-2] : NULLMOVE;

	can_first = can_append_HTM_cached(alg, inverse_move(i1), false);
	can_swap  = can_append_HTM_cached(alg, inverse_move(i2), false);

	return can_first && (!commute(i1, i2) || can_swap);
}

static void
init_can_append_HTM()
{
	Move l2, l1, m;

	for (l1 = 0; l1 < NMOVES; l1++)
		for (l2 = 0; l2 < NMOVES; l2++)
			for (m = 0; m < NMOVES; m++)
				if (can_append_HTM(l2, l1, m))
					can_append_HTM_mask[l2][l1]
					    |= (((uint64_t)1) << m);
}

void
init_moveset(Moveset *ms)
{
	int j;
	Move m;

	for (j = 0, m = U; m < NMOVES; m++)
		if (ms->allowed(m))
			ms->sorted_moves[j++] = m;
	ms->sorted_moves[j] = NULLMOVE;

/* TODO: should be here? maybe just init all movesets together anyway... */
	init_can_append_HTM();
}

void
init_movesets()
{
	int i;

	for (i = 0; all_movesets[i] != NULL; i++)
		init_moveset(all_movesets[i]);
}
