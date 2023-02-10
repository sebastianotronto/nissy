#define ALG_C

#include "alg.h"

static int         axis(Move m);
static void        free_alglistnode(AlgListNode *aln);
static void        realloc_alg(Alg *alg, int n);

bool
allowed_HTM(Move m)
{
	return m >= U && m <= B3;
}

bool
allowed_URF(Move m)
{
	Move b = base_move(m);

	return b == U || b == R || b == F;
}

bool
allowed_eofb(Move m)
{
	Move b = base_move(m);

	return b == U || b == D || b == R || b == L ||
	       ((b == F || b == B) && m == b+1);
}

bool
allowed_drud(Move m)
{
	Move b = base_move(m);

	return b == U || b == D ||
	       ((b == R || b == L || b == F || b == B) && m == b + 1);
}

bool
allowed_htr(Move m)
{
	Move b = base_move(m);

	return moveset_HTM.allowed(m) && m == b + 1;
}

bool
allowed_next_all(Move l2, Move l1, Move m)
{
	bool p, q;

	p = l1 != NULLMOVE && base_move(l1) == base_move(m);
	q = l2 != NULLMOVE && base_move(l2) == base_move(m);

	return !(p || (commute(l1, l2) && q));
}

void
append_alg(AlgList *l, Alg *alg)
{
	AlgListNode *node = malloc(sizeof(AlgListNode));
	int i;

	node->alg = new_alg("");
	for (i = 0; i < alg->len; i++)
		append_move(node->alg, alg->move[i], alg->inv[i]);
	node->next = NULL;

	if (++l->len == 1)
		l->first = node;
	else
		l->last->next = node;
	l->last = node;
}

void
append_move(Alg *alg, Move m, bool inverse)
{
	if (alg->len == alg->allocated)
		realloc_alg(alg, 2*alg->len);

	alg->move[alg->len] = m;
	alg->inv [alg->len] = inverse;
	alg->len++;

	if (inverse)
		alg->move_inverse[alg->len_inverse++] = m;
	else
		alg->move_normal[alg->len_normal++] = m;
}

static int
axis(Move m)
{
	static int aux[] = {
		[NULLMOVE] = 0,

		[U]  = 1, [U2]  = 1, [U3]  = 1,
		[D]  = 1, [D2]  = 1, [D3]  = 1,
		[Uw] = 1, [Uw2] = 1, [Uw3] = 1,
		[Dw] = 1, [Dw2] = 1, [Dw3] = 1,
		[E]  = 1, [E2]  = 1, [E3]  = 1,
		[y]  = 1, [y2]  = 1, [y3]  = 1,

		[R]  = 2, [R2]  = 2, [R3]  = 2,
		[L]  = 2, [L2]  = 2, [L3]  = 2,
		[Rw] = 2, [Rw2] = 2, [Rw3] = 2,
		[Lw] = 2, [Lw2] = 2, [Lw3] = 2,
		[M]  = 2, [M2]  = 2, [M3]  = 2,
		[x]  = 2, [x2]  = 2, [x3]  = 2,

		[F]  = 3, [F2]  = 3, [F3]  = 3,
		[B]  = 3, [B2]  = 3, [B3]  = 3,
		[Fw] = 3, [Fw2] = 3, [Fw3] = 3,
		[Bw] = 3, [Bw2] = 3, [Bw3] = 3,
		[S]  = 3, [S2]  = 3, [S3]  = 3,
		[z]  = 3, [z2]  = 3, [z3]  = 3,
	};

	return aux[m];
}

Move
base_move(Move m)
{
	if (m == NULLMOVE)
		return NULLMOVE;
	else
		return m - (m-1)%3;
}

bool
commute(Move m1, Move m2)
{
	return axis(m1) == axis(m2);
}

void
compose_alg(Alg *alg1, Alg *alg2)
{
	int i;

	for (i = 0; i < alg2->len; i++)
		append_move(alg1, alg2->move[i], alg2->inv[i]);
}

void
copy_alg(Alg *src, Alg *dst)
{
	dst->len = dst->len_normal = dst->len_inverse = 0;
	compose_alg(dst, src);
}

void
free_alg(Alg *alg)
{
	free(alg->move);
	free(alg->inv);
	free(alg);
}

void
free_alglist(AlgList *l)
{
	AlgListNode *aux, *i = l->first;

	while (i != NULL) {
		aux = i->next;
		free_alglistnode(i);
		i = aux;
	}
	free(l);
}

static void
free_alglistnode(AlgListNode *aln)
{
	free_alg(aln->alg);
	free(aln);
}

Alg *
inverse_alg(Alg *alg)
{
	Alg *ret = new_alg("");
	int i;

	for (i = alg->len-1; i >= 0; i--)
		append_move(ret, inverse_move(alg->move[i]), alg->inv[i]);

	return ret;
}

Move
inverse_move(Move m)
{
	return m == NULLMOVE ? NULLMOVE : m + 2 - 2*((m-1) % 3);
}

