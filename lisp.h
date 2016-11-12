#ifndef __lisp__
#define __lisp__


#define error(x) do { fprintf(stderr, "%s\n", x); exit(1); } while(0);
// #define eq(x, y) ((atom(x) && atom(y))
#define atom(x) ((x)->type != CONS || (x)->cdr == NULL || (x)->cdr == &nil)
// #define car(x) (atom(x) ? NULL : (x)->car)
#define cadr(x) (car(cdr(x)))
#define cddr(x) (cdr(cdr(x)))
#define caddr(x) (car(cddr(x)))
#define cadar(x) (car(cdr(car((x)))))
#define caddar(x) (car(cdr(cdr(car((x))))))
#define caar(x) (car(car((x))))
#define print(x) do { print_object(x); puts(""); } while(0);
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
		/* cons */
		struct {
			struct object* car;
			struct object* cdr;
		};
		/* procedure */
		struct {
			struct object* name; 
			struct object* parameters; 
			struct object* body;
		};
		/* primitive */
		primitive_t primitive;

	};
} nil;

object_t C_TRUE = {.type = BOOL, .boolean = true};
object_t C_FALSE = {.type = BOOL, .boolean = false};

extern object_t* eval(object_t* sexp);
extern object_t* evlis(object_t* sexp);
extern object_t* evcon(object_t* cond) ;
extern object_t* apply(object_t*, object_t*);
extern object_t* appq(object_t*);
#endif 