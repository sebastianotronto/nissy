#ifndef COMMANDS_H
#define COMMANDS_H

#include <time.h>

#include "solve.h"
#include "steps.h"
#include "solver_step.h"
#include "threader_single.h"
#include "threader_eager.h"

void                    free_args(CommandArgs *args);
CommandArgs *           new_args();

/* Arg parsing functions *****************************************************/

CommandArgs *           gen_parse_args(int c, char **v);
CommandArgs *           help_parse_args(int c, char **v);
CommandArgs *           parse_only_scramble(int c, char **v);
CommandArgs *           parse_no_arg(int c, char **v);
CommandArgs *           solve_parse_args(int c, char **v);
CommandArgs *           scramble_parse_args(int c, char **v);

/* Exec functions ************************************************************/

void                    gen_exec(CommandArgs *args);
void                    cleanup_exec(CommandArgs *args);
void                    invert_exec(CommandArgs *args);
void                    solve_exec(CommandArgs *args);
void                    scramble_exec(CommandArgs *args);
void                    steps_exec(CommandArgs *args);
void                    commands_exec(CommandArgs *args);
void                    freemem_exec(CommandArgs *args);
void                    print_exec(CommandArgs *args);
/*void                    twophase_exec(CommandArgs *args);*/
void                    help_exec(CommandArgs *args);
void                    quit_exec(CommandArgs *args);
void                    unniss_exec(CommandArgs *args);
void                    version_exec(CommandArgs *args);

/* Commands ******************************************************************/

#ifndef COMMANDS_C

extern Command cleanup_cmd;
extern Command commands_cmd;
extern Command freemem_cmd;
extern Command gen_cmd;
extern Command help_cmd;
extern Command invert_cmd;
extern Command print_cmd;
extern Command quit_cmd;
extern Command scramble_cmd;
extern Command solve_cmd;
extern Command steps_cmd;
extern Command unniss_cmd;
extern Command version_cmd;

extern Command *commands[];

#else

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
freemem_cmd = {
	.name        = "freemem",
	.usage       = "freemem",
	.description = "free large tables from RAM",
	.parse_args  = parse_no_arg,
	.exec        = freemem_exec,
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

/*
Command
twophase_cmd = {
	.name        = "twophase",
	.usage       = "twophase",
	.description = "Find a solution quickly using a 2-phase method",
	.parse_args  = parse_only_scramble,
	.exec        = twophase_exec,
};
*/

Command
quit_cmd = {
	.name        = "quit",
	.usage       = "quit",
	.description = "Quit nissy",
	.parse_args  = parse_no_arg,
	.exec        = quit_exec,
};

Command
cleanup_cmd = {
	.name        = "cleanup",
	.usage       = "cleanup SCRAMBLE",
	.description = "Rewrite a scramble using only standard moves (HTM)",
	.parse_args  = parse_only_scramble,
	.exec        = cleanup_exec,
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

Command *commands[] = {
	&commands_cmd,
	&freemem_cmd,
	&gen_cmd,
	&help_cmd,
	&invert_cmd,
	&print_cmd,
	&quit_cmd,
	&solve_cmd,
	&scramble_cmd,
	&steps_cmd,
/*	&twophase_cmd,*/
	&cleanup_cmd,
	&unniss_cmd,
	&version_cmd,
	NULL
};

#endif

#endif
