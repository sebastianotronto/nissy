#include "alg.h"

/* Local functions ***********************************************************/

static void        free_alglistnode(AlgListNode *aln);
static void        realloc_alg(Alg *alg, int n);

/* Movesets ******************************************************************/

bool
moveset_HTM(Move m)
{
	return m >= U && m <= B3;
}

bool
moveset_URF(Move m)
{
	Move b = base_move(m);

	return b == U || b == R || b == F;
}

bool
moveset_eofb(Move m)
{
	Move b = base_move(m);

	return b == U || b == D || b == R || b == L ||
	       ((b == F || b == B) && m == b+1);
}

bool
moveset_drud(Move m)
{
	Move b = base_move(m);

	return b == U || b == D ||
	       ((b == R || b == L || b == F || b == B) && m == b + 1);
}

bool
moveset_htr(Move m)
{
	Move b = base_move(m);

	return moveset_HTM(m) && m == b + 1;
}


/* Functions *****************************************************************/

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
}

Move
base_move(Move m)
{
	if (m == NULLMOVE)
		return NULLMOVE;
	else
		return m - (m-1)%3;
}

void
compose_alg(Alg *alg1, Alg *alg2)
{
	int i;

	for (i = 0; i < alg2->len; i++)
		append_move(alg1, alg2->move[i], alg2->inv[i]);
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

void
movelist_to_position(Move *movelist, int *position)
{
	Move m;

	for (m = 0; m < NMOVES && movelist[m] != NULLMOVE; m++)
		position[movelist[m]] = m;
}

void
moveset_to_list(Moveset ms, Move *r)
{
	int n = 0;
	Move i;

	if (ms == NULL) {
		fprintf(stderr, "Error: no moveset given\n");
		return;
	}

	for (i = U; i < NMOVES; i++)
		if (ms(i))
			r[n++] = i;

	r[n] = NULLMOVE;
}

Alg *
new_alg(char *str)
{
	Alg *alg = malloc(sizeof(Alg));
	int i;
	bool niss = false, move_read;
	Move j, m;

	alg->move      = malloc(30 * sizeof(Move));
	alg->inv       = malloc(30 * sizeof(bool));
	alg->allocated = 30;
	alg->len       = 0;

	for (i = 0; str[i]; i++) {
		if (str[i] == ' ' || str[i] == '\t' || str[i] == '\n')
			continue;

		if (str[i] == '(' && niss) {
			fprintf(stderr, "Error reading moves: nested ( )\n");
			return alg;
		}

		if (str[i] == ')' && !niss) {
			fprintf(stderr, "Error reading moves: unmatched )\n");
			return alg;
		}

		if (str[i] == '(' || str[i] == ')') {
			niss = !niss;
			continue;
		}

		move_read = false;
		for (j = 0; j < NMOVES; j++) {
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
			alg = new_alg("");
			return alg;
		}
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

void
print_alg(Alg *alg, bool l)
{
	/* TODO: make it possible to print to stdout or to string */
	/* Maybe just return a string */
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

	alg->move = realloc(alg->move, n * sizeof(int));
	alg->inv  = realloc(alg->inv,  n * sizeof(int));
	alg->allocated = n;
}

