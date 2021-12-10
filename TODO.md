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
* "slow" optimal solver, using drud table but all the tricks
  of khuge (+ trick to avoid 180° moves when one of the inverse
  probes returns exactly the target value)
* QTM optimal solving
* Block-building steps (cross, roux blocks, ...)
* Other common steps (LSE, ...)

### Improvements to currently implemented commands
* solve should re-orient first if needed and not just give up if centers are off
* solve should try up to a small bound without loading the large pruning table
* drfin for HTR scrambles should try all 3 axis and pick the best solutions;
  in general every step that automatically detects orientation should do this

### New features
* cleanup: translate an alg to the standard HTM moveset + reorient at the end
* batch mode: read list of commands from stdin or a file and exec them
  one after the other non-interactively
* configurability: add an `alias` command, run config file at startup

## Distribution

* README.md: cite nxopt
* Add EXAMPLES.md file
* better man page
* webapp (cgi)

## Technical stuff

## Performance (optimal solver)
* Khuge optimal solver: change direction of search when doing so leads to
less branching (like nxopt). Need to add some info to EstimateData or to
DfsData (like last moves on inverse/other scramble) and to change some of
the logic of niss (allow for switching multiple times).
* Light optimal solver: use drud table instead of khuge, with tricks as above
and one more trick: if the last move is 180° avoid computing inverse cube
and just use previous values for all 3 axes.

## Coordinates, symmetries, pruning tables
* Cleanup symcoord.c: some coordinates and symdata are never actually used;
remove also sd_eofbepos and just use sd_coud for khuge (this changes the
coordinate so the whole table must be generated again!) or viceversa
* Use pruning values mod 4 instead of mod 16 (or maybe not, I like the
current system)

### Structural changes
* client/server architecture: run a server process in the background so that
  multiple client processess can send it queries and get results; this would
  open up the door for a web-based version or graphical clients
