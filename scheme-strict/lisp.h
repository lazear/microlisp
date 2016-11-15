#ifndef __lisp__
#define __lisp__

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#define error(x) do { fprintf(stderr, "%s\n", x); exit(1); } while(0);
#define cadr(x) (car(cdr(x)))
#define cddr(x) (cdr(cdr(x)))
#define caddr(x) (car(cddr(x)))
#define cadddr(x) (car(cdr(cddr(x))))
#define cadar(x) (car(cdr(car((x)))))
#define caddar(x) (car(cdr(cdr(car((x))))))
#define caar(x) (car(car((x))))
#define print(x) do { print_object((x)); puts(""); } while(0);
#define new() (malloc(sizeof(struct object)))
#define null(x) ((x) == NULL || (x) == &nil )
#define arity(x) (__number_of_args(__func__, sexp, x))

typedef enum type { INT, SYM, STRING, BOOL, CONS, PROC, PRIM } type_t;
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
		primitive_t primitive;
	};
} nil;

struct obj_list {
	object_t* val;
	struct obj_list* next;
	struct obj_list* prev;
};


static object_t C_TRUE = {.type = BOOL, .boolean = true};
static object_t C_FALSE = {.type = BOOL, .boolean = false};
static object_t UNBOUND = { .type = SYM, .symbol = "Unbound variable!"};

static object_t* lambda;
static object_t* quote;


/* scheme.c */
extern object_t* eval(object_t* sexp, object_t*);
extern object_t* evlis(object_t* sexp, object_t*);
extern object_t* evcon(object_t* cond, object_t*) ;
extern object_t* appq(object_t*);
extern object_t* append(object_t* pair, object_t* list);
extern object_t* pair(object_t* x, object_t* y) ;

/* core.c */
extern int atom(object_t* sexp);
extern void print_object(object_t*);
extern object_t* scan(const char* str);
extern object_t* car(object_t* sexp) ;
extern object_t* cdr(object_t* sexp);
extern object_t* new_cons();
extern object_t* cons(object_t* x, object_t* y);
extern object_t* list(int count, ...);
extern object_t* new_sym(char* s) ;
extern object_t* new_int(int x);

/* primitives.c */
extern void __number_of_args(const char* func, object_t* sexp, int expected);

extern object_t* add(object_t* first, object_t* rest);
extern object_t* sub(object_t* first, object_t* rest);
extern object_t* imul(object_t* first, object_t* rest);
extern object_t* idiv(object_t* first, object_t* rest);

#endif