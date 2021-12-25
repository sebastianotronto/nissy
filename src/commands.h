#ifndef COMMANDS_H
#define COMMANDS_H

#include "solve.h"
#include "steps.h"

#define NCOMMANDS       20

void                    free_args(CommandArgs *args);
CommandArgs *           new_args();

extern Command *        commands[NCOMMANDS];

#endif
