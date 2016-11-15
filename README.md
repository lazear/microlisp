# microlisp
A set of minimal LISP implementations.

!["I've just received word that the Emperor has dissolved the MIT computer science program permanently."](https://imgs.xkcd.com/comics/lisp_cycles.png)

1. lisp1 is a self contained interpreter written in ANSI C for the original LISP language described in John McCarthy's 1960 paper [Recursive Functions of Symbolic Expressions
and Their Computation by Machine, Part I][1]
lisp1 defines only the primitives described in McCarthy's paper - namely quote, atom, eq, cond, cons, car, cdr, label, and lambda. There is no ability to define variables, and no scope. 

2. lisp2 contains several extensions, including the scheme-style "define" keyword, which constructs lambda expressions from, i.e. '(define (next item) (cdr item))' would be translated into '(define next (lambda (item) (cdr item))', but this does not allow returning of closures. In addition, the arithmetic primitives (*, /, +, -) have been added.

3. The scheme folder contains a minimal implementation of scheme. Currently working on tail call optimization, as laid out in the standard

[1]: http://www-formal.stanford.edu/jmc/recursive.pdf
