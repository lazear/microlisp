# microlisp
[![Build Status](https://travis-ci.org/lazear/microlisp.svg?branch=master)](https://travis-ci.org/lazear/microlisp)

A set of minimal LISP implementations.

!["I've just received word that the Emperor has dissolved the MIT computer science program permanently."](https://imgs.xkcd.com/comics/lisp_cycles.png)

1. `lisp1` is a self contained interpreter written in ANSI C for the original LISP language described in John McCarthy's 1960 paper [Recursive Functions of Symbolic Expressions
and Their Computation by Machine, Part I][1]
lisp1 defines only the primitives described in McCarthy's paper - namely quote, atom, eq, cond, cons, car, cdr, label, and lambda. There is no ability to define variables, and no scope. 

2. The `scheme` folder contains a minimal implementation of scheme that supports the most important primitives. Currently only supports integer type for numbers. This implementation takes the homoiconic approach outlined in SICP's metacircular evaluator, in which procedures are represented as lists tagged with 'procedure. This allows some interesting differences over other C implementations where procedures are stored as C-style data structures. 

3. The `scheme-gc` folder contains a version of the `scheme` interpreter with a garbage collector

[1]: http://www-formal.stanford.edu/jmc/recursive.pdf
