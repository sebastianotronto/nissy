# TODO list

This is a list of things that I would like to add or change at some point.
It's more of a personal reminder than anything else.

## Bugs
* Segfault on "nissy solve -s 10U" (no space between 10 and U)

## Commands

### Commands that are available in nissy 1.0, but not in this version (yet):
* drcorners (solve corners after dr)
* search and improve non-optimal subsequences
* unniss (rewrite A (B) -> B' A)
* scramble [dr, corners only, edges only, htr, ...]
* save and edit algs as "variables"
* invert an alg

### More steps for `solve`
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

* make env.c compatible with Windows (and check that it works with
  BSD/MacOS)
* default to current directory for tables; this will work on any OS, up to
  using the correct #ifdef guards to avoid checking for posix directories
  in non-posix systems
* better man page
* find a better way to distribute the large tables, especially khuge

## Technical stuff

### Better pruning tables
* Use pruning values mod 4 instead of mod 16

### Memory management
* fail gracefully when there is not enough memory to load a large table
* free tables from memory when not used
* optionally run in low-memory friendly version (no tables above a few Mb);
  this can be useful e.g. for embedded devices

### Structural changes
* client/server architecture: run a server process in the background so that
  multiple client processess can send it queries and get results; this would
  open up the door for a web-based version or graphical clients
* use multiple threads to search for solutions in parallel
