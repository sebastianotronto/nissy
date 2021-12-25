# TODO list

This is a list of things that I would like to add or change at some point.
It's more of a personal reminder than anything else.

**Things in bold: to do before 2.0 release**

## Commands

### Commands that are available in nissy 1.0, but not in this version (yet):
* drcorners (solve corners after dr)
* search and improve non-optimal subsequences
* **fast non-optimal solver (also needed for scramble)**
* **scramble [dr, corners only, edges only, htr, fmc(RUF)...]**
* save and edit algs as "variables"
  (or just use a "logging system" to keep info about previously run commands,
including e.g. solutions that were not shown because -c)

### More steps for `solve`
* QTM optimal solving
* Block-building steps (cross, roux blocks, ...)
* Other common steps (LSE, ...)

### Improvements to currently implemented commands
* **solve should re-orient first if needed and not just give up if centers are off**
* solve should try up to a small bound without loading the large pruning table

### New features
* cleanup: translate an alg to the standard HTM moveset + reorient at the end
* configurability: add an `alias` command, run config file at startup
* configure max ram to be used (via config file and/or command line option)
* transform alg, rufify etc...
* more scramble stuff (scramble FMC with rufify...)
* command notation to list available moves

## Distribution

* Add EXAMPLES.md file
* webapp (cgi)
* **Re-upload tables**
* **fix README.md**
* **fix examples in manpage**

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
* unniss and inverse_alg work differently (one in place, the other makes
  a copy and returns) changing inverse_alg seems the best option.
