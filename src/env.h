#ifndef ENV_H
#define ENV_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

void init_env();

extern char *tabledir;

#endif
