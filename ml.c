/* ul.c 
Implements McCarthy's 5 LISP primitives:
atom
eq
car
cdr
cons
*/

/* general form of a procedure definition
(define (<name> <formal parameters>) <body>) 

atomic symbols (m) should be (m . NIL)
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "lisp.h"


object_t* car(object_t* sexp) {
	if (sexp == NULL || sexp == &nil)
		return &nil;
	if (sexp->type == CONS)
		return sexp->car;
	return &nil;
}

object_t* cdr(object_t* sexp) {
	if (sexp == NULL || sexp == &nil)
		return &nil;
	if (sexp->type == CONS)
		return sexp->cdr;
	return &nil;
}

object_t* new_cons() {
	object_t* cell = new();
	cell->type = CONS;
	cell->car = NULL;
	cell->cdr = NULL;
	return cell;
}

object_t* cons(object_t* x, object_t* y) {

	object_t* cell = new_cons();
	cell->car = x;
	cell->cdr = y;
	return cell;
}

object_t* new_int(int x) {
	object_t* n = new();
	n->type = INT;
	n->integer = x;
	return n;
}

object_t* new_sym(char* s) {
	object_t* n = new();
	n->type = SYM;
	n->symbol = strdup(s);
	return n;
}


 int __eq(object_t* one, object_t* two) {
 	
	if (one->type != two->type) 
		return 0;

	switch(one->type) {
		case INT:
			return one->integer == two->integer;
		case SYM:
			return strcmp(one->symbol, two->symbol) == 0;
		case BOOL:
			return one->boolean == two->boolean;

	}
	return 0;
}

object_t* eq(object_t* x, object_t* y) {
	return (__eq(x,y)) ? &C_TRUE : &C_FALSE;
}

void print_object(object_t* obj) {
	if (!obj) {
		printf("<null>");
		return;
	}

	if (obj == &nil) {
		printf("NIL");
		return;
	}
	
	switch(obj->type) {

		case PRIM:
			printf("<prim>");
			break;
		case PROC:
			printf("<proc> %s", obj->name->symbol);
			print_object(obj->parameters);
			printf(" [");
			print_object(obj->body);
			printf("]");
			break;
		case INT:
			printf("%d", obj->integer);
			break;
		case SYM:
			printf("%s", obj->symbol);
			break;
		case BOOL:
			if (obj->boolean)
				printf("#t");
			else
				printf("#f");
			break;
		case CONS: {
			printf("(");

			object_t** t = &obj;

			while(*t) {

				print_object((*t)->car);
				
				if ((*t)->cdr) {
					printf(" ");
					if ((*t)->cdr->type == CONS) 
						t = &(*t)->cdr;
					else{
						print_object((*t)->cdr);
						break;
					}
					
				} else
					break;

			}

			printf(")");
		}

	}
}

object_t* tok_to_obj(char* token) {
	if (!token) return;
	int string = 0;
	int quote = 0;
//	printf("token: %s]\n", token);
s:
	switch(*token) {
		case '#':
			token++;
			if (*token == 't')
				return &C_TRUE;
			if (*token == 'f')
				return &C_FALSE;
			goto s;
			
		case '(':
		case ')':
			break;
		case '\"':
			string = !string;
			token++;
			goto s;
		case '\'':
			return new_sym("quote");
			

		case '0'...'9':
			if (string & !quote) {
				char* n = malloc(strlen(token) - 1);
				strncpy(n, token, strlen(token) - 1);
				return new_sym(n);
			}

			return new_int(atoi(token));
			break;
		default: {
			if (string || quote) {
				char* n = malloc((quote)? strlen(token) : strlen(token) - 1);
				strncpy(n, token, (quote)? strlen(token) : strlen(token) - 1);
				return new_sym(n);
			}
			return new_sym(token);
		}
	}	

	return NULL;
}

/*
object_t* cons(object_t* x, object_t* y) {
	object_t* cell = new_object(CONS, 1);
	cell->car = x;
	cell->cdr = new_object(CONS, 1);
	cell->cdr->car = y;
	cell->cdr->cdr = NULL;
	return cell;
}
*/

struct obj_list {
	object_t* val;
	struct obj_list* next;
	struct obj_list* prev;
};

struct obj_list* new_ol() {
	struct obj_list* ol = malloc(sizeof(struct obj_list));
	ol->val = new_cons();
	return ol;
}

struct obj_list* ol_push(struct obj_list** ol) {
	if (!ol)
		return NULL;

