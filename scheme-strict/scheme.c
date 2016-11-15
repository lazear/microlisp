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


bool is_self_evaluating(object_t* exp) {
	if (null(exp))
		return false;
	return (exp->type == INT || exp->type == STRING);
}

bool is_variable(object_t* exp) {
	if (null(exp))
		return false;
	return (exp->type == SYM);
}

bool is_tagged(object_t* exp, object_t* tag) {
	if (!null(exp) && exp->type == CONS)
		return eq(car(exp), tag);
	return false;
}

bool is_quoted(object_t* exp) {
	if (null(exp) || atom(exp))
		return false;
	return is_tagged(exp, quote);
}

bool is_lambda(object_t* exp) {
	if (null(exp) || atom(exp))
		return false;
	return is_tagged(exp, lambda);
}

bool is_procedure(object_t* exp) {
	if (null(exp) || atom(exp))
		return false;
	return is_tagged(exp, new_sym("procedure"));
}

object_t* make_lambda(object_t* params, object_t* body) {
	return cons(lambda, cons(params, body));
}

object_t* make_procedure(object_t* params, object_t* body, object_t* env) {
	return cons(new_sym("procedure"), cons(params, cons(body, cons(env, &nil))));
}


object_t* assoc(object_t* key, object_t* alist) {
	object_t** tmp;
	for (tmp = &alist; *tmp; tmp=&(*tmp)->cdr)  {
		if (eq(car(car((*tmp))), key))
			return cdr(car(*tmp));
	}
	return &nil;	
}

object_t* evlis(object_t* sexp, object_t* env) {
	if (null(sexp))
		return &nil;
	object_t* first = eval(car(sexp), env);
	return cons(first, evlis(cdr(sexp), env));
}

object_t* evcon(object_t* cond, object_t* env) {
	if (eval(caar(cond), env) == &C_TRUE)
		return eval(cadar(cond), env);
	return eval(caddar(cond), env);
}

object_t* eval_sequence(object_t* exps, object_t* env) {
	printf("eval-sequence:");
	print(car(exps));
	print(env);
	if (null(cdr(exps))) {
		return eval(car(exps), env);
	}
	
	eval(car(exps), env);
	return eval_sequence(cdr(exps), env);
}



object_t* lookup_variable(object_t* var, object_t* env) {
	while (!null(env)) {
		object_t* frame = first_frame(env);
		object_t* vars 	= frame_variables(frame);
		object_t* vals 	= frame_values(frame);
		while(!null(vars)) {
			if (eq(car(vars), var))
				return car(vals);
			vars = cdr(vars);
			vals = cdr(vals);
		}
		env = enclosing_env(env);
	}
	return &UNBOUND;
}

/* set_variable binds var to val in the first frame in which var occurs */
void set_variable(object_t* var, object_t* val, object_t* env) {
	while (!null(env)) {
		object_t* frame = first_frame(env);
		object_t* vars 	= frame_variables(frame);
		object_t* vals 	= frame_values(frame);
		while(!null(vars)) {
			if (eq(car(vars), var)) {
				vals->car = val;
				return;
			}
			vars = cdr(vars);
			vals = cdr(vals);
		}
		env = enclosing_env(env);
	}
}

/* define_variable binds var to val in the *current* frame */
void define_variable(object_t* var, object_t* val, object_t* env) {
	object_t* frame =first_frame(env);
	object_t* vars = frame_variables(frame);
	object_t* vals = frame_values(frame);

	while(!null(vars)) {
		if (eq(var, car(vars))) {
			vals->car = val;
			return;
		}
		vars = cdr(vars);
		vals = cdr(vals);
	}
	add_binding(var, val, frame);
}



/* (define (apply procedure arguments)
  (cond ((primitive-procedure? procedure)
         (apply-primitive-procedure procedure arguments))
        ((compound-procedure? procedure)
         (eval-sequence
           (procedure-body procedure)
           (extend-environment
             (procedure-parameters procedure)
             arguments
             (procedure-environment procedure))))
        (else
         (error
          "Unknown procedure type - APPLY" procedure))))
*/
object_t* apply(object_t* procedure, object_t* arguments) {

	if (procedure->type == PRIM) {
		return procedure->primitive(arguments);
	}
	else if (is_procedure(procedure)) {
		object_t* env = extend_env(procedure_params(procedure), arguments, procedure_env(procedure));
		return eval_sequence(procedure_body(procedure), env);
	}
	else {
		print(procedure);
		error("Unknown procedure type - APPLY");
	}
}


