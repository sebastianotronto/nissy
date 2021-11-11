#include "env.h"

bool initialized_env = false;
char *tabledir;

void
init_env()
{
	char *nissydata = getenv("NISSYDATA");
	char *localdata = getenv("XDG_DATA_HOME");
	char *home      = getenv("HOME");
	bool read, write;

	if (initialized_env)
		return;

	if (nissydata != NULL) {
		tabledir = malloc(strlen(nissydata) * sizeof(char) + 20);
		strcpy(tabledir, nissydata);
	} else if (localdata != NULL) {
		tabledir = malloc(strlen(localdata) * sizeof(char) + 20);
		strcpy(tabledir, localdata);
		strcat(tabledir, "/nissy");
	} else if (home != NULL) {
		tabledir = malloc(strlen(home) * sizeof(char) + 20);
		strcpy(tabledir, home);
		strcat(tabledir, "/.nissy");
	}

	mkdir(tabledir, 0777);
	strcat(tabledir, "/tables");
	mkdir(tabledir, 0777);

	read  = !access(tabledir, R_OK);
	write = !access(tabledir, W_OK);

	if (!read) {
		fprintf(stderr, "Table files cannot be read.\n");
	} else if (!write) {
		fprintf(stderr, "Data directory not writable: ");
		fprintf(stderr, "tables can be loaded, but not saved.\n");
	}

	initialized_env = true;
}