char *
move_string(Move m)
{
	static char move_string_aux[NMOVES][7] = {
		[NULLMOVE] = "-",
		[U]  = "U",  [U2]  = "U2",  [U3]  = "U\'",
		[D]  = "D",  [D2]  = "D2",  [D3]  = "D\'",
		[R]  = "R",  [R2]  = "R2",  [R3]  = "R\'",
		[L]  = "L",  [L2]  = "L2",  [L3]  = "L\'",
		[F]  = "F",  [F2]  = "F2",  [F3]  = "F\'",
		[B]  = "B",  [B2]  = "B2",  [B3]  = "B\'",
		[Uw] = "Uw", [Uw2] = "Uw2", [Uw3] = "Uw\'",
		[Dw] = "Dw", [Dw2] = "Dw2", [Dw3] = "Dw\'",
		[Rw] = "Rw", [Rw2] = "Rw2", [Rw3] = "Rw\'",
		[Lw] = "Lw", [Lw2] = "Lw2", [Lw3] = "Lw\'",
		[Fw] = "Fw", [Fw2] = "Fw2", [Fw3] = "Fw\'",
		[Bw] = "Bw", [Bw2] = "Bw2", [Bw3] = "Bw\'",
		[M]  = "M",  [M2]  = "M2",  [M3]  = "M\'",
		[E]  = "E",  [E2]  = "E2",  [E3]  = "E\'",
		[S]  = "S",  [S2]  = "S2",  [S3]  = "S\'",
		[x]  = "x",  [x2]  = "x2",  [x3]  = "x\'",
		[y]  = "y",  [y2]  = "y2",  [y3]  = "y\'",
		[z]  = "z",  [z2]  = "z2",  [z3]  = "z\'",
	};

	return move_string_aux[m];
}

Alg *
new_alg(char *str)
{
	Alg *alg;
	int i;
	bool niss, move_read;
	Move j, m;

	alg = malloc(sizeof(Alg));
	alg->allocated    = 30;
	alg->move         = malloc(alg->allocated * sizeof(Move));
	alg->inv          = malloc(alg->allocated * sizeof(bool));
	alg->move_normal  = malloc(alg->allocated * sizeof(Move));
	alg->move_inverse = malloc(alg->allocated * sizeof(Move));
	alg->len          = 0;
	alg->len_normal   = 0;
	alg->len_inverse  = 0;

	niss = false;
	for (i = 0; str[i]; i++) {
		if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n')
			continue;

		if (str[i] == '(' && niss) {
			fprintf(stderr, "Error reading moves: nested ( )\n");
			alg->len = alg->len_normal = alg->len_inverse = 0;
			return alg;
		}

		if (str[i] == ')' && !niss) {
			fprintf(stderr, "Error reading moves: unmatched )\n");
			alg->len = alg->len_normal = alg->len_inverse = 0;
			return alg;
		}

		if (str[i] == '(' || str[i] == ')') {
			niss = !niss;
			continue;
		}

		/* Single slash for comments */
		if (str[i] == '/') {
			while (str[i] && str[i] != '\n')
				i++;

			if (!str[i])
				i--;

			continue;
		}

		move_read = false;
		for (j = U; j < NMOVES; j++) {
			if (str[i] == move_string(j)[0] ||
			    (str[i] >= 'a' && str[i] <= 'z' &&
			     str[i] == move_string(j)[0]-('A'-'a') && j<=B)) {
				m = j;
				if (str[i] >= 'a' && str[i] <= 'z' && j<=B) {
					m += Uw - U;
				}
				if (m <= B && str[i+1]=='w') {
					m += Uw - U;
					i++;
				}
				if (str[i+1]=='2') {
					m += 1;
					i++;
				} else if (str[i+1] == '\'' ||
				           str[i+1] == '3'  ||
					   str[i+1] == '`' ) {
					m += 2;
					i++;
				} else if ((int)str[i+1] == -62 &&
					   (int)str[i+2] == -76) {
					/* Weird apostrophe */
					m += 2;
					i += 2;
				} else if ((int)str[i+1] == -30 &&
					   (int)str[i+2] == -128 &&
					   (int)str[i+3] == -103) {
					/* MacOS apostrophe */
					m += 2;
					i += 3;
				}
				append_move(alg, m, niss);
				move_read = true;
				break;
			}
		}

		if (!move_read) {
			free(alg);
			return new_alg("");
		}
	}

	if (niss) {
		fprintf(stderr, "Error reading moves: unmatched (\n");
		alg->len = alg->len_normal = alg->len_inverse = 0;
	}

	return alg;
}

AlgList *
new_alglist()
{
	AlgList *ret = malloc(sizeof(AlgList));

	ret->len   = 0;
	ret->first = NULL;
	ret->last  = NULL;

	return ret;
}

Alg *
on_inverse(Alg *alg)
{
	Alg *ret = new_alg("");
	int i;

	for (i = 0; i < alg->len; i++)
		append_move(ret, alg->move[i], !alg->inv[i]);

	return ret;
}

