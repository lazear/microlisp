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



int atom(object_t* sexp) {

	if (null(sexp))
		return 1;
	if (sexp->type == CONS) {
		return 0;
		if (null(sexp->cdr))
			return 1;
		if (sexp->cdr->type == CONS)
			return 0;
	}
	return 1;
}

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

object_t* list(int count, ...) {
	va_list ap;
	va_start(ap, count);
	int i = 0;
	object_t* list = new_cons();
	object_t* ret = list;
	for (i = 0; i < count; i++) {
		list->car = va_arg(ap, object_t*);
		if (i != count - 1)
		{
			list->cdr = new_cons();
			list->cdr->cdr = &nil;
			list = list->cdr;
		} 
		else
			list->cdr = &nil;
	}
	return ret;
}


object_t* cons(object_t* x, object_t* y) {

	object_t* cell = new_cons();
	cell->type = CONS;
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
	env_insert(new_sym("t"), &C_TRUE);
	env_insert(new_sym("f"), &C_FALSE);
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

object_t* eval(object_t* sexp, object_t* env) {
	if (null(sexp))
		return &nil;

	switch(sexp->type) {
		case INT:
			return sexp;
		case SYM: {
			return assoc(sexp, env);
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
					if (!strcmp(car(sexp)->symbol, "define")){

						if (atom(cadr(sexp))) {
							env_insert(cadr(sexp), eval(caddr(sexp), env));
						} else {
													//print(cons(new_sym("lambda"), cons(cdr(cadr(sexp)), caddr(sexp))));
							object_t* l = list(3, new_sym("lambda"), cdr(cadr(sexp)), caddr(sexp));
							print(l);
							env_insert(car(cadr(sexp)), l);
						//	env_insert(car(cadr(sexp)), cons(new_sym("label"), cons(car(cadr(sexp)), cons(l, &nil))));
						}
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
					if (!strcmp(sexp->car->symbol, "quote"))
						return cadr(sexp);
					if (!strcmp(sexp->car->symbol, "eq")) 
						return eq(eval(cadr(sexp), env), eval(caddr(sexp), env));
					if (!strcmp(sexp->car->symbol, "atom")) 
						return atom(eval(cadr(sexp), env)) ? &C_TRUE : &C_FALSE;
					if (!strcmp(sexp->car->symbol, "cond")) 
						return evcon(cdr(sexp), env);
		
				}
				return eval(cons(assoc(car(sexp), env), evlis(cdr(sexp), env)), env);
			}
			
/*
			((label ff (lambda (ar dr) ar)) 1 2)
			((label swap (lambda (list) (cons (cdr list) (car list)))) (cons 1 2))
			(LABEL, FF, (LAMBDA, (X), (COND, (ATOM, X), X), ((QUOTE,T),(FF, (CAR, X))))));((A· B))
			((label ff (lambda (x) (cond (atom x) x f))) (quote (1 2))

			eq [caar [e]; LABEL] → eval [cons [caddar [e]; cdr [e]]; cons [list [cadar [e]; car [e]; a]];
			eq [caar [e]; LAMBDA] → eval [caddar [e]; append [pair [cadar [e]; evlis [cdr [e]; a]; a]]]*/

			if (car(car(sexp))->type == SYM) {
				if (!strcmp(caar(sexp)->symbol, "label")) 
					return eval(cons(caddar(sexp), (cdr(sexp))), cons(list(2, cadar(sexp), car(sexp)), env));
				if (!strcmp(caar(sexp)->symbol, "lambda")) {
					object_t* p = pair(cadar(sexp), (evlis(cdr(sexp), env)));
					return eval(caddar(sexp), (append(p, env)));
				}
			}
		}
		default:
			printf("Unknown eval expression:\n");
			print(sexp);
	}

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
	// if (atom(sexp))
	// 	return cons(new_sym("quote"), sexp);
	return cons(cons(new_sym("quote"), car(sexp)), appq(cdr(sexp)));
}

object_t* apply(object_t* f, object_t* args) {
	
}


int main(int argc, char** argv) {
	env_init();
	char* input = malloc(256);
	object_t* env = new_cons();
	size_t sz;
	do {
		printf("user> ");
		getline(&input, &sz, stdin);
		if (*input == 'q')
			break;
		printf("=> ");
		print(eval(scan(input), env));

	} while(*input);
}