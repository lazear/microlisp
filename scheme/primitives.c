#include "lisp.h"
#include <stdio.h>

#define type(x, t) (__type_check(__func__, x, t))

void __type_check(const char* func, object_t* obj, type_t type) {
	if (null(obj)) {
		fprintf(stderr, "Invalid argument to function %s: NIL", func);
		exit(1);
	}
	if (obj->type != type) {
		fprintf(stderr, "Invalid argument to function %s. Expected type %d got %d\n", func, type, obj->type);
		exit(1);
	}
}

void __number_of_args(const char* func, object_t* sexp, int expected) {
	object_t** tmp;
	int args = 0;

	if (null(sexp))
		args = 0;
	else if (atom(sexp))
		args = 1;
	else {
		for (tmp = &sexp; !null(*tmp); tmp=&(*tmp)->cdr)
			args++;
	}
	if (args != expected) {
		fprintf(stderr, "Invalid number of arguments to function %s. Expected %d got %d\n", func, expected, args);
		exit(1);
	}
}

object_t* add_list(object_t* list) {
	type(car(list), INT);
	int total = car(list)->integer;
	list = cdr(list);
	while (!null(list)) {
		type(car(list), INT);
		total += car(list)->integer;
		list = cdr(list);
	}
	return new_int(total);
}

object_t* sub_list(object_t* list) {
	type(car(list), INT);
	int total = car(list)->integer;
	list = cdr(list);
	while (!null(list)) {
		type(car(list), INT);
		total -= car(list)->integer;
		list = cdr(list);
	}
	return new_int(total);
}

object_t* div_list(object_t* list) {
	type(car(list), INT);
	int total = car(list)->integer;
	list = cdr(list);
	while (!null(list)) {
		type(car(list), INT);
		total /= car(list)->integer;
		list = cdr(list);
	}
	return new_int(total);
}

object_t* mul_list(object_t* list) {
	type(car(list), INT);
	int total = car(list)->integer;
	list = cdr(list);
	while (!null(list)) {
		type(car(list), INT);
		total *= car(list)->integer;
		list = cdr(list);
	}
	return new_int(total);
}



object_t* p_gt(object_t* sexp) {
	arity(2);
	return (car(sexp)->integer > cadr(sexp)->integer) ? TRUE : FALSE;
}

object_t* p_lt(object_t* sexp) {
	arity(2);
	return (car(sexp)->integer < cadr(sexp)->integer) ? TRUE : FALSE;
}

object_t* p_eq(object_t* sexp) {
	arity(2);
	return eq(car(sexp), cadr(sexp)) ? 	TRUE : FALSE;
}

object_t* p_atom(object_t* sexp) {
	return atom(car(sexp)) ? TRUE : FALSE;
}

object_t* p_cons(object_t* sexp) {
	//arity(2);
	return cons(car(sexp), cadr(sexp));
}

object_t* p_car(object_t* sexp) {
	return caar(sexp);
}

object_t* p_cdr(object_t* sexp) {
	return cdr(car(sexp));
}
object_t* p_null(object_t* sexp) {
	return null(car(sexp)) ? TRUE : FALSE;
}

object_t* p_pair (object_t* exp) {
	if (car(exp)->type == CONS)
		return (caar(exp)->type != CONS && cdr(car(exp))->type != CONS) ? TRUE : FALSE;
	return FALSE;
}

object_t* p_listq(object_t* exp) {
	return (car(exp)->type == CONS && p_pair(exp) != TRUE) ? TRUE : FALSE;
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
	add_proc("+", add_list);
	add_proc("-", sub_list);
	add_proc("*", mul_list);
	add_proc("/", div_list);
	add_proc(">", p_gt);
	add_proc("<", p_lt);
	add_proc("eq", p_eq);
	add_proc("atom?", p_atom);
	add_proc("null?", p_null);
	add_proc("pair?", p_pair);
	add_proc("list?", p_listq);
}