	(*ol)->next = new_ol();
	(*ol)->next->prev = (*ol);
	(*ol) = (*ol)->next;
	//(*ol)->val = obj;
	
	return *ol;
}

struct obj_list* ol_pop(struct obj_list** ol) {
	(*ol)->next = (*ol);
	(*ol) = (*ol)->prev;
	return *ol;
}

void push(object_t* list, object_t* new) {
	object_t** tmp;
	if (list == NULL || list->type != CONS)
		return;
	if (list->car == NULL) {
		list->car = new;
		return;
	}
	for (tmp = &list; *tmp; tmp=&(*tmp)->cdr) 
		;
	(*tmp) = new_cons();
	(*tmp)->car = new;
}




/* Accept a character string and split it into separate string tokens based on 
delim, escape. Return number of arguments in _argc */
object_t* scan(const char* str) {
	if (!str)
		return NULL;
	int i, idx = 0;
	int d = 0;
	int literal = 0;
	char word[128];
	memset(word, 0, 128);
	struct obj_list* ol = new_ol();
	//printf("input: %s\n", str);
	/* second pass, split strings by the delimiter */
	for (i = 0; i < strlen(str); i++) {
		switch(str[i]) {
			case ' ':
				if (str[i + 1] == ' ')
					continue;
				word[idx] = '\0';
				if (idx) 
					push(ol->val, tok_to_obj(word));
					//ol->val = (ol->val) ? cons(ol->val, tok_to_obj(word)) : tok_to_obj(word);

				idx = 0;
				continue;
			case '\'':
				break;
			case '\n':
			case '\0':
				word[idx] = '\0';
				if (idx) 
					push(ol->val, tok_to_obj(word));
				idx = 0;
				break;
			case '(':
				literal = 1;
				d++;
				word[idx] = '\0';
				if (idx) 
					push(ol->val, tok_to_obj(word));

				if (d>1) {
					ol_push(&ol);
					push(ol->prev->val, ol->val);
					//ol->prev->val = cons(ol->prev->val, ol->val);
				}
				idx = 0;
				continue;
			case ')':
				d--;
				word[idx] = '\0';
				if (idx)
					push(ol->val, tok_to_obj(word));
					//ol->val = cons(ol->val, tok_to_obj(word));
				if(d>0)
					ol_pop(&ol);
				idx=0;	
				continue;
			default:
				word[idx++] = str[i];
				continue;


		}
	}
	word[idx] = '\0';
	//if (ol->val->cdr == NULL)
	if(!literal)
		return ol->val->car;
	if (idx) 
		push(ol->val, tok_to_obj(word));

	return ol->val; 
}




struct env {
	object_t* alist;
	struct env* prev;
	struct env* next;
	struct env* head;
} *global_env;


struct env* env_new() {
	struct env* env = malloc(sizeof(struct env));
	env->alist = new_cons();
	env->prev = env;
	env->head = env;
	env->next = NULL;
}



void env_pop() {
	global_env = global_env->prev;
	global_env->next = NULL;
}

object_t* assoc(object_t* key, object_t* alist) {
	object_t** tmp;
	for (tmp = &alist; *tmp; tmp=&(*tmp)->cdr)  {
		if (car(car((*tmp))) == key)
			return cdr(car(*tmp));
		if (!strcmp((car(car(*tmp)))->symbol, key->symbol))
			return cdr(car(*tmp));
	}
	return NULL;	
}

object_t* env_lookup(object_t* key) {
	struct env** env;
	for (env = &global_env; *env != global_env->head; env=&(*env)->prev) {
		object_t* ret = assoc(key, (*env)->alist);
		if (ret)
			return ret;
	}
	return assoc(key, (*env)->alist);
}



void env_insert(object_t* key, object_t* val) {

	object_t** tmp;
	if (global_env->alist->car == NULL) {
		global_env->alist->car = cons(key, val);
		return;
	}
	for (tmp = &global_env->alist; *tmp; tmp=&(*tmp)->cdr) {
		if (car(car((*tmp))) == key) {
			(*tmp)->car->cdr = val;
			return;
		}
			
		if (!strcmp((car(car(*tmp)))->symbol, key->symbol)){
			(*tmp)->car->cdr = val;
			return;
		}
	}
		
	(*tmp) = new_cons();
	(*tmp)->car = cons(key, val);
	//(*tmp)->cdr = new_cons();
	//(*tmp) = global_env->alist->cdr;
}

