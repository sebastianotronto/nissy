# Build options for memory and multithreading

## Investigate

* Check exactly how much memory is needed for everything.
* Take note of which parts use threading (solving, genptable, other?).

## Prepare code

* Use define / ifdef or similar to compile and build tables only for the
  parts to be used.
* If threads = 1, use a much simpler version of the solve method. Remember
  that checking if enough solutions have been found is the first thing to
  do in singlethread (no locking).
* Do not include pthread if threads = 1. 
* Only one optimal solver should be compiled.
* Some simple steps may also need alternatives with smaller tables
  (e.g. for staying sub 1Gb). For example dr and drfin.
* If necessary, work out alternatives to "twophase" for low-resource versions.

## Makefile

* Figure out how to change these options via makefile. For example: one
  variable for the maximum allowed ram and one for the number of threads.
* (Optional) use a configure script?
* (Optional) interactive installation script?

## Automate

* Scout for resources during installation and choose best configuration
  automatically.
* How to do this in Linux / POSIX?
* How to do this in Windows?
