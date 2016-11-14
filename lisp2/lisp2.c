/* MICHAEL LAZEAR 2016
Implements McCarthy's 5 LISP primitives in ANSI C:
atom, eq, car, cdr, cons
+ label, lambda, and quote */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "lisp.h"


object_t* add(object_t* first, object_t* rest) {
	if (null(rest) || null(car(rest)))
		return first;
	first->integer += car(rest)->integer;
	return add(first, cdr(rest));
}

object_t* sub(object_t* first, object_t* rest) {
	if (null(rest) || null(car(rest)))
		return first;
	first->integer -= car(rest)->integer;
	return sub(first, cdr(rest));
}

object_t* imul(object_t* first, object_t* rest) {
	if (null(rest) || null(car(rest)))
		return first;
	first->integer *= car(rest)->integer;
	return imul(first, cdr(rest));
}

object_t* idiv(object_t* first, object_t* rest) {
	if (null(rest) || null(car(rest)))
		return first;
	first->integer /= car(rest)->integer;
	return idiv(first, cdr(rest));
}

object_t* assoc(object_t* key, object_t* alist) {
	object_t** tmp;
	for (tmp = &alist; *tmp; tmp=&(*tmp)->cdr)  {
		if (eq(car(car((*tmp))), key))
			return cdr(car(*tmp));
	}
	return &nil;	
}

object_t* evlis(object_t* sexp, object_t** env) {
	if (null(sexp))
		return &nil;
	object_t* first = eval(car(sexp), env);
	return cons(first, evlis(cdr(sexp), env));
}

object_t* evcon(object_t* cond, object_t** env) {
	if (eval(caar(cond), env) == &C_TRUE)
		return eval(cadar(cond), env);
	return eval(caddar(cond), env);
}


object_t* eval(object_t* sexp, object_t** env) {
	if (null(sexp))
		return &nil;
	printf("eval:");
	print(sexp);
	switch(sexp->type) {
		case INT:
			return sexp;
		case SYM: {
			object_t* ret = assoc(sexp, *env);
			return ret;
		} case CONS: {
			if (atom(sexp->car)) {
				if(car(sexp)->type == SYM) {
					if (!strcmp(car(sexp)->symbol, "define")) {
						object_t* new_env;
						if (atom(cadr(sexp))) {
							new_env = cons(cons(cadr(sexp), eval(caddr(sexp), env)), *env);
						} else {
							new_env = cons(cons(car(cadr(sexp)), cons(new_sym("lambda"), cons(cdr(cadr(sexp)), cons(caddr(sexp), &nil)))), &nil);
							print(new_env);
						}						
						*env = new_env;
						return new_sym("ok");
					}
					if (!strcmp(car(sexp)->symbol, "nil"))
						return &nil;
					if (!strcmp(sexp->car->symbol, "cons")) 
						return cons(eval(cadr(sexp), env), eval(caddr(sexp), env));
					if (!strcmp(sexp->car->symbol, "car"))
						return car(eval(cadr(sexp), env));
					if (!strcmp(sexp->car->symbol, "cdr")) 
						return cdr(eval(cadr(sexp), env));
					if (!strcmp(sexp->car->symbol, "+")) 
						return add(eval(cadr(sexp), env), evlis(cddr(sexp), env));
					if (!strcmp(sexp->car->symbol, "-")) 
						return sub(eval(cadr(sexp), env), evlis(cddr(sexp), env));
					if (!strcmp(sexp->car->symbol, "*")) 
						return imul(eval(cadr(sexp), env), evlis(cddr(sexp), env));
					if (!strcmp(sexp->car->symbol, "/")) 
						return idiv(eval(cadr(sexp), env), evlis(cddr(sexp), env));
					if (!strcmp(sexp->car->symbol, "quote"))
						return cadr(sexp);
					if (!strcmp(sexp->car->symbol, "eq")) 
						return eq(eval(cadr(sexp), env), eval(caddr(sexp), env)) ? &C_TRUE : &C_FALSE;
					if (!strcmp(sexp->car->symbol, "atom")) 
						return atom(eval(cadr(sexp), env)) ? &C_TRUE : &C_FALSE;
					if (!strcmp(sexp->car->symbol, "cond")) 
						return evcon(cdr(sexp), env);
				}

				/* Stack lambda expressions */

				object_t* func = assoc(car(sexp), *env);
				object_t* args = cdr(sexp);
				if (strcmp(car(func)->symbol, "lambda"))
					args = evlis(args, env);

				return eval(cons(func, args), env);
			}
			if (car(car(sexp))->type == SYM) {
				if (!strcmp(caar(sexp)->symbol, "label")) {
					object_t* new_env = cons(list(2, cadar(sexp), car(sexp)), *env);
					return eval(cons(caddar(sexp), (cdr(sexp))), &new_env);
				}
				if (!strcmp(caar(sexp)->symbol, "lambda")) {
					object_t* p = append(pair(cadar(sexp), evlis(cdr(sexp), env)), *env);
					return eval(caddar(sexp), &p);
				}

				return eval(cons((cons(assoc(caar(sexp), *env), cdr(car(sexp)))), evlis(cdr(sexp), env)), env);
			}
			return &nil;
		}
		default:
			printf("Unknown eval expression:\n");
			print(sexp);
			return &nil;
	}
	return &nil;
}

object_t* append(object_t* pair, object_t* list) {
	if (null(pair))
		return list;
	return cons(car(pair), append(cdr(pair), list));
}

object_t* pair(object_t* x, object_t* y) {
	if (null(x) && null(y))
		return &nil;
	return cons(cons(car(x), car(y)), pair(cdr(x), cdr(y)));
}
object_t* appq(object_t* sexp) {
	if (null(sexp))
		return &nil;
	return cons(cons(new_sym("quote"), car(sexp)), appq(cdr(sexp)));
}
 //((label ff (lambda (a) (cond ((atom a) a ((quote t) (ff (car a))))))) (cons 1 2))

int main(int argc, char** argv) {
	char* input = malloc(256);
	object_t* env = new_cons();
	env = append(pair(cons(new_sym("t"), &nil), cons(&C_TRUE, &nil)), env);
	env = append(pair(cons(new_sym("f"), &nil), cons(&C_FALSE, &nil)), env);
	env = append(pair(cons(new_sym("nil"), &nil), cons(&nil, &nil)), env);

	printf("Welcome to LISP2. Press ! to exit\n");
	size_t sz;
	do {
		printf("user> ");
		getline(&input, &sz, stdin);
		if (*input == '!')
			break;
		printf("=> ");
		print(eval(scan(input), &env));
	} while(*input);
	return 0;
}