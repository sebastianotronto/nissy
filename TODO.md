# TODO list

This is a list of things that I would like to add or change at some point.
It's more of a personal reminder than anything else.

## After symcoord
### Solving standard coordinates
* add Void * extradata to DfsArg and a custom move function
* add optional custom pre-process for generating special table (nx)
* copy_dfsdata should copy extra too!
### nx.c
* implement nxopt with all tables and all tricks
  (maybe compile time variable for maximum memory to use?)
* is_valid should also unniss / cleanup the alg
### fst_cube
* slightly different from cube in v2.0.2: each "side" coordinate
  is a transformation of the other, not an eorl or similar (changes
  the permutation!)
* add fst_index for some coordinates?
* inverse: for edges, just generate ep[12] and convert back
* corners: big table (150Mb if 16bit integers are used)

## For version 2.1
### Changes to Step and Solve
* remove cube from dfsarg? (i still need to save the scramble somewhere,
  but I really only use it in dfs_niss)
* coord.c: all old coordinates (WIP...)
* steps.c: checkers (use coordinates), all stepalt and steps (WIP...)
* commands gen and freemem
* commands.c: twophase, ...?
### Rotate, not transform, before solving
* solve should re-orient first if needed and not just give up if centers are off
### Documentation
* Document how coordinates and pruning tables work now
* Write an examples.md file
* More screenshots!
### Tables management
* Check files in tables directory automatically remove old / extraneous files
* Add checksum to check that tables are generated / downloaded correctly
### Conditional compiling
* Option to avoid large tables at compile time
* option to avoid multithreading (write a simpler solve for t=1, and also
  check if found enough solutions before checking pruning values)
### Technical
* generic option parser
* testing? Maybe just hardcode some examples generated with old nissy
### Commands
* Easy: add option -I (inverse) and -L (linear, like inverse + normal)
  to do only linear NISS

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
* Larger table for drudfin (include epe)? About 1Gb uncompressed,
  500Mb compressed (fallback to noE), 250 compressed + parity trick
  (is it doable?)

### Improvements to currently implemented commands
* solve multidfs: do multithread by step, not by alternative (this way
  if there are multiple alternatives it can make use of more threads)
* solve should try up to a small bound without loading the large pruning table
  (maybe this is not necessary if loading the table is fast enough)
* silent batch mode without >>>

### New features
* EO analysis (and also DR and HTR analysis): group similar EOs together
  and such (suggested by Jay)
* configurability: add an `alias` command, run config file at startup
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
* change some function and variable names to make everything consistent
* more stuff to load at start (or when suitable command is called) rather
  than when called directly, to avoid nasty problems with threading
* parse command args: one function per arg type, then each command has
  a list of options that it accepts (as a string)

### Style
* do not declare all variables at the beginning of a function
* remove var names from prototypes
* various stuff from style(9)

### Random
Collect random info like this somewhere:

Table pt_nxopt31_HTM
Base value: 9
0               1
1               6
2              29
3             164
4            1433
5           16772
6          205033
7         2513871
8        30329976
9       342440769
10     2815191126
11     6147967200
12      524918774
13           3546
14              0
15              0
