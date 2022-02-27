# TODO list

This is a list of things that I would like to add or change at some point.
It's more of a personal reminder than anything else.

## For version 2.1
### Scrambles
* dr and htr scrambles
### Memory management
* Free large tables from memory before exit (this is not strictly necessary,
  but can help with WSL bugs)
### Installation
* At startup, check if tables are all there; if not, print an error message
  suggesting to run `nissy gen -t 8` (or more than 8). Is it
  possible to make table generation at least 3x faster?
* make 8 threads default for gen?
### Documentation
* Write an examples.md file
* More screenshots!
### More
* Anything quick and easy from the sections below

## Commands

### Commands that are available in nissy 1.0, but not in this version (yet):
* drcorners (solve corners after dr)
* search and improve non-optimal subsequences
* save and edit algs as "variables"
  (or just use a "logging system" to keep info about previously run commands,
including e.g. solutions that were not shown because -c)

### More steps for `solve`
* QTM optimal solving
* Block-building steps (cross, roux blocks, ...)
* Other common steps (LSE, ...)

### Improvements to currently implemented commands
* solve should re-orient first if needed and not just give up if centers are off
* solve should try up to a small bound without loading the large pruning table
* silent batch mode without >>>

### New features
* configurability: add an `alias` command, run config file at startup
* configure max ram to be used (via config file and/or command line option)
* transform alg, rufify etc...
* command notation to list available moves
* make multi-step solve much more general and create command

## Distribution

* Add EXAMPLES.md file
* webapp (cgi)
* installation: get ptables with curl or similar (on Windows what?)
  also, keep only one compressed format (+uncompressed?) on server

## Technical stuff

### Memory management
* free pruning table after solve is done? if I do this I need to deafault to a
  small table for < 8 moves solutions or smth
* improve multi-threading when solving multiple scrambles
* alternative: just add a command "free" to free up memory; it is not
  user friendly (who wants to manage memory manually?) but on the other hand
  it will only be used by the few who have less than 4(?) Gb of ram.
* nissy -M maxmem option for running with at most maxmem memory; if exceeded
  when loading a pruning table, return failure (or make every solve command
  use tiny tables instead?); if maxmem is very 600Mb or
  less do not use invtables (the performance loss is minimal anyway). If the
  limit is really tiny, do not use mtables or ttables (but this would be
  very slow and probably nobody will ever use it)
* Check if memory is enough for loading pruning tables; if not, abort
* For optimal solver: choose largest that fits in memory between nxopt and light

### Other optimal solvers
* try htr corners + edges in slice but not oriented (300Mb table);
  de Bondt's trick does not work, but I can use full symmetry and
  take advantage of the fact that it is a subset invariant under half-turns
  (like in light optimal solver)
* Another idea: DR + cornershtr (5Gb table); same as above, de Bondt's trick
  does not work but I can use half-turn trick

### Structural changes
* client/server architecture: run a server process in the background so that
  multiple client processess can send it queries and get results; this would
  open up the door for a web-based version or graphical clients

### Cleanup
* Remove khuge from everywhere
* Arrays (commands, steps...): end with NULL and remove size constant
* sort again functions alphabetically in their files
* more stuff to load at start (or when suitable command is called) rather
  than when called directly, to avoid nasty problems with threading
* parse command args: one function per arg type, then each command has
  a list of options that it accepts (as a string)
