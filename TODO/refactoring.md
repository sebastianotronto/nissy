# Refactoring

## Init functions

* All .h files should have a single init function.
* This function should initialize everything that this module needs, including
  calling the init functions of the modules it depends on.
* To avoid multiple initialization of the same module, each should have a
  static bool initialized variable.
* Everything that a module needs should be initialized by init(), avoid
  initializing stuff when solving. Exception: pruning tables, move tables.
* Most functions should generate some tables and save them to disk.
* Init functions should have a consistent structure (e.g. the way they check
  if the tables are already generated should be the same).

## Cube types

* Get rid of cubetype.h, split type definitionss into the other modules.
* Every type definition should be in the most fundamental module that needs it.

## Code style

* Stop declaring all variables at the beginning of a function.
* Remove variable names from prototypes.
* Sort function implementations alphabetically, ignore static vs non static.
* Rename functions and variable to have a consistent naming scheme.
* Functions that copy data: swap src and dest, follow memcpy standard.
* The way coord uses define guards to organize the .h file is good, apply it
  to other modules too - including tests.
* Read style(9) and decide what to implement.
