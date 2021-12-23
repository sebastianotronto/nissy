#include "shell.h"

static void             cleanwhitespaces(char *line);
static int              parseline(char *line, char **v);

static void
cleanwhitespaces(char *line)
{
	char *i;

	for (i = line; *i != 0; i++)
		if (*i == '\t' || *i == '\n')
			*i = ' ';
}

/* This function assumes that **v is large enough */
static int
parseline(char *line, char **v)
{
	char *t;
	int n = 0;
	
	cleanwhitespaces(line);

	for (t = strtok(line, " "); t != NULL; t = strtok(NULL, " "))
		strcpy(v[n++], t);

	return n;
}

void
exec_args(int c, char **v)
{
	int i;
	Command *cmd = NULL;
	CommandArgs *args;

	for (i = 0; i < NCOMMANDS; i++)
		if (commands[i] != NULL && !strcmp(v[0], commands[i]->name))
			cmd = commands[i];

	if (cmd == NULL) {
		fprintf(stderr, "%s: command not found\n", v[0]);
		return;
	}

	args = cmd->parse_args(c-1, &v[1]);
	if (!args->success) {
		fprintf(stderr, "usage: %s\n", cmd->usage);
		return;
	}

	cmd->exec(args);
	free_args(args);
}

void
launch(bool batchmode)
{
	int i, shell_argc;
	char line[MAXLINELEN], **shell_argv;

	shell_argv = malloc(MAXNTOKENS * sizeof(char *));
	for (i = 0; i < MAXNTOKENS; i++)
		shell_argv[i] = malloc((MAXTOKENLEN+1) * sizeof(char));

	if (!batchmode) {
		fprintf(stderr, "Welcome to Nissy "VERSION".\n"
				"Type \"commands\" for a list of commands.\n");
	}

	while (true) {
		if (!batchmode) {
			fprintf(stdout, "nissy-# ");
		}

		if (fgets(line, MAXLINELEN, stdin) == NULL)
			break;

		if (batchmode) {
			printf(">>>\n"
			       ">>> Executing command: %s"
			       ">>>\n", line);
		}

		shell_argc = parseline(line, shell_argv);

		if (shell_argc > 0)
			exec_args(shell_argc, shell_argv);
	}

	for (i = 0; i < MAXNTOKENS; i++)
		free(shell_argv[i]);
	free(shell_argv);
}

int
main(int argc, char *argv[])
{
/*
	init_movesets();
	init_symcoord();
	print_ptable(&pd_nxopt31_HTM);
*/

	if (argc > 1) {
		if (!strcmp(argv[1], "-b")) {
			launch(true);
		} else {
			exec_args(argc-1, &argv[1]);
		}
	} else {
		launch(false);
	}

	return 0;
}
