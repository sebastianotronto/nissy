/* To generate this help page, use the script makedoc.sh */

int Npages = 21;

char *helppages[][10] = {

{ "add",
"\
\n\
HELP PAGE FOR COMMAND add\n\
\n\
SYNTAX\n\
add [MOVES|$ID1|@ID1] $ID2\n\
\n\
DESCRIPTION\n\
Appends either MOVES, the scramble memorized under $ID1 or the output sequence\n\
memorized under @ID1 at the end of the scramble memorized under $ID2. If none\n\
of MOVES, $ID1 or @ID1 is specified, the user will be asked to type the moves.\n\
Menmonic: \"add x to y\" or just \"add to y\".\n\
\n\
EXAMPLES\n\
add $1\n\
  The user is required to type the moves that will be appended to $1.\n\
add F R B $1\n\
  Appends the moves F R B to scramble $1. Now scramble $1 ends with F R B.\n\
\n\
"
},

{ "change",
"\
\n\
HELP PAGE FOR COMMAND change\n\
\n\
SYNTAX\n\
change $ID1 [MOVES|$ID2|@ID2]\n\
\n\
DESCRIPTION\n\
Changes the scramble $ID1 to either MOVES, the scramble $ID2, the output @ID2\n\
or, if none is specified, the moves entered by the user. The scramble that was\n\
memorized under $ID1 is then lost.\n\
Mnemonic: \"change x to y\", or just \"change x\".\n\
\n\
EXAMPLES\n\
change $1\n\
  The user is required to type the moves that will replace $1.\n\
change $2 $3\n\
  Saves the scramble that was saved under $3 in $2. Now $2 and $3 are the same\n\
  scrambles, and the old $2 is lost.\n\
change $1 U R\n\
  Saves U R as scramble $1.\n\
\n\
"
},

{ "clear",
"\
\n\
HELP PAGE FOR COMMAND clear\n\
\n\
SYNTAX\n\
clear\n\
\n\
DESCRIPTION\n\
Resets all saved scrambles and output sequences.\n\
\n\
"
},

{ "dr",
"\
\n\
HELP PAGE FOR COMMAND dr\n\
\n\
SYNTAX\n\
dr [OPTIONS] [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Solves DR for a given scramble. A scramble can be given as last argument of the\n\
command, or an ID of a saved scramble can be provided. If none of the two is\n\
given, a prompt will ask the user to input a new scramble.\n\
If the option \"from\" is given (see below), it solves DR from an EO (if edges\n\
are oriented) without breaking that EO.\n\
The first time this command is called without the option from (and, to some\n\
extent, also the first time it is called with the option from), nissy loads\n\
some pruning tables that were not loaded on startup, causing a small but\n\
noticeable delay.\n\
\n\
OPTIONS\n\
axis={fb,rl,ud}    Specify the axis for the DR. One to three axes can be given,\n\
                   comma separated, no spaces.\n\
                   Default: DR on any of the three axis (omitting the option is\n\
                   the same as specifying axis=fb,rl,ud).\n\
b=N                Specify a bound for the number of moves. N must be a number.\n\
                   Default value: 20.\n\
h                  Show hidden DRs.\n\
                   Default, if an DR ending in e.g. R is shown, the equivalent\n\
                   one ending in R' is hidden.\n\
from {fb|rl|ud}    Solve DR from the specified EO, which must be solved,\n\
                   without breaking the EO.\n\
niss               Use NISS. It works only if solving DR from EO.\n\
                   Default: does not use NISS.\n\
n=N                Specify a maximum number of EOs to be output. N must be a\n\
                   number.\n\
                   Default value: 1.\n\
\n\
EXAMPLES\n\
dr from rl axis=ud $1\n\
  Finds optimal DR on ud, starting from EO on rl, for the first saved scramble.\n\
\n\
dr niss from fb n=10 m=6 F2 R L B' F D U' R2 L' F D B\n\
  Finds up to 10 DRs of length at most 6 from EO on fb, possibly using NISS.\n\
\n\
dr n=100 axis=ud h\n\
  Finds 100 DRs on ud, including \"hidden\" DRs.\n\
\n\
"
},

{ "drcorners",
"\
\n\
HELP PAGE FOR COMMAND drcorners\n\
\n\
SYNTAX\n\
drcorners [OPTIONS] [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Similar to drfinish, but only solves corners. CO must be solved. The scramble\n\
can be given as the last argument of the command, or it may be given as an $ID\n\
or @ID, or it can be typed out on the following line.\n\
\n\
OPTIONS\n\
from {ud|fb|rl}    Allows to specify on which axis the CO is solved. It is\n\
                   usually not necessary, since nissy will find a CO on any\n\
                   axis.\n\
i                  Ignores E-layer centers. By default cornersare solved\n\
                   relatively to centers; this options allows for solutions\n\
                   which solve corners relatively to each other and to the\n\
                   U and D sides, but not to the E layer (or any equivalent\n\
                   if the CO is not on U/D).\n\
b=N                Specify a bound for the number of moves. N must be a number.\n\
                   Default value: 20.\n\
n=N                Specify a maximum number of solutions to be output. N must\n\
                   be a number.\n\
                   Default value: 1.\n\
\n\
EXAMPLES\n\
drcorners n=3 R' D R2 D' R' U2 R D R' U2 R' D' R\n\
  Produces the following output:\n\
Found 3 results.\n\
@1:	U' F2 U R2 U2 F2 U F2 U R2 (10)\n\
@2:	U' F2 U R2 U2 B2 U R2 D R2 (10)\n\
@3:	U' F2 U R2 U2 B2 U L2 U L2 (10)\n\
\n\
"
},

{ "drfinish",
"\
\n\
HELP PAGE FOR COMMAND drfinish\n\
\n\
SYNTAX\n\
drfinish [OPTIONS] [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Solves the given scramble using the DR moveset. DR must be solved. The scramble\n\
can be given as the last argument of the command, or it may be given as an $ID\n\
or @ID, or it can be typed out on the following line.\n\
\n\
OPTIONS\n\
from {ud|fb|rl}    Allows to specify on which axis the DR is solved. It is\n\
                   usually not necessary, since nissy will find a DR on any\n\
                   axis, but it can be useful if one want to e.g. solve an HTR\n\
                   state allowing quarter-turns from a specific DR.\n\
b=N                Specify a bound for the number of moves. N must be a number.\n\
                   Default value: 20.\n\
n=N                Specify a maximum number of solutions to be output. N must\n\
                   be a number.\n\
                   Default value: 1.\n\
\n\
EXAMPLES\n\
dr from ud R L' U2 R' L F2\n\
  Solves the given scramble using the moveset <U,D,R2,L2,F2,B2>. In this case:\n\
@1:	U2 R2 F2 R2 U2 R2 F2 R2 (8)\n\
drfinish b=7 n=10 $1\n\
  Finds (at most) 10 solutions of length at most 7 for the scramble $1.\n\
\n\
"
},

{ "eo",
"\
\n\
HELP PAGE FOR COMMAND eo\n\
\n\
SYNTAX\n\
eo [OPTIONS] [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Solves EO for a given scramble. A scramble can be given as last argument of the\n\
command, or an ID of a saved scramble can be provided. If none of the two is\n\
given, a prompt will ask the user to input a new scramble.\n\
\n\
OPTIONS\n\
axis={fb,rl,ud}    Specify the axis for the EO. One to three axes can be given,\n\
                   comma separated, no spaces.\n\
                   Default: EO on any of the three axis (omitting the option is\n\
                   the same as specifying axis=fb,rl,ud).\n\
b=N                Specify a bound for the number of moves. N must be a number.\n\
                   Default value: 20.\n\
h                  Show hidden EOs.\n\
                   Default, if an EO ending in e.g. F is shown, the equivalent\n\
                   one ending in F' is hidden.\n\
niss               Use NISS.\n\
                   Default: does not use NISS.\n\
n=N                Specify a maximum number of EOs to be output. N must be a\n\
                   number.\n\
                   Default value: 1.\n\
\n\
EXAMPLES\n\
eo axis=fb $1\n\
  Finds one optimal EO on fb for the first saved scramble.\n\
\n\
eo n=5 b=4 U R F  \n\
  Finds up to 5 EOs of length at most 4 for scramble U R F.\n\
\n\
eo n=100 b=5 niss axis=fb,ud h  R' U' F L R'U'F\n\
  Finds up to 100 EOs of lenth at most 4, possibly using NISS, including\n\
  \"hidden\" EOs, excluding the rl axis.\n\
\n\
"
},

{ "exit",
"\
\n\
HELP PAGE FOR COMMAND exit\n\
\n\
SYNTAX\n\
exit\n\
\n\
DESCRIPTION\n\
Exits nissy.\n\
\n\
"
},

{ "help",
"\
\n\
HELP PAGE FOR COMMAND help\n\
\n\
SYNTAX\n\
help [nissy|COMMAND]\n\
\n\
DESCRIPTION\n\
'help nissy' prints a general user manual. 'help COMMAND' prints a detailed\n\
help page for the command COMMAND, if it exists. 'help' prints a list of all\n\
available commands a short description for each.\n\
\n\
EXAMPLES\n\
help help\n\
  Prints this help page.\n\
\n\
"
},

{ "htr",
"\
\n\
HELP PAGE FOR COMMAND htr\n\
\n\
SYNTAX\n\
htr [OPTIONS] [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Finds HTR for a given scramble. DR must be solved. A scramble can be given as\n\
last argument of the command, or an ID of a saved scramble can be provided. If\n\
none of the two is given, a prompt will ask the user to input a new scramble.\n\
\n\
OPTIONS\n\
from {ud|fb|rl}    Allows to specify on which axis the DR is. This is usually\n\
                   not needed, since nissy will automatically find it out.\n\
b=N                Specify a bound for the number of moves. N must be a number.\n\
                   Default value: 20.\n\
h                  Show hidden HTRs.\n\
                   Default, if an HTR ending in e.g. R is shown, the equivalent\n\
                   one ending in R' is hidden.\n\
niss               Use NISS.\n\
                   Default: does not use NISS.\n\
n=N                Specify a maximum number of HTRs to be output. N must be a\n\
                   number.\n\
                   Default value: 1.\n\
\n\
EXAMPLES\n\
eo n=10 b=7 niss $1\n\
  Finds up to 100 HTRs of lenth at most 7, possibly using NISS, including\n\
  \"hidden\" HTRs, for scramble $1. DR must be solved.\n\
\n\
"
},

{ "htrfinish",
"\
\n\
HELP PAGE FOR COMMAND htrfinish\n\
\n\
SYNTAX\n\
htrfinish [OPTIONS] [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Similar to drfinish, but uses the moveset <U2,D2,R2,L2,F2,B2>. HTR must be\n\
solved. The scramble can be given as the last argument of the command, or it\n\
may be given as an $ID or @ID, or it can be typed out on the following line.\n\
\n\
OPTIONS\n\
b=N                Specify a bound for the number of moves. N must be a number.\n\
                   Default value: 20.\n\
n=N                Specify a maximum number ofsolutions to be output. N must be\n\
                   a number.\n\
                   Default value: 1.\n\
\n\
EXAMPLES\n\
htrfinish R L' U2 R' L F2\n\
  Produces the following solution:\n\
@1:	U2 R2 F2 R2 U2 R2 F2 R2 (8)\n\
\n\
"
},

{ "invert",
"\
\n\
HELP PAGE FOR COMMAND invert\n\
\n\
SYNTAX\n\
invert [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Inverts a sequence of moves, which can be given also as $ID or @ID. The given\n\
sequence must not use NISS (if it does, use the command unniss first).\n\
\n\
EXAMPLES\n\
invert F R D'\n\
  Prints D R' F'\n\
\n\
"
},

{ "nissy",
"\
\n\
*******************************************************************************\n\
********************* NISSY: a cube solver and FMC helper *********************\n\
*******************************************************************************\n\
\n\
If you just want to solve the cube, type 'solve' followed by the scramble. This\n\
will not always give you an optimal solution, unless it is 10 moves or less or\n\
you use the \"o\" option. Finding the optimal solution might take very long if it\n\
is 16 moves or more, especially for the first time.\n\
\n\
Now the fun stuff. With nissy you can save and manipulate move sequences, for\n\
example:\n\
\n\
nissy-# save R' U' F\n\
$1:	R' U' F\n\
nissy-# add L2D' $1\n\
$1:	R' U' F L2 D'\n\
\n\
You can then ask nissy to solve certain substepson a saved scramble:\n\
\n\
nissy-# eo axis=rl $1\n\
@1:	U D F' R (4)\n\
\n\
And of course it uses also NISS, if you ask:\n\
\n\
nissy-# eo niss axis=rl $1\n\
@1:	(R) (1)\n\
\n\
Notice that the sequences you save are marked with a $, while the \"output\"\n\
sequences are marked with @. The difference between these two type of sequences\n\
is that those marked with @ are temporary and get lost once you get new output.\n\
Most commands accept as input either a move sequence typed out, a $-sequence or\n\
a @-sequence. For example, you can however save a @-sequence and make it\n\
persistent:\n\
\n\
nissy-# save @1\n\
$2:	(R)\n\
\n\
Nissy also understands NISS. Let's see a more complicated example where you\n\
save a scramble, ask for some EOs (using NISS) and then a DR on inverse:\n\
\n\
nissy-# save R' U' F R U R2 F2 R2 D R2 U L2 U R2 D2 B' D U' R D R' D U2 F2 R' U' F\n\
$3:	R' U' F R U R2 F2 R2 D R2 U L2 U R2 D2 B' D U' R D R' D U2 F2 R' U' F \n\
nissy-# eo n=10 niss axis=fb,rl $3\n\
Found 10 results.\n\
@1:	(U' L B D F) (5)\n\
@2:	(U' L B' D F) (5)\n\
@3:	(L B U D F) (5)\n\
@4:	(L B' U D F) (5)\n\
@5:	R U B U L (5)\n\
@6:	R U' L (B L) (5)\n\
@7:	R U' B U L (5)\n\
@8:	R L (L B L) (5)\n\
@9:	R (U2 D' F R) (5)\n\
@10:	R (U2 F D' R) (5)\n\
nissy-# add @6 $3\n\
$3:	R' U' F R U R2 F2 R2 D R2 U L2 U R2 D2 B' D U' R D R' D U2 F2 R' U' F R U' L (B L) \n\
nissy-# unniss $3\n\
@1:	L' B' R' U' F R U R2 F2 R2 D R2 U L2 U R2 D2 B' D U' R D R' D U2 F2 R' U' F R U' L \n\
nissy-# invert @1\n\
@1:	L' U R' F' U R F2 U2 D' R D' R' U D' B D2 R2 U' L2 U' R2 D' R2 F2 R2 U' R' F' U R B L \n\
nissy-# save @1\n\
$5:	L' U R' F' U R F2 U2 D' R D' R' U D' B D2 R2 U' L2 U' R2 D' R2 F2 R2 U' R' F' U R B L \n\
nissy-# dr from rl $5\n\
@1:	F2 U D2 F' B D B (7)\n\
nissy-# \n\
\n\
If you ask nissy to solve a substep (or the whole cube) using a sequence with\n\
NISS as scramble, it will first un-NISS it (but without saving the unNISSed\n\
scramble anywhere):\n\
\n\
print $3\n\
$3:	R' U' F R U R2 F2 R2 D R2 U L2 U R2 D2 B' D U' R D R' D U2 F2 R' U' F R U' L (B L) \n\
nissy-# solve $3\n\
@1:	F U' R2 F2 U2 F U2 R2 L2 D R2 U B2 D' L2 D B2 D2 (18)\n\
\n\
Nissy knows how to solve certain common sub-steps for DR (or Thistlethwaite /\n\
Kociemba algorithms). For now it does know more common speedsolving methods.\n\
\n\
For a full list of commands type \"help\". For a more detailed help on a specific\n\
command, type \"help (command)\". The help pages can also be found in the docs\n\
folder.\n\
\n\
If you want to report a bug (I'm sure there are many!) or give a suggestion,\n\
you can send an email to sebastiano.tronto@gmail.com.\n\
\n\
Have fun!\n\
\n\
"
},

{ "pic",
"\
\n\
HELP PAGE FOR COMMAND pic\n\
\n\
SYNTAX\n\
pic [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Prints the cube state after applying the given scramble.\n\
\n\
EXAMPLES\n\
pic R' U' F R U R2 F2 R2 D R2 U L2 U R2 D2 B' D U' R D R' D U2 F2 R' U' F \n\
  Gives the following output:\n\
               UF  UL  UB  UR  DF  DL  DB  DR  FR  FL  BL  BR \n\
EP:		         FR  UL  FL  UR  UF  DB  DR  BL  UB  BR  DL  DF \n\
EO(F/B):	      x   x   x   x       x           x   x   x     \n\
\n\
		           UFR  UFL  UBL  UBR  DFR  DFL  DBL  DBR \n\
CP:		         UBR  UFR  DFL  DBL  UFL  UBL  DBR  DFR \n\
CO(U/D):	     ccw                  cw       ccw   cw\n\
\n\
"
},

{ "print",
"\
\n\
HELP PAGE FOR COMMAND print\n\
\n\
SYNTAX\n\
print [$ID|@ID]\n\
\n\
DESCRIPTION\n\
Prints memorized sequences. If no argument is given, it prints all memorized\n\
scrambles ($ only). If $ID or @ID is specified, it only prints the relative\n\
memorized sequence.\n\
\n\
EXAMPLES\n\
print\n\
  Prints a list of all memorized scrambles (only $).\n\
print $2\n\
  Prints the second memorized scramble.\n\
print @13\n\
  Prints the 13th sequence that was part of the output of the last command.\n\
\n\
"
},

{ "quit",
"\
\n\
HELP PAGE FOR COMMAND quit\n\
\n\
SYNTAX\n\
quit\n\
\n\
DESCRIPTION\n\
Exits nissy.\n\
\n\
"
},

{ "replace",
"\
\n\
HELP PAGE FOR COMMAND replace\n\
\n\
SYNTAX\n\
eo [OPTIONS] [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Looks for non-optimal subsequences and replaces them to shorten the given\n\
sequence. By default it tries to shorten every subsequence of up to 10 moves,\n\
but this can be change with the \"b\" option.\n\
It outputs at most 10 equivalent optimal sequences for each replaceable part.\n\
\n\
OPTIONS\n\
b=N                Finds non-optimal subsequences of up to N moves.\n\
\n\
EXAMPLES\n\
replace D2 F' D2 U2 F' L2 R2 U' D B2 D B2 U B2 F L2 R' F' D U' \n\
  Produces the following output:\n\
Replace [ R2 U' D B2 D B2 ] (moves 7-12) with: [ D R2 D U' ] (-6+4)\n\
Replace [ R2 U' D B2 D B2 U ] (moves 7-13) with: [ D R2 D ] (-7+3)\n\
Replace [ U' D B2 D B2 U ] (moves 8-13) with: [ R2 D R2 D ] (-6+4)\n\
\n\
"
},

{ "save",
"\
\n\
HELP PAGE FOR COMMAND save\n\
\n\
SYNTAX\n\
save [MOVES|@ID|$ID]\n\
\n\
DESCRIPTION\n\
Memorizes the scramble specified by MOVES, given as input or temporarily saved\n\
as @ID, where ID is a number ('help nissy' for for more on IDs). If an $ID is\n\
given, it makes a copy of the scramble. An identifier of the form $ID, where ID\n\
is a number, is assigned to the memorized scramble.\n\
\n\
EXAMPLES\n\
save R U R' U'\n\
  Saves the scramble R U R' U'.\n\
save F (B)\n\
  Saves the scramble F (B) (NISS notation).\n\
save @3\n\
  Saves the third output sequence of the last command.\n\
save $2\n\
  Makes a copy of the second saved scramble.\n\
\n\
"
},

{ "scramble",
"\
\n\
HELP PAGE FOR COMMAND scramble\n\
\n\
SYNTAX\n\
scramble [OPTIONS]\n\
\n\
DESCRIPTION\n\
Produces a random-state scramble. There are options to get a corners-only,\n\
edges-only or dr-state scramble.\n\
\n\
OPTIONS\n\
c                  Scrambles corners only (edges are solved).\n\
e                  Scrambles edges only (corners are solved).\n\
dr                 DR-state scramble. The DR is always on the U/D axis.\n\
\n\
\n\
EXAMPLES\n\
scramble\n\
  Gives a random-state scramble\n\
scramble dr\n\
  Gives a random-DR-state scramble\n\
\n\
"
},

{ "solve",
"\
\n\
HELP PAGE FOR COMMAND solve\n\
\n\
SYNTAX\n\
solve [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Solves the given scramble, which can be given as a sequence of moves or as $ID\n\
or @ID. If none is given, the user can type it on the next line.\n\
The algorithm first tries to find a short (<=10 moves) solution, and then\n\
switches to a 2-step algorithm (unless the option \"o\" is specified, in which\n\
case it keeps looking for an optimal solution).\n\
The first time it uses the 2-step algorithm it needs to load some tables, which\n\
can take a few seconds. It runs much faster after that. If the option \"o\" is\n\
specified, the first time it loads some large tables, which can take a minute\n\
or two.\n\
\n\
OPTIONS\n\
b=N                Only looks for solutions up to N moves.\n\
n=N                Tries to find multiple solutions, at most N. Multiple\n\
                   Solutions will only be found if they are <=10 moves.\n\
o                  Looks for optimal solution.\n\
\n\
\n\
EXAMPLES\n\
solve R' U' F\n\
  Solves the scramble R' U' F.\n\
solve o b=14 $1\n\
  Tries to solve the scramble $1 optimally, but stops if no solution of 14\n\
  moves or shorter is found.\n\
solve n=16 R L' U2 R' L F2\n\
  Finds the 16 shortest solutions for the scramble above.\n\
\n\
"
},

{ "unniss",
"\
\n\
HELP PAGE FOR COMMAND unniss\n\
\n\
SYNTAX\n\
unniss [MOVES|$ID|@ID]\n\
\n\
DESCRIPTION\n\
Removes NISS from a sequence of moves, which can be given also as $ID or @ID.\n\
A sequence of the form A (B) is translated to B' A.\n\
\n\
EXAMPLES\n\
invert F R (D' L2)\n\
  Prints L2 D F R\n\
\n\
"
},
};
