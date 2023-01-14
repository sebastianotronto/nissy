# Simplify and improve installation

## Tables

* Make install should generate tables, or add a "make tables" target to
  generate tables.
* Make tables should also check for existing files and remove old ones
  (maybe more for nissy's command than for makefile).

## Correctness

* Add checksum for all generated files.
* Hard-code results? Check for compatibility problems between different OSes
  and filesystems - but there should not be any, since we use stdint.h.
