#include "commands.h"

/* Arg parsing functions *****************************************************/

CommandArgs *           gen_parse_args(int c, char **v);
CommandArgs *           help_parse_args(int c, char **v);
CommandArgs *           parse_only_scramble(int c, char **v);
CommandArgs *           parse_no_arg(int c, char **v);
CommandArgs *           solve_parse_args(int c, char **v);
CommandArgs *           scramble_parse_args(int c, char **v);

/* Exec functions ************************************************************/

static void             gen_exec(CommandArgs *args);
static void             invert_exec(CommandArgs *args);
static void             solve_exec(CommandArgs *args);
static void             scramble_exec(CommandArgs *args);
static void             steps_exec(CommandArgs *args);
static void             commands_exec(CommandArgs *args);
static void             print_exec(CommandArgs *args);
static void             twophase_exec(CommandArgs *args);
static void             help_exec(CommandArgs *args);
static void             quit_exec(CommandArgs *args);
static void             unniss_exec(CommandArgs *args);
static void             version_exec(CommandArgs *args);

/* Local functions ***********************************************************/

static bool             read_step(CommandArgs *args, char *str);
static bool             read_scrtype(CommandArgs *args, char *str);
static bool             read_scramble(int c, char **v, CommandArgs *args);

/* Commands ******************************************************************/

Command
solve_cmd = {
	.name        = "solve",
	.usage       = "solve STEP [OPTIONS] SCRAMBLE",
	.description = "Solve a step; see command steps for a list of steps",
	.parse_args  = solve_parse_args,
	.exec        = solve_exec
};

Command
scramble_cmd = {
	.name        = "scramble",
	.usage       = "scramble [TYPE] [-n N]",
	.description = "Get a random-position scramble",
	.parse_args  = scramble_parse_args,
	.exec        = scramble_exec,
};

Command
gen_cmd = {
	.name        = "gen",
	.usage       = "gen [-t N]",
	.description = "Generate all tables [using N threads]",
	.parse_args  = gen_parse_args,
	.exec        = gen_exec
};

Command
invert_cmd = {
	.name        = "invert",
	.usage       = "invert SCRAMBLE]",
	.description = "Invert a scramble",
	.parse_args  = parse_only_scramble,
	.exec        = invert_exec,
};

Command
steps_cmd = {
	.name        = "steps",
	.usage       = "steps",
	.description = "List available steps",
	.parse_args  = parse_no_arg,
	.exec        = steps_exec
};

Command
commands_cmd = {
	.name        = "commands",
	.usage       = "commands",
	.description = "List available commands",
	.parse_args  = parse_no_arg,
	.exec        = commands_exec
};

Command
print_cmd = {
	.name        = "print",
	.usage       = "print SCRAMBLE",
	.description = "Print written description of the cube",
	.parse_args  = parse_only_scramble,
	.exec        = print_exec,
};

Command
help_cmd = {
	.name        = "help",
	.usage       = "help [COMMAND]",
	.description = "Display nissy manual page or help on specific command",
	.parse_args  = help_parse_args,
	.exec        = help_exec,
};

Command
twophase_cmd = {
	.name        = "twophase",
	.usage       = "twophase",
	.description = "Find a solution quickly using a 2-phase method",
	.parse_args  = parse_only_scramble,
	.exec        = twophase_exec,
};

Command
quit_cmd = {
	.name        = "quit",
	.usage       = "quit",
	.description = "Quit nissy",
	.parse_args  = parse_no_arg,
	.exec        = quit_exec,
};

Command
unniss_cmd = {
	.name        = "unniss",
	.usage       = "unniss SCRAMBLE",
	.description = "Rewrite a scramble without NISS",
	.parse_args  = parse_only_scramble,
	.exec        = unniss_exec,
};

Command
version_cmd = {
	.name        = "version",
	.usage       = "version",
	.description = "print nissy version",
	.parse_args  = parse_no_arg,
	.exec        = version_exec,
};

