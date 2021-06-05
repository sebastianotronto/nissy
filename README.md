# nissy
A Rubik's cube solver and FMC assistant.

## Just another cube solver?
Yes, pretty much. I wanted to write one and I started coding without any
specific goal in mind. It is not more efficient than [CubeExplorer](http://kociemba.org/cube.htm), nor it is
particularly user-friendly.

## But does it do something unique?
Yes, actually it does, but only for a very small niche of people. It allows to produce step-by-step solutions using DR
([Thistlethwaite](/https://www.speedsolving.com/wiki/index.php/Thistlethwaite%27s_algorithm)/[Kociemba](https://www.speedsolving.com/wiki/index.php/Kociemba%27s_Algorithm) algorithm)
combined with [NISS](https://www.speedsolving.com/wiki/index.php/Fewest_Moves_techniques). This makes it somewhat useful for [FMC](https://www.speedsolving.com/wiki/index.php/Fewest_Moves_Challenge) solvers who want to analyze a scramble and see if they missed something,
or what was the optimal way to solve a certain substep at a given point, and so on.

## How to use it
**Update:** the file nissy-win.exe should work as a Windows executable, but
I have not tested it (I don't have a Windows machine).

Check out the help pages in the docs folder. They are also available from
within nissy with the command "help".

I will add more examples and maybe screenshots when I feel like.

## Installation
For now you have to download all the files and compile the source 
code yourself. Remember to tell your
compiler to use the [C99 standard](https://en.wikipedia.org/wiki/C99). For
example, on a Linux system with GCC installed:

```
cd path/to/nissy
gcc -O2 -std=c99 -o nissy ./src/*.c
./nissy
```

You can also use the script compile.sh, which executes that
gcc line (with a few extra options).

## Tips
You can use a tool such as [rlwrap](https://github.com/hanslub42/rlwrap) to allow
for infinte command history within nissy!
