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

#define error(x) do { fprintf(stderr, "%s\n", x); exit(1); } while(0);
#define cadr(x) (car(cdr(x)))
#define cddr(x) (cdr(cdr(x)))
#define caddr(x) (car(cddr(x)))
#define cadar(x) (car(cdr(car((x)))))
#define caddar(x) (car(cdr(cdr(car((x))))))
#define caar(x) (car(car((x))))
#define print(x) do { print_object((x)); puts(""); } while(0);
#define new() (malloc(sizeof(struct object)))
#define null(x) ((x) == NULL || (x) == &nil )

typedef enum type { INT, SYM, BOOL, CONS, PROC, PRIM } type_t;
typedef struct object* (*primitive_t) (struct object*);
typedef struct object object_t;
typedef struct cons cons_t;

struct object {
	type_t type;
	union {
		int integer;
		char* symbol;
		bool boolean;
		struct {
			struct object* car;
			struct object* cdr;
		};
	};
} nil;

struct obj_list {
	object_t* val;
	struct obj_list* next;
	struct obj_list* prev;
};


object_t C_TRUE = {.type = BOOL, .boolean = true};
object_t C_FALSE = {.type = BOOL, .boolean = false};
object_t UNBOUND = { .type = SYM, .symbol = "Unbound variable!"};

object_t* eval(object_t* sexp, object_t*);
object_t* evlis(object_t* sexp, object_t*);
object_t* evcon(object_t* cond, object_t*) ;
object_t* appq(object_t*);
int atom(object_t* sexp);
void print_object(object_t*);
object_t* append(object_t* pair, object_t* list);
object_t* pair(object_t* x, object_t* y) ;


int atom(object_t* sexp) {
	return (null(sexp) || sexp->type != CONS);
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
		if (i != count - 1)	{
			list->cdr = new_cons();
			list->cdr->cdr = &nil;
			list = list->cdr;
		} else
			list->cdr = &nil;
	}
	va_end(ap);
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


 int eq(object_t* one, object_t* two) {
  	if (one == two)
 		return 1;
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

void print_object(object_t* obj) {
	if (null(obj)) {
		printf("NIL");
		return;
	}	
	switch(obj->type) {
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
	if (null(list) || list->type != CONS)
		return;
	if (null(list->car)) {
		list->car = new;
		return;
	}
	for (tmp = &list->cdr; *tmp; tmp=&(*tmp)->cdr) 
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
	int quoted = 0;
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
				quoted = 1;
				if (str[i + 1] == '(') {
					//d++;
					ol_push(&ol);
					push(ol->val, new_sym("quote"));
					push(ol->prev->val, ol->val);
				}
				else {
					push(ol->val, new_sym("quote"));
					if (idx) 
						push(ol->val, tok_to_obj(word));
					//ol->val = (ol->val) ? cons(ol->val, tok_to_obj(word)) : tok_to_obj(word);

				}
				
				idx = 0;
				continue;
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

object_t* eval(object_t* sexp, object_t* env) {
	if (null(sexp))
		return &nil;
	switch(sexp->type) {
		case INT:
			return sexp;
		case SYM: {
			object_t* ret = assoc(sexp, env);
			return ret;
		} case CONS: {
			if (atom(sexp->car)) {
				if(car(sexp)->type == SYM) {
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
						return eq(eval(cadr(sexp), env), eval(caddr(sexp), env)) ? &C_TRUE : &C_FALSE;
					if (!strcmp(sexp->car->symbol, "atom")) 
						return atom(eval(cadr(sexp), env)) ? &C_TRUE : &C_FALSE;
					if (!strcmp(sexp->car->symbol, "cond")) 
						return evcon(cdr(sexp), env);
				}
				return eval(cons(assoc(car(sexp), env), evlis(cdr(sexp), env)), env);
			}
			if (car(car(sexp))->type == SYM) {
				if (!strcmp(caar(sexp)->symbol, "label")) 
					return eval(cons(caddar(sexp), (cdr(sexp))), cons(list(2, cadar(sexp), car(sexp)), env));
				if (!strcmp(caar(sexp)->symbol, "lambda")) {
					object_t* p = pair(cadar(sexp), (evlis(cdr(sexp), env)));
					return eval(caddar(sexp), (append(p, env)));
				}
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

int main(int argc, char** argv) {
	char* input = malloc(256);
	object_t* env = new_cons();
	env = append(pair(cons(new_sym("t"), &nil), cons(&C_TRUE, &nil)), env);
	env = append(pair(cons(new_sym("f"), &nil), cons(&C_FALSE, &nil)), env);
	env = append(pair(cons(new_sym("nil"), &nil), cons(&nil, &nil)), env);
	print(env);
	printf("Welcome to microLISP. Press ! to exit\n");
	size_t sz;
	do {
		printf("user> ");
		getline(&input, &sz, stdin);
		if (*input == '!')
			break;
		printf("=> ");
		print(eval(scan(input), env));
	} while(*input);
	return 0;
}