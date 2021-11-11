# Nissy

A Rubik's cube solver and FMC assistant. For optimal HTM solving nissy is about as
fast as Herbert Kociemba's [Cube Explorer](http://kociemba.org/cube.htm), and it
uses the same method. Nissy can also solve many different substeps of
Thistlethwaite's algorithm (DR/HTR), and can use NISS (Normal-Inverse Scramble Switch).

It can be useful to analyze your DR solves (and more, once I implement more features).

## Why should I use nissy?

You should use nissy if you:
* Want to analyze your DR solutions or check for multiple optimal (or sub-optimal)
solutions for EO/DR/HTR or similar substeps.
* You just want a Rubik's cube solver and you like command line interfaces.
* You want an alternative to Cube Explorer.

## Requirements

A full installation of nissy requires about 1.8Gb of space, of which 1.6Gb are
occupied by the huge pruning table for optimal solving, and running it requires
the same amount of RAM.
One can choose to never use the optimal solver and not to install the relative
pruning table. If so, about 200Mb should be enough.

## Installation

### On a UNIX system:

You can download the source code for the latest version from git or simply clone
the repo with `git clone https://github.com/sebastianotronto/nissy`.

Edit the Makefile to match your local configuration (usually not necessary, but you
may want to change the `PREFIX` variable) and run `make`, followed by `make install`.
After that, you need to download some files and manually save them to your `NISSYDATA`
folder (see manual page, ENVIRONMENT section). If you don't do it nissy can compute
these files when needed and save them in the correct folders, but some of them
take hours to generate even for a powerful computer (for reference, the huge pruning
table takes about 20 hours on my laptop).

Choose one of the following:

| |.zip|.tar.gz|
|-|-|-|
|Full (~720Mb)|[full.zip](https://math.uni.lu/tronto/nissy/nissy-tables-full.zip)|[full.tar.gz](https://math.uni.lu/tronto/nissy/nissy-tables-full.tar.gz)|
|No huge table (~90Mb)|[nohuge.zip](https://math.uni.lu/tronto/nissy/nissy-tables-nohuge.zip)|[nohuge.tar.gz](https://math.uni.lu/tronto/nissy/nissy-tables-nohuge.tar.gz)|

extract the archive and copy the `tables` folder into `NISSIDATA` (paste there
the whole folder, not the single files). The `nohuge` files are much smaller and
do not contain the huge pruning table for the optimal solver.

### On Windows

Coming soon.

## Structure of the code

You can find all the source code in the `src` folder.
I strived to keep it legible but I did not write many comments (barely any at all).
I'll try to explain here the main parts of the program.

### Cube, moves and transformations

There are many ways to represent a cube. In nissy I use two:
* An array representation `CubeArray`: 3 arrays representing the permutation
of corners, edges and centers and 2 arrays for the orientation of corners and edges.
* An 11-integers representation `Cube`: 3 integers for edge orientation (with respect
to the three axes), 3 for corner orientation, and so on. Edge permutation is a bit
complicated because encoding 12 factorial as a single number is too large for some
practical reasons, so I use 3 integers for that.

Moves are easy to apply on the array form, but they are slow. So `moves.c`
contains the instructions to create all the transition tables necessary
to get the next position for the cube with just 11 lookup operations
(one for each of the 11 integers in the second representation).
These transition tables are saved in the `mtables` file in the
`tables` folder in binary format.

The 11 integers are obviously redundant, but keeping all of them makes it easy
to apply transformations. A transformation is a rotation of the whole cube, possibly
combined with a mirror operation. Applying a transformation to a cube (say obtained
by applying a scramble to the solved cube) means applying the transformation to a
solved cube, then the scramble and then the inverse of the transformation
(i.e. conjugating by it).

### Coordinates and pruning tables

A *coordinate* consists of a function that takes a cube (in the 11-integer
representation) and return an (unsigned, 64-bit) integer. They are used
to "linearize" a cube and build pruning tables, which speed up significantly the
solving process. To be able to access the pruning table quickly, the function
needs to be very fast (e.g. it should not convert between the two representations
of the cube if not necessary).

Some coordinates make use of symmetries to reduce the size of the resulting
pruning table. Unfortunately this complicates the code a lot, but it is a huge
advantage: it reduces by a factor of about 16 the huge pruning table, which
results in around 1.6Gb instead of 24 or so.

Pruning tables are related to a specific step, a moveset and a coordinate. They
contain one value from 0 to 15 (4 bits) for each possible value for the coordinate,
which is less or equal than the minimum number of moves required to solve the
given step with the given moveset for a cube which has the given coordinate. For example,
say the coordinate `neo` gives the number of non-oriented edges (say with respect to
F/B). Then the possible values for the coordinate are 0,2,4,...,12. An associate pruning
table to solving EO with HTM moveset and this coordinate would have values 0 (for
`neo=0`), 3 (for `neo=2`), 1 (for `neo=4`)...

There is one caveat: each coordinates also needs an inverse function that takes a
coordinate value and returns a cube which has that coordinate. This is in general
more complicated, but luckily the cube does not need to be fully built or consistent.
This inverse-coordinate is used only in one specific step when building pruning tables
to avoid using up hundreds of Gb of memory.

Note: this part is different from what Cube Explorer does. Overall I think it is
conceptually easier, although in practice it was still hard to implement.
If anything it is more generalizable and one can use it to build any coordinate
they might like.

### Solving

Solving is implemented as a generic function that takes both a step and
a (scrambled) cube as input, as well as some extra parameters that say e.g.
how many solution one wants. A step consists, among other things, of
an estimator function that, given a cube, gives a lower bound for the number
of moves needed to complete the step. Many of these estimators simply
look up the corresponding values in the appropriate pruning table.