void env_push() {
	global_env->next = env_new();
	global_env->next->prev = global_env;
	global_env->next->head = global_env->head;
	global_env = global_env->next;	
}


void env_list() {
	struct env** env;
	for (env = &global_env->head; *env ; env=&(*env)->next) {
		print_object((*env)->alist);
		printf("\n");
			
	}
}
void env_init() {
	global_env = env_new();
	env_insert(new_sym("#t"), &C_TRUE);
	env_insert(new_sym("#f"), &C_FALSE);
	env_insert(new_sym("nil"), &nil);
}

object_t* mkproc(object_t* name, object_t* params, object_t* body) {
	object_t* p = new();
	p->type = PROC;
	p->name = name;
	p->parameters = params;
	p->body = body;
	return p;
}
object_t* mkprim(primitive_t prim) {
	object_t* p = new();
	p->type = PRIM;
	p->primitive = prim;
	return p;
}

object_t* add(object_t* sexp) {
	int total;
	object_t** tmp;
	for (tmp = &sexp; *tmp; tmp=&(*tmp)->cdr)
		total += eval(car(*tmp))->integer;
	return new_int(total);
}

object_t* appq(object_t* sexp) {
	if (null(sexp))
		return &nil;
	if (atom(sexp))
		return cons(new_sym("quote"), sexp);
	return cons(cons(new_sym("quote"), car(sexp)), appq(cdr(sexp)));
}

void apply_param(object_t* param, object_t* args) {
	object_t** t;

	if (param->type == CONS) {
		if (atom(param)) {
			env_insert(car(param), car(args));
			return;
		}
		for (t = &param; *t; t=&(*t)->cdr) {
			//printf("arg insert");
			env_insert((*t)->car, args->car);
			args = args->cdr;
		}
	} else {
		//printf("arg insert no cons");
		env_insert(param, args);
	}
}

object_t* apply(object_t* f, object_t* args) {
	if (null(f))
		return &nil;
	if (f->type == PRIM) {
		return f->primitive((args));
	}
	if (f->type == PROC) {
		env_push();

		apply_param(f->parameters, (args));
		//env_list();
		object_t* ret = (atom(car(f->body))) ? eval(f->body) : evlis(f->body);
		env_pop();
		return ret;
	}
}

object_t* evlis(object_t* sexp) {
	if (null(sexp))
		return &nil;
	object_t* first = eval(car(sexp));
	return cons(first, evlis(cdr(sexp)));
}

object_t* evcon(object_t* cond) {
	if (eval(caar(cond)) == &C_TRUE)
		return eval(cadar(cond));
	return eval(caddar(cond));
}