/*
(define (eval exp env)
  (cond ((self-evaluating? exp) exp)
        ((variable? exp) (lookup-variable-value exp env))
        ((quoted? exp) (text-of-quotation exp))
        ((assignment? exp) (eval-assignment exp env))
        ((definition? exp) (eval-definition exp env))
        ((if? exp) (eval-if exp env))
        ((lambda? exp)
         (make-procedure (lambda-parameters exp)
                         (lambda-body exp)
                         env))
        ((begin? exp) 
         (eval-sequence (begin-actions exp) env))
        ((cond? exp) (eval (cond->if exp) env))

        application == pair?
        operator == (car exp)
        operand == (cdr exp)
        ((application? exp)
         (apply (eval (operator exp) env)
                (list-of-values (operands exp) env)))
        (else
         (error "Unknown expression type - EVAL" exp))))

(define (eval-sequence exps env)
  (cond ((last-exp? exps) (eval (first-exp exps) env))
        (else (eval (first-exp exps) env)
              (eval-sequence (rest-exps exps) env))))

(define (eval-assignment exp env)
  (set-variable-value! (assignment-variable exp)
                       (eval (assignment-value exp) env)
                       env)
  'ok)

(define (eval-definition exp env)
  (define-variable! (definition-variable exp)
                    (eval (definition-value exp) env)
                    env)
  'ok)

*/

object_t* eval(object_t* exp, object_t* env) {
	printf("[eval]");
	print(exp);

tail:
	if (null(exp)) 
		return &nil;
 	else if (is_self_evaluating(exp)) 
 		return exp;
 	else if (exp->type == SYM)
 		return lookup_variable(exp, env);
 	else if (is_tagged(exp, quote))
 		return cadr(exp);
 	else if (is_tagged(exp, new_sym("set!"))) {
 		set_variable(cadr(exp), eval(caddr(exp), env), env);
 		exp = cadr(exp);
 		goto tail;
 	} else if (is_tagged(exp, new_sym("define"))) {
		if ((atom(cadr(exp)))) {
			define_variable(cadr(exp), eval(caddr(exp), env), env);
		} else {
			object_t* closure = eval(make_lambda(cdr(cadr(exp)), cddr(exp)), env);
			define_variable(car(cadr(exp)), closure, env);
		}
		return ok;
 	} 
 	else if (is_tagged(exp, new_sym("if"))) 
 		return ok;
 	else if (is_tagged(exp, new_sym("begin"))) {
 		exp = eval_sequence(cdr(exp), env);
 		goto tail;
  	}
 	else if (is_lambda(exp))
 		return make_procedure(cadr(exp), cddr(exp), env);
 	else if (is_tagged(exp, new_sym("cond"))) 
 		return new_sym("COND");
 	else if (!atom(exp)) {
 		object_t* proc = eval(car(exp), env);
 		object_t* args = evlis(cdr(exp), env);

 		exp = apply(eval(car(exp), env), evlis(cdr(exp), env));
 		goto tail;
 	}
 	else
 		error("Unknown eval!");
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


int main(int argc, char** argv) {
	char* input = malloc(256);
	object_t* env = new_cons();
	lambda = new_sym("lambda");
	quote = new_sym("quote");
	ok = new_sym("ok");
	env = extend_env(scan("(true false)"), cons(&C_TRUE, &C_FALSE), new_cons());
	init_prim(env);
	define_variable(new_sym("y"), eval(scan("(lambda(a) a)"), env), env);
	

	print(env);
	printf("Welcome to LISP2. Press ! to exit\n");
	size_t sz;
	do {
		printf("user> ");
		getline(&input, &sz, stdin);
		if (*input == '!')
			break;
		object_t* in = scan(input);
		print(in);
		printf("=> ");
		print(eval(in, env));
	} while(*input);
	return 0;
}