#/!bin/sh

mkdir build
cd build
cp -R ../../src ./
rm src/shell.c
cp ../$1 src/
cp ../../Makefile ./
make
cp nissy ../run
rm src/*
rmdir src
rm *
cd ..
rmdir build
