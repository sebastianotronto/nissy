# Where is nissy?

In April 2023 I have decided to split nissy in 2 separate projects: an
optimal solver and an FMC assistant. These two projects are not usable
yet, so in the meantime I will keep a third branch where I update the
classic version of nissy with bugfixes and minor improvements.

You can find these three branches at the following pages (also
on github, links below are to my personal git instance):

* [nissy-classic](https://git.tronto.net/nissy-classic): The stable
  branch, will receive bugfixes and minor improvements but no big
  change. Will eventually be replaced by the other two versions.
* [nissy-fmc](https://git.tronto.net/nissy-fmc): This will focus on
  features useful for practicing FMC, for example finding EOs and
  DRs (if you do not know what this means, there is a good chance
  you don't care). I plan to make a graphical interface for it and
  make it more usable. It will not be able to find an optimal
  solution. *Not working at the moment.*
* [nissy-nx](https://git.tronto.net/nissy-nx): An optimal solver.
  For now this is just my playground for implementing complex
  optimizations, following the ideas of Tomas's Rokicki's
  [nxopt](https://github.com/rokicki/cube20src/blob/master/nxopt.md).
  Eventually it will become faster than nissy-classic at optimal
  solving, but without all other features. *Not working at the moment.*

## F.A.Q.

### I am a user of nissy, what should I do?

If you are happy with using nissy as it is, you can keep using it.

When nissy-fmc and nissy-classic are ready, you can chek them out too.

### But I liked that nissy can do both optimal solving! Why did you split it?

Then I encourage you to keep using nissy-classic :-)

See
[my blog post](https://sebastiano.tronto.net/blog/2023-04-10-the-big-rewrite/)
for some reasons behind this change.

### I just want to look at the code, where should I go?

If you want to check out the git repository of the version you are
running, you probably want nissy-classic. The newer nissy-fmc will
eventually be nicer and easier to read, containing more or less
the same functionality (except for optimal solving).

If you want to follow my progress on an advanced optimal Rubik's
cube solver, you can check out nissy-nx - just remember that it
does not work yet!
