# Testing

## Architecture

* Folder structure: each module (.h file) has a corresponding test/module_name
  folder containing the important tests.
* How to test pre / post -init()?
* Makefile: one target for each module with correct dependencies.
* Makefile: perhaps write a specific makefile for testing in test folder.

## Test sttructure

* Make consistent
* Little output for success
* Stop on first failed? (automatic with makefile)

## Write tests

* Pretty much all are missing, except fst.
* Start from bottom (utils.c)

## Other

* Move test_coord from coord.c to test folder.
