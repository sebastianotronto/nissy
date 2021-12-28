# Nissy

A Rubik's cube solver and FMC assistant.
For optimal HTM solving Nissy uses techniquest from Herbert Kociemba's
[Cube Explorer](http://kociemba.org/cube.htm) and Tomas Rokicki's
[nxopt](https://github.com/rokicki/cube20src/blob/master/nxopt.md).
With 4 cores at 2.5GHz and using less than 3Gb of RAM, Nissy can find an
optimal solution in about a minute on average.

Nissy can also solve many different substeps of Thistlethwaite's algorithm
(DR/HTR), and can use NISS (Normal-Inverse Scramble Switch).
It can be useful to analyze your DR solves (and more, once I implement more features).

## Why should I use Nissy?

You should use Nissy if you:
* Want to analyze your DR solutions or check for multiple optimal (or sub-optimal)
solutions for EO/DR/HTR or similar substeps.
* You just want a Rubik's cube solver and you like command line interfaces.
* You want an alternative to Cube Explorer.

## Requirements

A full installation of nissy requires about 3Gb of space,
of which 2.3Gb are occupied by the huge pruning table for fast optimal solving,
and running it requires the same amount of RAM.
One can choose to never use this function and not to install the relative
pruning table. There is an alternative (slower)
optimal solving function that uses about 500Mb of RAM.

## Installation

### On Windows

Try downloading and executing in a terminal the file nissy.exe, then
follow the instructions in the **Tables** section below for
installing the pruning tables.
If nissy.exe does not work, you can try following the UNIX instructions
in WSL (Windows Subsystem for Linux) or in a similar environment.

Sorry for the inconvenience, I don't have a Windows machine to test this on.

### On a UNIX system:

Edit the Makefile to match your local configuration (usually not necessary, but you
may want to change the `PREFIX` variable) and run `make`, followed by `make install`.
Follows the instructions below to install the pruning tables.

### Tables
Nissy needs to generate certain large tables to work. These tables are by default
generated the first time they are needed (e.g the first time you ask to solve a
certain step) and then saved to a file. Whenever these tables are needed again,
nissy simply loads the corresponding file from the hard disk.

The very large table for optimal solving can take some time to generate
(about 1.5 hours on my fairly old but decent laptop, using 8 CPU threads).
In order to generate it you need at least 5Gb or RAM.
All other tables are much faster.

You can ask Nissy to generate all the tables it will ever need with the `gen`
command. It is recommended to use more than one thread, if your CPU has them.
For example, you can run:

```
nissy gen -t 8
```

to generate all tables using 8 threads.

Alternatively, you can simply download all the tables and copy them into the
correct folder (see manual page, `ENVIRONMENT` section). On UNIX operating
systems this folder is either `.nissy/tables` in the user's home directory or
`$XDG_DATA_HOME/nissy/tables` if the XDG variable is configured. On Windows
it is the same directory as the nissy.exe executable file.

Choose either the
[zip](https://math.uni.lu/tronto/nissy/nissy-tables-2.0.zip)
or the
[tar.gz](https://math.uni.lu/tronto/nissy/nissy-tables-2.0.tar.gz)
file (click the links to download) and
extract them in the correct folder.

## Structure of the code

You can find all the source code in the `src` folder.
I strived to keep it legible but I did not write many comments (barely any at all).
I'll try to explain here the main parts of the program.

### Cube, moves and transformations

There are many ways to represent a cube. In Nissy I use two:
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
advantage: it reduces by a factor of about 16 the pruning table size.

Pruning tables are related to a specific step, a moveset and a coordinate. They
contain one value from 0 to 15 (4 bits) for each possible value for the coordinate,
which is less or equal than the minimum number of moves required to solve the
given step with the given moveset for a cube which has the given coordinate. For example,
say the coordinate `neo` gives the number of non-oriented edges (say with respect to
F/B). Then the possible values for the coordinate are 0,2,4,...,12. An associated
pruning table to solving EO with HTM moveset and this coordinate would have values 0
(for `neo=0`), 3 (for `neo=2`), 1 (for `neo=4`)...

The values for most pruning tables are memorized modulo 16, so they only occupy
4 bits per entry, and values larger than 15 are saved as 15. This is good enough
for most applications.
Some large tables are memorized in compact form using only 2 bits, similarly
to what [nxopt](https://github.com/rokicki/cube20src/blob/master/nxopt.md) does:
a base value `b` is picked and a value of `n` is saved as `MIN(3,MAX(0,n-b))`.
When a value of `v=1,2,3` is read it is simply returned as `v+b`, while if
`0` is a successive lookup to a fallback table is performed. The base value `b`
is picked to maximize the sum frequency of the values `1,2,3`.

There is one caveat: each coordinates also needs an inverse function that takes a
coordinate value and returns a cube which has that coordinate. This is in general
more complicated, but luckily the cube does not need to be fully built or consistent.
This inverse-coordinate is used only in one specific step when generating symmetry
data, and I don't know if it is possible to avoid it (maybe it is). It is also used
when building pruning tables, but in that case it is avoidable.

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

