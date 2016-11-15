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

bool is_tagged(object_t* exp, object_t* tag) {
	if (!null(exp) && exp->type == CONS)
		return eq(car(exp), tag);
	return false;
}

object_t* make_lambda(object_t* params, object_t* body) {
	return cons(LAMBDA, cons(params, body));
}

object_t* make_procedure(object_t* params, object_t* body, object_t* env) {
	return cons(PROCEDURE, cons(params, cons(body, cons(env, &nil))));
}

object_t* evlis(object_t* sexp, object_t* env) {
	if (null(sexp))
		return &nil;
	object_t* first = eval(car(sexp), env);
	return cons(first, evlis(cdr(sexp), env));
}

object_t* eval_sequence(object_t* exps, object_t* env) {
	if (null(cdr(exps))) {
		return eval(car(exps), env);
	}
	
	eval(car(exps), env);
	return eval_sequence(cdr(exps), env);
}

object_t* extend_env(object_t* var, object_t* val, object_t* env) {

	return cons(cons(var, val), env);
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
	object_t* frame = first_frame(env);
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
	else if (is_tagged(procedure, PROCEDURE)) {
		object_t* env = extend_env(procedure_params(procedure), arguments, procedure_env(procedure));
		return eval_sequence(procedure_body(procedure), env);
	}
	else {
		printf("Error in apply: ");
		print(procedure);
		print(arguments);
		error("Unknown procedure type");
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
*/

object_t* eval(object_t* exp, object_t* env) {
//printf("EVAL\n");
tail:
	if (null(exp)) 
		return &nil;
 	else if (exp->type == INT || exp->type == STRING)
 		return exp;
 	else if (exp->type == SYM) {
 		object_t* ret = lookup_variable(exp, env);
 		if (ret == &UNBOUND) {
 			print(exp);
 			error("Unbound variable!");
 		}
 		return ret;
 	}
 	else if (is_tagged(exp, QUOTE))
 		return cadr(exp);
 	else if (is_tagged(exp, SET)) {
 		set_variable(cadr(exp), eval(caddr(exp), env), env);
 		exp = cadr(exp);
 		goto tail;
 	} else if (is_tagged(exp, DEFINE)) {
		if ((atom(cadr(exp)))) {
			define_variable(cadr(exp), eval(caddr(exp), env), env);
		} else {
			object_t* closure = eval(make_lambda(cdr(cadr(exp)), cddr(exp)), env);
			define_variable(car(cadr(exp)), closure, env);
		}
		return OK;
 	} 
 	else if (is_tagged(exp, IF))  {
  		object_t* predicate = eval(cadr(exp), env);
 		object_t* consequent = caddr(exp);
 		object_t* alternative = cadddr(exp);

 		if (!eq(predicate, FALSE)) 
 			exp = consequent;
 		else 
 			exp = alternative;
 		goto tail;
 	}
 	else if (is_tagged(exp, BEGIN)) {
 		exp = eval_sequence(cdr(exp), env);
 		goto tail;
  	}
 	else if (is_tagged(exp, LAMBDA))
 		return make_procedure(cadr(exp), cddr(exp), env);
 	else if (is_tagged(exp, COND)) 
 		return new_sym("COND");
 	else if (!atom(exp)) {
 		object_t* proc = eval(car(exp), env);
 		object_t* args = evlis(cdr(exp), env);
 	// 	print(env);
		// if (proc->type == PRIM)
		// 	return proc->primitive(args);

 	// 	if (is_tagged(proc, PROCEDURE)) {
 	// 		env = extend_env(procedure_params(proc), args, procedure_env(proc));
 	// 		proc->cdr->cdr->cdr->car = env;
 	// 		exp = cons(BEGIN, procedure_body(proc));
 	// 		goto tail;
 	// 	}

 		return apply(proc, args);
 	}
 	else
 		error("Unknown eval!");
}

/*
(define factorial (lambda(n) (if (= n 0) 1 (* n (factorial (- n 1))))))
(define (fact n) (if (= n 0) 1 (* n (factorial (- n 1)))))
(define (fact n) (define (product min max) (if (= min n) max (product (+ 1 min) (* min max))))(product 1 n))
      
(define (fact-iter product counter max-count) (if (> counter max-count) product (fact-iter (* counter product) (+ counter 1) max-count)))
      */

int main(int argc, char** argv) {
	char* input = malloc(256);
	object_t* env = new_cons();
	/* Initialize static keywords */
	LAMBDA 		= new_sym("lambda");
	QUOTE 		= new_sym("quote");
	OK 			= new_sym("ok");
	PROCEDURE 	= new_sym("procedure");
	BEGIN 		= new_sym("begin");
	IF 			= new_sym("if");
	COND 		= new_sym("cond");
	SET 		= new_sym("set!");
	DEFINE 		= new_sym("define");
	/* Initialize initial environment */
	env = extend_env(scan("(true false)"), cons(TRUE, FALSE), new_cons());
	init_prim(env);
	eval(scan("(define = eq)"), env);
	//eval(scan("(define factorial (lambda(n) (if (= n 0) 1 (* n (factorial (- n 1))))))"), env);
	//print(eval(scan("(factorial 5)"), env));
	printf("LITH ITH LITHENING...\n");
	size_t sz;
	do {
		printf("user> ");
		getline(&input, &sz, stdin);
		if (*input == '!')
			break;
		object_t* in = scan(input);
		//print(in);
		printf("=> ");
		print(eval(in, env));
	} while(*input);
	return 0;
}