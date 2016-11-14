# microlisp
A set of minimal LISP implementations.

!["I've just received word that the Emperor has dissolved the MIT computer science program permanently."](https://imgs.xkcd.com/comics/lisp_cycles.png)

lisp1.c is a self contained interpreter written in ANSI C for the original LISP language described in John McCarthy's 1960 paper [Recursive Functions of Symbolic Expressions
and Their Computation by Machine, Part I][1]
lisp1 defines only the primitives described in McCarthy's paper - namely quote, atom, eq, cond, cons, car, cdr, label, and lambda. There is no ability to define variables, and no scope. But that can be easily added in, and the other C files here will contain some minor additional functionalities.

[1]: http://www-formal.stanford.edu/jmc/recursive.pdf
