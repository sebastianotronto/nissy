#ifndef COMMANDS_H
#define COMMANDS_H

#include "solve.h"
#include "steps.h"

#define NCOMMANDS       10

void                    free_args(CommandArgs *args);

extern Command *        commands[NCOMMANDS];

#endif
