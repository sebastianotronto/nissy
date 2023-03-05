#define COMMANDS_C

#include "commands.h"

static bool             read_cs(CommandArgs *args, char *str);
static bool             read_scrtype(CommandArgs *args, char *str);
static bool             read_scramble(int c, char **v, CommandArgs *args);

/* Arg parsing functions implementation **************************************/

CommandArgs *
solve_parse_args(int c, char **v)
{
	int i;
	bool infinitesols, fixedmsols;
	long val;

	CommandArgs *a = new_args();

	a->opts->min_moves     = 0;
	a->opts->max_moves     = 20;
	a->opts->max_solutions = 1;
	a->opts->nthreads      = 1;
	a->opts->optimal       = -1;
	a->opts->can_niss      = false;
	a->opts->verbose       = false;
	a->opts->all           = false;
	a->opts->print_number  = true;
	a->opts->count_only    = false;

	fixedmsols = false;
	infinitesols = false;

	for (i = 0; i < c; i++) {
		if (!strcmp(v[i], "-m") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 0 || val > 100) {
				fprintf(stderr,
					"Invalid min number of moves"
					"(0 <= N <= 100).\n");
				return a;
			}
			a->opts->min_moves = val;
		} else if (!strcmp(v[i], "-M") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 0 || val > 100) {
				fprintf(stderr,
					"Invalid max number of moves"
					"(0 <= N <= 100).\n");
				return a;
			}
			a->opts->max_moves = val;
			infinitesols = true;
		} else if (!strcmp(v[i], "-t") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 1 || val > 64) {
				fprintf(stderr,
					"Invalid number of threads."
					"1 <= t <= 64\n");
				return a;
			}
			a->opts->nthreads = val;
		} else if (!strcmp(v[i], "-n") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 1 || val > 1000000) {
				fprintf(stderr,
					"Invalid number of solutions.\n");
				return a;
			}
			a->opts->max_solutions = val;
			fixedmsols = true;
		} else if (!strcmp(v[i], "-o")) {
			a->opts->optimal = 0;
			infinitesols = true;
		} else if (!strcmp(v[i], "-O") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 0 || val > 100 ||
			    (val == 0 && strcmp("0", v[i]))) {
				fprintf(stderr,
					"Invalid max number of moves"
					" (0 <= N <= 100).\n");
				return a;
			}
			a->opts->optimal = val;
			infinitesols = true;
		} else if (!strcmp(v[i], "-N")) {
			a->opts->can_niss = true;
		} else if (!strcmp(v[i], "-i")) {
			a->scrstdin = true;
		} else if (!strcmp(v[i], "-v")) {
			a->opts->verbose = true;
		} else if (!strcmp(v[i], "-a")) {
			a->opts->all = true;
		} else if (!strcmp(v[i], "-p")) {
			a->opts->print_number = false;
		} else if (!strcmp(v[i], "-c")) {
			a->opts->count_only = true;
		} else if (!read_cs(a, v[i])) {
			break;
		}
	}

	if (infinitesols && !fixedmsols)
		a->opts->max_solutions = 1000000; /* 1M = +infty */

	a->success = (a->scrstdin && i == c) || read_scramble(c-i, &v[i], a);
	return a;
}

CommandArgs *
scramble_parse_args(int c, char **v)
{
	int i;
	long val;

	CommandArgs *a = new_args();

	a->success = true;
	a->n       = 1;

	for (i = 0; i < c; i++) {
		if (!strcmp(v[i], "-n") && i+1 < c) {
			val = strtol(v[++i], NULL, 10);
			if (val < 1 || val > 1000000) {
				fprintf(stderr,
					"Invalid number of scrambles.\n");
				a->success = false;
				return a;
			}
			a->n = val;
		} else if (!read_scrtype(a, v[i])) {
			a->success = false;
			return a;
		}
	}

	return a;
}

CommandArgs *
gen_parse_args(int c, char **v)
{
	int val;
	CommandArgs *a = new_args();

	a->opts->nthreads = 64;
	a->success  = false;

	if (c == 0) {
		a->success = true;
	} else {
		if (!strcmp(v[0], "-t") && c > 1) {
			val = strtol(v[1], NULL, 10);
			if (val < 1 || val > 64) {
				fprintf(stderr,
					"Invalid number of threads."
					"1 <= t <= 64\n");
				return a;
			}
			a->opts->nthreads = val;
			a->success = true;
		}
	}
	
	return a;
}

CommandArgs *
help_parse_args(int c, char **v)
{
	int i;
	CommandArgs *a = new_args();

	if (c == 1) {
		for (i = 0; commands[i] != NULL; i++)
			if (!strcmp(v[0], commands[i]->name))
				a->command = commands[i];
		if (a->command == NULL)
			fprintf(stderr, "%s: command not found\n", v[0]);
	}

	a->success = c == 0 || (c == 1 && a->command != NULL);
	return a;
}

