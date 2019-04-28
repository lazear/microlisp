# microlisp
[![Build Status](https://travis-ci.org/lazear/microlisp.svg?branch=master)](https://travis-ci.org/lazear/microlisp)

A set of minimal LISP[1] implementations.

!["I've just received word that the Emperor has dissolved the MIT computer science program permanently."](https://imgs.xkcd.com/comics/lisp_cycles.png)

1. The `scheme` folder contains a minimal implementation of scheme that supports the most important primitives. Currently only supports integer type for numbers. This implementation takes the homoiconic approach outlined in SICP's metacircular evaluator, in which procedures are represented as lists tagged with 'procedure. This allows some interesting differences over other C implementations where procedures are stored as C-style data structures. Due to lack of a garbage collector, it's relatively quick but can be a major memory hog (e.g. when calculating the 30th fibonacci number using recursion)

2. The `scheme-gc` folder contains a version of the `scheme` interpreter with a garbage collector

To build and run either of the projects, `cd` into the folder and run `./configure && make && ./build/microlisp`

[1]: http://www-formal.stanford.edu/jmc/recursive.pdf
