#include "lisp.h"
#include <stdio.h>

void __number_of_args(const char* func, object_t* sexp, int expected) {
	object_t** tmp;
	int args;

	if (null(sexp))
		args = 0;
	else if (atom(sexp))
		args = 1;
	else {
		for (tmp = &sexp; *tmp; tmp=&(*tmp)->cdr)
			args++;
	}

	if (args != expected) {
		fprintf(stderr, "Invalid number of arguments to function %s. Expected %d got %d\n", func, expected, args);
		exit(1);
	}
}

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

object_t* p_add(object_t* sexp) {
	return add(car(sexp), cdr(sexp));
}

object_t* p_gt(object_t* sexp) {
	arity(2);
	return (car(sexp)->integer > cadr(sexp)->integer) ? \
		&C_TRUE : &C_FALSE;
}

object_t* p_lt(object_t* sexp) {
	arity(2);
	return (car(sexp)->integer < cadr(sexp)->integer) ? \
		&C_TRUE : &C_FALSE;
}

object_t* p_car(object_t* sexp) {
	return car(sexp);
}

object_t* p_cdr(object_t* sexp) {
	return cdr(sexp);
}

object_t* p_cons(object_t* sexp) {
	object_t* c = cons(car(sexp), cdr(sexp));
	c->type = CONS;
	return c;
}

object_t* new_primitive(primitive_t op) {
	object_t* p = new();
	p->type = PRIM;
	p->primitive = op;
	return p;
}

object_t* init_prim(object_t* env) {
	#define add_proc(scheme, c) define_variable(new_sym(scheme), new_primitive(c), env);

	add_proc("cons", p_cons);
	add_proc("car", p_car);
	add_proc("cdr", p_cdr);
}