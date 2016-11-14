#ifndef __lisp__
#define __lisp__


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


static object_t C_TRUE = {.type = BOOL, .boolean = true};
static object_t C_FALSE = {.type = BOOL, .boolean = false};
static object_t UNBOUND = { .type = SYM, .symbol = "Unbound variable!"};

extern object_t* eval(object_t* sexp, object_t**);
extern object_t* evlis(object_t* sexp, object_t**);
extern object_t* evcon(object_t* cond, object_t**) ;
extern object_t* appq(object_t*);
extern int atom(object_t* sexp);
extern void print_object(object_t*);
extern object_t* append(object_t* pair, object_t* list);
extern object_t* pair(object_t* x, object_t* y) ;

extern object_t* scan(const char* str);
extern object_t* car(object_t* sexp) ;
extern object_t* cdr(object_t* sexp);
extern object_t* new_cons();
extern object_t* list(int count, ...);
extern object_t* cons(object_t* x, object_t* y);
extern object_t* new_sym(char* s) ;
extern object_t* new_int(int x) ;

#endif