Command *commands[NCOMMANDS] = {
	&commands_cmd,
	&gen_cmd,
	&help_cmd,
	&invert_cmd,
	&print_cmd,
	&quit_cmd,
	&solve_cmd,
	&scramble_cmd,
	&steps_cmd,
	&twophase_cmd,
	&unniss_cmd,
	&version_cmd,
};

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
		} else if (!read_step(a, v[i])) {
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
	a->scrt    = -1;

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

	a->opts->nthreads = 1;
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
		for (i = 0; i < NCOMMANDS; i++)
			if (commands[i] != NULL &&
			    !strcmp(v[0], commands[i]->name))
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

static void
solve_exec(CommandArgs *args)
{
	Cube c;
	AlgList *sols;

	init_movesets();
	init_symcoord();

	c = apply_alg(args->scramble, (Cube){0});
	sols = solve(c, args->step, args->opts);

	if (args->opts->count_only)
		printf("%d\n", sols->len);
	else
		print_alglist(sols, args->opts->print_number);

	free_alglist(sols);
}

static void
scramble_exec(CommandArgs *args)
{
	Cube cube;
	Alg *scr;
	int i;

	init_movesets();
	init_symcoord();

	srand(time(NULL));

	for (i = 0; i < args->n; i++) {
		cube = random_cube(args->scrt);
		scr = solve_2phase(cube, 1);
		print_alg(scr, false);
		free_alg(scr);
	}
}

static void
gen_exec(CommandArgs *args)
{
	int i;

	fprintf(stderr, "Generating coordinates...\n");
	init_movesets();
	init_symcoord();

	fprintf(stderr, "Generating pruning tables...\n");
	for (i = 0; i < NPTABLES && allpd[i] != NULL; i++)
		genptable(allpd[i], args->opts->nthreads);

	fprintf(stderr, "Done!\n");
}

static void
invert_exec(CommandArgs *args)
{
	Alg *inv;

	inv = inverse_alg(args->scramble);
	print_alg(inv, false);

	free_alg(inv);
}

static void
steps_exec(CommandArgs *args)
{
	int i;

	for (i = 0; i < NSTEPS && steps[i] != NULL; i++)
		printf("%-15s %s\n", steps[i]->shortname, steps[i]->name);
}

static void
commands_exec(CommandArgs *args)
{
	int i;

	for (i = 0; i < NCOMMANDS && commands[i] != NULL; i++)
		printf("%s\n", commands[i]->usage);

}

static void
print_exec(CommandArgs *args)
{
	init_moves();
	print_cube(apply_alg(args->scramble, (Cube){0}));
}

static void
twophase_exec(CommandArgs *args)
{
	Cube c;
	Alg *sol;

	init_movesets();
	init_symcoord();

	c = apply_alg(args->scramble, (Cube){0});
	sol = solve_2phase(c, 1);

	print_alg(sol, false);
	free_alg(sol);
}

static void
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
		       "https://github.com/sebastianotronto/nissy\n"
		      );
	} else {
		printf("Command %s: %s\nusage: %s\n", args->command->name,
		       args->command->description, args->command->usage);
	}
}

static void
quit_exec(CommandArgs *args)
{
	exit(0);
}

static void
unniss_exec(CommandArgs *args)
{
	unniss(args->scramble);
	print_alg(args->scramble, false);
}

static void
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
	Alg *aux;

	if (c < 1) {
		fprintf(stderr, "Error: no scramble given?\n");
		return false;
	}

	n = 0;
	for(i = 0; i < c; i++) {
		aux = new_alg(v[i]);
		if (aux->len == 0) {
			fprintf(stderr, "Error: %s or its argument"
			    "unrecognized\n", v[i]);
			free(aux);
			return false;
		}
		free(aux);
		n += strlen(v[i]);
	}

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

	args->scrt = -1;
	for (i = 0; i < NSCRTYPES; i++)
		if (!strcmp(scrtypes[i], str))
			args->scrt = i;

	return args->scrt != -1;
}

static bool
read_step(CommandArgs *args, char *str)
{
	int i;

	for (i = 0; i < NSTEPS; i++) {
		if (steps[i] != NULL && !strcmp(steps[i]->shortname, str)) {
			args->step = steps[i];
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
	args->step = steps[0]; /* default: first step in list */
	args->command = NULL;

	return args;
}
