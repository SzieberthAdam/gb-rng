GB-RNG
Copyright (c) 2018 Szieberth Ádám
0BSD License (see LICENSE file for more info)


ABSTRACT
========

The aim of this program is to standardize the random number gen‐
erators and their testing written for the classic Game Boy hard‐
ware.  The main (host) program is kept simple as possible yet it
provides a standard initial random seed  for the RNGs and a vis‐
ual feedback of the random data.  The attached RNG ASM files are
only required  to have two exported subroutines  named as "rand"
and "rand-init".

As this project went to hiatus, its current state it is rather a
small  commented collection  of ASM RNGs  for the Game Boy which
may worth some attention of its own especaially by beginner Game
Boy developers.