object_t* eval(object_t* sexp) {
	if (null(sexp))
		return &nil;

			//printf("[eval]: ");
		//	print(sexp);

	switch(sexp->type) {
		case INT:
			return sexp;
		case SYM: {

			object_t* ret = env_lookup(sexp);
			if (null(ret)) {
				printf("Unbound variable!");
				print(sexp);
				return &nil;
			}
			while(ret->type == SYM)
				ret = env_lookup(ret);
			return eval(ret);
		} case CONS: {
			/*atom [car [e]] → [
			eq [car [e]; QUOTE] → cadr [e];
			eq [car [e]; ATOM] → atom [eval [cadr [e]; a]];
			eq [car [e]; EQ] → [eval [cadr [e]; a] = eval [caddr [e]; a]];
			eq [car [e]; COND] → evcon [cdr [e]; a];
			eq [car [e]; CAR] → car [eval [cadr [e]; a]];
			eq [car [e]; CDR] → cdr [eval [cadr [e]; a]];
			eq [car [e]; CONS] → cons [eval [cadr [e]; a]; eval [caddr [e]; a]]; 
			T → eval [cons [assoc [car [e]; a]; evlis [cdr [e]; a]]; a]
			];
			*/
			if (atom(sexp->car)) {

				if(car(sexp)->type == SYM) {
					if (!strcmp(sexp->car->symbol, "define")) {
						if (atom(cadr(sexp))) {
							env_insert(cadr(sexp), (caddr(sexp)));
							return new_sym("ok");
						} else {
							print(sexp);

							object_t* proc = mkproc(car(cadr(sexp)), cdr(cadr(sexp)), caddr(sexp));
							env_insert(car(cadr(sexp)), proc);
							return env_lookup(car(cadr(sexp)));
						}
					}
					if (!strcmp(car(sexp)->symbol, "nil"))
						return &nil;
					if (!strcmp(sexp->car->symbol, "cons")) 
						return cons(eval(cadr(sexp)), eval(caddr(sexp)));
					if (!strcmp(sexp->car->symbol, "car"))
						return car(eval(cadr(sexp)));
					if (!strcmp(sexp->car->symbol, "cdr")) 
						return cdr(eval(cadr(sexp)));
					if (!strcmp(sexp->car->symbol, "quote")) {
						return cadr(sexp);
					}
					if (!strcmp(sexp->car->symbol, "eq")) 
						return eq(eval(cadr(sexp)), eval(caddr(sexp)));
					if (!strcmp(sexp->car->symbol, "atom")) 
						return atom(eval(cadr(sexp))) ? &C_TRUE : &C_FALSE;
					if (!strcmp(sexp->car->symbol, "cond")) 
						return evcon(cdr(sexp));
					if (!strcmp(sexp->car->symbol, ">"))
						return eval(cadr(sexp))->integer > eval(caddr(sexp))->integer ?&C_TRUE : &C_FALSE;
					if (!strcmp(sexp->car->symbol, "<"))
						return eval(cadr(sexp))->integer < eval(caddr(sexp))->integer ?&C_TRUE : &C_FALSE;
					if (!strcmp(sexp->car->symbol, "="))
						return eval(cadr(sexp))->integer == eval(caddr(sexp))->integer ?&C_TRUE : &C_FALSE;
					if (!strcmp(car(sexp)->symbol, "+"))
						return apply(mkprim(add), cdr(sexp));
					if (!strcmp(car(sexp)->symbol, "lambda")) {
						return mkproc(car(sexp), cadr(sexp), caddr(sexp));
					}
		
				}
				print((cons(env_lookup(car(sexp)), evlis(cdr(sexp)))));

				object_t* ret = apply(eval(car(sexp)), evlis(cdr(sexp)));
				while(!null(ret) && ret->type == PROC)
					ret = apply(ret, evlis(cdr(sexp)));
				return ret;
			}

		
		/*
			eq [caar [e]; LABEL] → eval [cons [caddar [e]; cdr [e]];
			cons [list [cadar [e]; car [e]; a]];
			eq [caar [e]; LAMBDA] → eval [caddar [e]
;			append [pair [cadar [e]; evlis [cdr [e]; a]; a]]]
	*/

			if (car(car(sexp))->type == SYM) {
				if (!strcmp(caar(sexp)->symbol, "label")) {
					printf("EVAL:");
					print(cons(cadar(sexp), cdr(sexp)));
					printf("ASSOC");
					print(cons(cadar(sexp), car(sexp)));

					env_insert(cadar(sexp), car(sexp));
					env_list();

				}

				if (!strcmp(caar(sexp)->symbol, "quote")) {
					return (cdr(car(sexp)));
				}
				if (!strcmp(caar(sexp)->symbol, "lambda")) {
					return apply(mkproc(car(car(sexp)), cadar(sexp), caddar(sexp)), cdr(sexp));
				}
				object_t* pr = env_lookup(caar(sexp));

				if (null(pr)) {
					printf("Unbound variable!\n");
					return &nil;
				}

				/* Stacking two procedures */
				if (!null(pr) && pr->type == PROC) {
						return apply(apply(pr, cdr(car(sexp))), cdr(sexp));
				}

				
				//if LABEL?
				// eval (cons (caddar sexp) (cdr sexp))
				//if LAMBDA?
				// eval (caddar sexp)
			}
			return car(car(sexp));
		}
		default:
			printf("Weird object eval:\n");
			print(sexp);
	}

}

int main(int argc, char** argv) {
	env_init();


	// eval(scan("(define w (cons 1 2))"));
	// print(eval(scan("((lambda (x) (car x)) w)")));
	eval(scan("(define x (cons 3 (cons 1 2))"));
	eval(scan("(define (swap list)\n(cons (cdr list) (car list)))"));
	eval(scan("(define (two n) (define (xx item)(cons 1 (car item)) (xx n))"));
	eval(scan("(define (if predicate then else) (cond ((predicate) then else)))"));
	eval(scan("(define (length list) (cond ((atom list) 1 (+ 1 (length (cdr list))))))"));
	print((scan("(1 2 3)")));
	print(evlis(scan("(x 2 3)")));
	char* input = malloc(256);
	size_t sz;
	do {
		printf("user> ");
		getline(&input, &sz, stdin);
		if (*input == 'q')
			break;
		printf("=> ");
		print(eval(scan(input)));

	} while(*input);
}