CommandArgs *
parse_only_scramble(int c, char **v)
{
	CommandArgs *a = new_args();

	if (!strcmp(v[0], "-i")) {
		a->scrstdin = true;
		a->success = c == 1;
	} else {
		a->success = read_scramble(c, v, a);
	}

	return a;
}

CommandArgs *
parse_no_arg(int c, char **v)
{
	CommandArgs *a = new_args();

	a->success = true;

	return a;
}

/* Exec functions implementation *********************************************/

void
solve_exec(CommandArgs *args)
{
	Cube c;
	AlgList *sols;
	Solver *solver[99];
	Threader *threader;

	make_solved(&c);
	apply_alg(args->scramble, &c);
/* TODO: adjust */
/*	threader = &threader_single;*/
	threader = &threader_eager;

/* TODO: adjust */
	int i;
	for (i = 0; args->cs->step[i] != NULL; i++)
		solver[i] = new_stepsolver_lazy(args->cs->step[i]);
	solver[i] = NULL;
	sols = solve(&c, args->opts, solver, threader);

	if (args->opts->count_only)
		printf("%d\n", sols->len);
	else
		print_alglist(sols, args->opts->print_number);

	free_alglist(sols);
}

void
scramble_exec(CommandArgs *args)
{
	Cube cube;
	Alg *scr, *ruf, *aux;
	int i, j, eo, ep, co, cp;
	uint64_t ui, uj, uk;

	srand(time(NULL));

	for (i = 0; i < args->n; i++) {

		if (!strcmp(args->scrtype, "dr")) {
			ui = rand() % FACTORIAL8;
			uj = rand() % FACTORIAL8;
			uk = rand() % FACTORIAL4;

			make_solved(&cube);
			index_to_perm(ui, 8, cube.cp);
			index_to_perm(uj, 8, cube.ep);
			index_to_perm(uk, 4, cube.ep + 8);
			for (j = 8; j < 12; j++)
				cube.ep[j] += 8;
		} else if (!strcmp(args->scrtype, "htr")) {
			make_solved(&cube);
			/* TODO */
		} else {
			ep = rand() % FACTORIAL12;
			cp = rand() % FACTORIAL8;
			eo = rand() % POW2TO11;
			co = rand() % POW3TO7;

			if (!strcmp(args->scrtype, "eo")) {
				eo = 0;
			} else if (!strcmp(args->scrtype, "corners")) {
				eo = 0;
				ep = 0;
			} else if (!strcmp(args->scrtype, "edges")) {
				co = 0;
				cp = 0;
			}

			make_solved(&cube);
			index_to_perm(ep, 12, cube.ep);
			index_to_perm(cp,  8, cube.cp);
			int_to_sum_zero_array(eo, 2, 12, cube.eo);
			int_to_sum_zero_array(co, 3,  8, cube.co);
		}

		if (!is_admissible(&cube)) {
			if (!strcmp(args->scrtype, "corners"))
				swap(&cube.cp[UFR], &cube.cp[UFL]);
			else
				swap(&cube.ep[UF], &cube.ep[UB]);
		}

		/* TODO: can be optimized for htr and dr using htrfin, drfin */
		/*
		TODO: solve_2phase was removed
		scr = solve_2phase(&cube, 1);
		*/

		if (!strcmp(args->scrtype, "fmc")) {
			aux = new_alg("");
			copy_alg(scr, aux);
			/* Trick to rufify for free: rotate the scramble  *
			 * so that it does not start with F or end with R */
			for (j = 0; j < NROTATIONS; j++) {
				if (base_move(scr->move[0]) != F &&
				    base_move(scr->move[0]) != B &&
				    base_move(scr->move[scr->len-1]) != R &&
				    base_move(scr->move[scr->len-1]) != L)
					break;
				copy_alg(aux, scr);
				transform_alg(j, scr);
			}
			copy_alg(scr, aux);
			ruf = new_alg("R' U' F");
			copy_alg(ruf, scr);
			compose_alg(scr, aux);
			compose_alg(scr, ruf);
			free_alg(aux);
			free_alg(ruf);
		}
		print_alg(scr, false);
		free_alg(scr);
	}
}

void
gen_exec(CommandArgs *args)
{
/* TODO:
	int i;

	fprintf(stderr, "Generating coordinates...\n");
	fprintf(stderr, "Generating pruning tables...\n");
	for (i = 0; all_pd[i] != NULL; i++)
		genptable(all_pd[i], args->opts->nthreads);
*/

	fprintf(stderr, "Done!\n");
}

