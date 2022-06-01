# TODO list

This is a list of things that I would like to add or change at some point.
It's more of a personal reminder than anything else.

## For version 2.1
### Moving coordinates
* parallelize genptable_fixnasty
* general cleanup
### Changes to Step and Solve
* add a list of "helper" coordinates to every step
* Probably nicer: instead of passing a cube pass a structure "cube description"
  which can contain coordinates and one or more cubes.
  This can also be used to avoid using index_epud too much in drfin
  (it is slow because it goes through cubearray).
* add a step->move(Cube) function, which may apply moves to the cube or just to
  the coordinates
* optimal solver: move the coordinates for the 3 orientations, but also the
  cube so we can check the inverse
### Tables management
* Check files in tables directory, add command to remove old / extraneous files
* Add checksum to check that tables are generated / downloaded correctly
### Documentation
* Fix README.md with new coordinate system
* Write an examples.md file
* More screenshots!
### More
* Anything quick and easy from the sections below

## Refactor
### Coordinates
* Text (README.md) description of coordinate system with 3 (or 4) types of
  coordinates: basic (+ fundamental), sym, composite (consisting of at most
  one sym + one or more basic)
* Use this to restructure the coordinate part; maybe fundamental coordinates
  do not need to exist???
* Add also "transform" for every coordinate. For example, for EO and similar
  only allow transformations that fix the EO axis.
* Also: "basic" symcoord do not allow trans, composite coordinates assume
  the transformation fixes the basic sumcoord
* For each coordinate, manually disallow "bad" moves, or just ignore the error
  (probably better to check: low performance cost, detect problems that I might
  be overlooking)
### Loading at startup vs dynamically
* Consider moving more things to the initial loading phase (i.e. remove
  many of the "initialized" parts)


## Commands

### Commands that are available in nissy 1.0, but not in this version (yet):
* drcorners (solve corners after dr)
* search and improve non-optimal subsequences
* save and edit algs as "variables"
  (or just use a "logging system" to keep info about previously run commands,
including e.g. solutions that were not shown because -c)

### More steps for `solve`
* QTM optimal solving
* 5-side solve (for robots)
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
* input directly cube status instead of moves
  (graphical: maybe there is a cubing.js function; command line: ???)

## Distribution
* webapp (cgi)

## Technical stuff

### Testing
* write some proper tests, move test_coord to the testing module(s)

### Memory management
* free pruning table after solve is done? if I do this I need to deafault to a
  small table for < 8 moves solutions or smth
* improve multi-threading when solving multiple scrambles
* nissy -M maxmem option for running with at most maxmem memory; if exceeded
  when loading a pruning table, return failure (or make every solve command
  use tiny tables instead?); if maxmem is very 600Mb or
  less do not use invtables (the performance loss is minimal anyway). If the
  limit is really tiny, do not use mtables or ttables (but this would be
  very slow and probably nobody will ever use it)
* Check if memory is enough for loading pruning tables; if not, abort
* For optimal solver: choose largest that fits in memory between nxopt and light

### Structural changes
* client/server architecture: run a server process in the background so that
  multiple client processess can send it queries and get results; this would
  open up the door for a web-based version or graphical clients

### Cleanup
* sort again functions alphabetically in their files
* more stuff to load at start (or when suitable command is called) rather
  than when called directly, to avoid nasty problems with threading
* parse command args: one function per arg type, then each command has
  a list of options that it accepts (as a string)
