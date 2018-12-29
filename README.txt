GB-RNG
Copyright (c) 2018 Szieberth Ádám
MIT License (see LICENSE file for more info)


ABSTRACT
========

The aim of this program is to standardize the random number gen‐
erators and their testing written for the classic Game Boy hard‐
ware.  The main (host) program is kept simple as possible yet it
provides a standard initial random seed  for the RNGs and a vis‐
ual feedback of the random data.  The attached RNG ASM files are
only required  to have two exported subroutines  named as "rand"
and "rand-init".