void
invert_exec(CommandArgs *args)
{
	Alg *inv;

	inv = inverse_alg(args->scramble);
	print_alg(inv, false);

	free_alg(inv);
}

void
steps_exec(CommandArgs *args)
{
	int i;

	for (i = 0; csteps[i] != NULL; i++)
		printf("%-15s %s\n", csteps[i]->shortname, csteps[i]->name);
}

void
commands_exec(CommandArgs *args)
{
	int i;

	for (i = 0; commands[i] != NULL; i++)
		printf("%s\n", commands[i]->usage);

}

void
freemem_exec(CommandArgs *args)
{
/* TODO:
	int i;

	for (i = 0; all_pd[i] != NULL; i++)
		free_pd(all_pd[i]);

	for (i = 0; all_sd[i] != NULL; i++)
		free_sd(all_sd[i]);
*/
}

void
print_exec(CommandArgs *args)
{
	Cube c;

	make_solved(&c);
	apply_alg(args->scramble, &c);
	print_cube(&c);
}

/*
void
twophase_exec(CommandArgs *args)
{
	Cube c;
	Alg *sol;

	make_solved(&c);
	apply_alg(args->scramble, &c);
	sol = solve_2phase(&c, 1);

	print_alg(sol, false);
	free_alg(sol);
}
*/

void
help_exec(CommandArgs *args)
{
	if (args->command == NULL) {
		printf(
		       "Use the nissy command \"help COMMAND\" for a short "
		       "description of a specific command.\n"
		       "Use the nissy command \"commands\" for a list of "
		       "available commands.\n"
		       "See the manual page for more details. The manual"
		       " page is available with \"man nissy\" on a UNIX"
		       " system (such as Linux or MacOS) or in pdf and html"
		       " format in the docs folder.\n"
		       "Nissy is available for free at "
		       "https://nissy.tronto.net\n"
		      );
	} else {
		printf("Command %s: %s\nusage: %s\n", args->command->name,
		       args->command->description, args->command->usage);
	}
}

void
quit_exec(CommandArgs *args)
{
	exit(0);
}

void
cleanup_exec(CommandArgs *args)
{
	Alg *alg;

	alg = cleanup(args->scramble);
	print_alg(alg, false);

	free_alg(alg);
}

void
unniss_exec(CommandArgs *args)
{
	Alg *aux;

	aux = unniss(args->scramble);
	print_alg(aux, false);
	free(aux);
}

void
version_exec(CommandArgs *args)
{
	printf(VERSION"\n");
}

/* Local functions implementation ********************************************/

static bool
read_scramble(int c, char **v, CommandArgs *args)
{
	int i, k, n;
	unsigned int j;
	char *algstr;

	if (c < 1) {
		fprintf(stderr, "Error: no scramble given?\n");
		return false;
	}

	for(n = 0, i = 0; i < c; i++)
		n += strlen(v[i]);

	algstr = malloc((n + 1) * sizeof(char));
	k = 0;
	for (i = 0; i < c; i++)
		for (j = 0; j < strlen(v[i]); j++)
			algstr[k++] = v[i][j];
	algstr[k] = 0;

	args->scramble = new_alg(algstr);
	free(algstr);

	if (args->scramble->len == 0)
		fprintf(stderr, "Error reading scramble\n");

	return args->scramble->len > 0;
}

static bool
read_scrtype(CommandArgs *args, char *str)
{
	int i;
	static char *scrtypes[20] =
	    { "eo", "corners", "edges", "fmc", "dr", "htr", NULL };

	for (i = 0; scrtypes[i] != NULL; i++) {
		if (!strcmp(scrtypes[i], str)) {
			strcpy(args->scrtype, scrtypes[i]);
			return true;
		}
	}

	return false;
}

static bool
read_cs(CommandArgs *args, char *str)
{
	int i;

	for (i = 0; csteps[i] != NULL; i++) {
		if (!strcmp(csteps[i]->shortname, str)) {
			args->cs = csteps[i];
			return true;
		}
	}

	return false;
}

/* Public functions implementation *******************************************/

void
free_args(CommandArgs *args)
{
	if (args == NULL)
		return;

	if (args->scramble != NULL)
		free_alg(args->scramble);
	if (args->opts != NULL)
		free(args->opts);

	/* step and command must not be freed, they are static! */

	free(args);
}

CommandArgs *
new_args()
{
	CommandArgs *args = malloc(sizeof(CommandArgs));

	args->success = false;
	args->scrstdin = false;
	args->scramble = NULL; /* initialized in read_scramble */
	args->opts = malloc(sizeof(SolveOptions));

	/* step and command are static */
	args->cs = csteps[0]; /* default: first step in list */
	args->command = NULL;

	return args;
}
