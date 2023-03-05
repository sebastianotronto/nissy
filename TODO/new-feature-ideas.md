# Possible new features and improvements

This file contains non-refined ideas. Once an idea gets refined, it will
get its own file and more details.

## Steps

* QTM solver
* 5-side solver (for robots)
* Other steps (cross, blocks, LSE...)

## UX features

* Save algs as variables and edit them (like in old nissy)
* Use a logging system for previously run commands, info, results...
  (e.g. when solving with -c solutions are not shown, they can be logged here)
* Configurability: add an "alias" command, run config file at startup
* Input cube state directly instead of moves (ugly from command line / file)

## Improvements

* Optimal solver: when asking only for one solution, scan for upper bound in
  parallel using a non-optimal (but fast) solver (e.g. twophase).
* Optimal solver: up to a small bound, try with a small pruning table.
* Optimal solver: start at different depths in parallel
* Multi-step solver: make more general

## New features

* Allow user to specify moveset manually (see issue \#5 on github)
* EO analysis (and also DR and HTR analysis): group similar EOs (Jay)
* HTR "maze" analysis?