bool
possible_next(Move m, Moveset *ms, Move l0, Move l1)
{
	bool allowed, order;
	uint64_t mbit;

	mbit    = ((uint64_t)1) << m;
	allowed = mbit & ms->mask[l1][l0];
	order   = !commute(l0, m) || l0 < m;

	return allowed && order;
}

void
print_alg(Alg *alg, bool l)
{
	char fill[4];
	int i;
	bool niss = false;

	for (i = 0; i < alg->len; i++) {
		if (!niss && alg->inv[i])
			strcpy(fill, i == 0 ? "(" : " (");
		if (niss && !alg->inv[i])
			strcpy(fill, ") ");
		if (niss == alg->inv[i])
			strcpy(fill, i == 0 ? "" : " ");

		printf("%s%s", fill, move_string(alg->move[i]));
		niss = alg->inv[i];
	}

	if (niss)
		printf(")");
	if (l)
		printf(" (%d)", alg->len);

	printf("\n");
}

void
print_alglist(AlgList *al, bool l)
{
	AlgListNode *i;

	for (i = al->first; i != NULL; i = i->next)
		print_alg(i->alg, l);
}

static void
realloc_alg(Alg *alg, int n)
{
	if (alg == NULL) {
		fprintf(stderr, "Error: trying to reallocate NULL alg.\n");
		return;
	}

	if (n < alg->len) {
		fprintf(stderr, "Error: alg too long for reallocation ");
		fprintf(stderr, "(%d vs %d)\n", alg->len, n);
		return;
	}

	if (n > 1000000) {
		fprintf(stderr, "Warning: very long alg,");
		fprintf(stderr, "something might go wrong.\n");
	}

	alg->move         = realloc(alg->move,         n * sizeof(int));
	alg->inv          = realloc(alg->inv,          n * sizeof(int));
	alg->move_normal  = realloc(alg->move_normal,  n * sizeof(int));
	alg->move_inverse = realloc(alg->move_inverse, n * sizeof(int));
	alg->allocated = n;
}

void
swapmove(Move *m1, Move *m2)
{
	Move aux;

	aux = *m1;
	*m1 = *m2;
	*m2 = aux;
}

char *
trans_string(Trans t)
{
	static char trans_string_aux[NTRANS][20] = {
		[uf]  = "uf",  [ur]  = "ur", [ub] = "ub", [ul] = "ul",
		[df]  = "df",  [dr]  = "dr", [db] = "db", [dl] = "dl",
		[rf]  = "rf",  [rd]  = "rd", [rb] = "rb", [ru] = "ru",
		[lf]  = "lf",  [ld]  = "ld", [lb] = "lb", [lu] = "lu",
		[fu]  = "fu",  [fr]  = "fr", [fd] = "fd", [fl] = "fl",
		[bu]  = "bu",  [br]  = "br", [bd] = "bd", [bl] = "bl",

		[uf_mirror] = "uf*", [ur_mirror] = "ur*",
		[ub_mirror] = "ub*", [ul_mirror] = "ul*",
		[df_mirror] = "df*", [dr_mirror] = "dr*",
		[db_mirror] = "db*", [dl_mirror] = "dl*",
		[rf_mirror] = "rf*", [rd_mirror] = "rd*",
		[rb_mirror] = "rb*", [ru_mirror] = "ru*",
		[lf_mirror] = "lf*", [ld_mirror] = "ld*",
		[lb_mirror] = "lb*", [lu_mirror] = "lu*",
		[fu_mirror] = "fu*", [fr_mirror] = "fr*",
		[fd_mirror] = "fd*", [fl_mirror] = "fl*",
		[bu_mirror] = "bu*", [br_mirror] = "br*",
		[bd_mirror] = "bd*", [bl_mirror] = "bl*",
	};

	return trans_string_aux[t];
}

Alg *
unniss(Alg *alg)
{
	int i;
	Alg *ret;

	ret = new_alg("");

	for (i = 0; i < alg->len_normal; i++)
		append_move(ret, alg->move_normal[i], false);

	for (i = 0; i < alg->len_inverse; i++)
		append_move(ret, inverse_move(alg->move_inverse[i]), false);

	return ret;
}

void
init_moveset(Moveset *ms)
{
	int j;
	uint64_t l, one;
	Move m, l2, l1;

	one = 1;

	for (j = 0, m = U; m < NMOVES; m++)
		if (ms->allowed(m))
			ms->sorted_moves[j++] = m;
	ms->sorted_moves[j] = NULLMOVE;

	for (l1 = 0; l1 < NMOVES; l1++) { 
		for (l2 = 0; l2 < NMOVES; l2++) { 
			ms->mask[l2][l1] = 0;
			for (l = 0; ms->sorted_moves[l] != NULLMOVE; l++) {
				m = ms->sorted_moves[l];
				if (ms->allowed_next(l2, l1, m))
					ms->mask[l2][l1] |= (one<<m);
			}
		}
	}
}
