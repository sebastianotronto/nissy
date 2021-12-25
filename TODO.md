# TODO list

This is a list of things that I would like to add or change at some point.
It's more of a personal reminder than anything else.

## Commands

### Commands that are available in nissy 1.0, but not in this version (yet):
* drcorners (solve corners after dr)
* search and improve non-optimal subsequences
* unniss (rewrite A (B) -> B' A)
* scramble [dr, corners only, edges only, htr, ...]
* save and edit algs as "variables"
* invert an alg

### More steps for `solve`
* QTM optimal solving (important: fix possible_next, which works only for HTM now)
* Block-building steps (cross, roux blocks, ...)
* Other common steps (LSE, ...)

### Improvements to currently implemented commands
* solve should re-orient first if needed and not just give up if centers are off
* solve should try up to a small bound without loading the large pruning table
* drfin for HTR scrambles should try all 3 axis and pick the best solutions;
  in general every step that automatically detects orientation should do this
* for solve -v, solving in different orientation does not give meaningful info,
  because I need to transform the alg as I go.
* for solve -v, print certain info like average branching value
* solve -O n find solutions within n moves from optimal
  (-o is the same as -O 0)

### New features
* cleanup: translate an alg to the standard HTM moveset + reorient at the end
* configurability: add an `alias` command, run config file at startup
* configure max ram to be used (via config file and/or command line option)
* command to transform cube and alg
* command notation to list available moves

## Distribution

* Add EXAMPLES.md file
* webapp (cgi)
* Re-upload tables, fix README.md

## Technical stuff

### Memory management
* free pruning table after solve is done? if so, I need to add another way
  of doing batch solving (I don't want to re-load the tables every time);
  for example I could add the possibility of reading scrambles from file,
  and execute the same solve command to every line; also improve multi-threading:
  I can just solve one scramble per thread, it's better because there is no lock.
* alternative: just add a command "free" to free up memory; it is not
  user friendly (who wants to manage memory manually?) but on the other hand
  it will only be used by the few who have less than 4(?) Gb of ram.
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
* sort again functions alphabetically in their files
* more stuff to load at start (or when suitable command is called) rather
  than when called directly, to avoid nasty problems with threading
