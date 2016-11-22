/* Single file scheme interpreter
MIT License
Copyright Michael Lazear (c) 2016 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define null(x) ((x) == NULL || (x) == NIL)
#define EOL(x) 	(null((x)) || (x) == EMPTY_LIST)
#define error(x) do { fprintf(stderr, "%s\n", x); exit(1); }while(0)
#define caar(x) (car(car((x))))
#define cdar(x) (cdr(car((x))))
#define cadr(x) (car(cdr((x))))
#define caddr(x) (car(cdr(cdr((x)))))
#define cadddr(x) (car(cdr(cdr(cdr((x))))))
#define cadar(x) (car(cdr(car((x)))))
#define cddr(x) (cdr(cdr((x))))
#define atom(x) (!null(x) && (x)->type != LIST)
#define type(x, t) (__type_check(__func__, x, t))

typedef enum { INTEGER, SYMBOL, STRING, LIST, PRIMITIVE } type_t;
typedef struct object* (*primitive_t)(struct object*);

/* Lisp object. We want to mimic the homoiconicity of LISP, so we will not be
providing separate "types" for procedures, etc. Everything is represented as
atoms (integers, strings, booleans) or a list of atoms */
typedef struct object object_t;
struct object {
	char gc;
	type_t type;
	union {
		int integer;
		char* string;
		struct {
			struct object* car;
			struct object* cdr;
		};
		primitive_t primitive;
	};
} __attribute__((packed));

/* We declare a couple of global variables as keywords */
static struct object* ENV;
static struct object* NIL;
static struct object* EMPTY_LIST;
static struct object* TRUE;
static struct object* QUOTE;
static struct object* DEFINE;
static struct object* SET;
static struct object* LET;
static struct object* IF;
static struct object* LAMBDA;
static struct object* BEGIN;
static struct object* PROCEDURE;

struct object* read_exp(FILE* in);
struct object* eval(struct object* exp, struct object* env);
struct object* cons(struct object* x, struct object* y);
struct object* load_file(struct object* args);
struct object* cdr(struct object*);
struct object* car(struct object*);
void print_exp(struct object*);
bool is_tagged(struct object* cell, struct object* tag);
struct object* lookup_variable(struct object* var, struct object* env);
/*==============================================================================
Hash table for saving memory
==============================================================================*/
struct htable {
	struct object* key;
};
/* One dimensional hash table */
static struct htable* HTABLE = NULL;
static int HTABLE_SIZE;

static uint64_t hash(const char* s) {
	uint64_t h = 0;
	uint8_t* u = (uint8_t*) s;
	while(*u) {
		h = (h * 256 + *u) % HTABLE_SIZE;
		u++;
	}
	return h;
}

int ht_init(int size) {
	if (HTABLE || !(size % 2))
		return 0;
	HTABLE = malloc(sizeof(struct htable) * size);
	memset(HTABLE, 0, sizeof(struct htable) * size);
	HTABLE_SIZE = size;
	return size;
}

void ht_insert(struct object* key) {
	uint64_t h = hash(key->string);
	HTABLE[h].key = key;
}

struct object* ht_lookup(char* s) {
	uint64_t h = hash(s);
	return HTABLE[h].key;
}


/*==============================================================================
  Memory management
==============================================================================*/

void* HEAP;
uint64_t HEAP_START;
uint64_t HEAP_TOP;
size_t FREE_SPACE;

void alloc_heap(size_t sz) {
	HEAP = malloc(sz);
	if (HEAP == NULL) 
		error("Out of memory!");
	memset(HEAP, 0, sz);
	FREE_SPACE = sz;
	HEAP_START = (uint64_t) HEAP;
	HEAP_TOP = HEAP_START+sz;
}

#define FREE 	0x0
#define ALLOCATED 		0x1
#define MARKED 			0x2
int alloc_count = 0;

struct object* alloc() {
	struct object* ret = malloc(sizeof(struct object));
	// struct object* ret = (struct object*) HEAP;
	// HEAP = (void*) HEAP_START;
	// while ((uint64_t) HEAP < HEAP_TOP) {
	// 	HEAP = (void*) ((uint64_t) HEAP + sizeof(struct object));
	// 	ret = (struct object*) HEAP;
	// 	if (ret->gc == FREE)
	// 		break;
	// }
	// ret->gc = ALLOCATED;
	// FREE_SPACE -= (sizeof (struct object));
	// if (FREE_SPACE < sizeof(struct object))
	// 	error("OUT OF Memory");
	// //printf("start: %x current %x\n", HEAP_START, HEAP);
	// alloc_count++;
	return ret;
}

void dealloc(struct object* obj) {
	if (obj->gc & MARKED)
		error("Trying to deallocated a marked object");
	if (!(obj->gc & ALLOCATED))
		error("Trying to dealloc a non-allocated object...");
	FREE_SPACE += sizeof(struct object);
	obj->gc = FREE;
}

/* Strategy is to traverse the global environment and mark all of the objects in use.
We then traverse the entire heap and dealloc those objects that were not marked */

int marked;
void mark(struct object* exp);

void mark_list(struct object* list) {
	struct object* tmp = list;
	while(!null(tmp)) {
		mark(car(tmp));
		tmp = cdr(tmp);
	}
}

void mark(struct object* exp) {
	if (null(exp))
		return;
	exp->gc = MARKED;
	marked++;
	if (exp->type == LIST) {
		if (is_tagged(exp, PROCEDURE)) {
			mark(cadr(exp));	/* params */
			mark(caddr(exp)); 	/* body */
			return;
		}
		mark_list(exp);
		return;
	}

	printf("\nmarking: ");
	print_exp(exp);
	return;
}

void gc_collect() {
	int free = 0;
	uint64_t trash_man;
	for (trash_man = HEAP_START; trash_man < HEAP; trash_man += sizeof(struct object)){
		struct object* g = (struct object*) (void*) trash_man;
		if (g->gc && (g->gc != MARKED)) {
			dealloc(g);
			print_exp(g);
			printf("\n");
			free++;
		}

	}
	printf("\nCurrent: %x Heap bottom: %x, Top %x, Size %lu\n", (uint64_t) HEAP, HEAP_START, HEAP_TOP, (HEAP_TOP - HEAP_START));
	printf("Collected %d/%d (%d marked) objects", free, alloc_count, marked);
}

void gc_traverse() {
	marked = 0;
	uint64_t trash_man;
	for (trash_man = HEAP_START; trash_man < HEAP; trash_man += sizeof(struct object)){
		struct object* g = lookup_variable((struct object*) (void*) trash_man, ENV);
		if (!null(g)) {
			mark(g);
			mark((struct object*) (void*) trash_man);
		}
	}
	//printf("marked a total of %d/%d items\n", marked, alloc_count);
	gc_collect();
	print_exp(ENV);
}



/*============================================================================
Constructors and etc
==============================================================================*/
int __type_check(const char* func, object_t* obj, type_t type) {
	if (null(obj)) {
		fprintf(stderr, "Invalid argument to function %s: NIL", func);
		exit(1);
	} else if (obj->type != type) {
		fprintf(stderr, "Invalid argument to function %s. Expected type %d got %d\n", func, type, obj->type);
		exit(1);
	}
	return 1;
}

struct object* make_symbol(char* s) {
	struct object* ret = ht_lookup(s);
	if (null(ret)) {
		ret =alloc();
		ret->type = SYMBOL;
		ret->string = strdup(s);
		ht_insert(ret);
	}	else 
	return ret;
}

struct object* make_integer(int x) {
	struct object* ret =alloc();
	ret->type = INTEGER;
	ret->integer = x;
	return ret;	
}

struct object* make_primitive(primitive_t x) {
	struct object* ret =alloc();
	ret->type = PRIMITIVE;
	ret->primitive = x;
	return ret;	
}

struct object* make_lambda(struct object* params, struct object* body) {
	return cons(LAMBDA, cons(params, body));
}

struct object* make_procedure(struct object* params, struct object* body, 
	struct object* env) {
	return cons(PROCEDURE, cons(params, cons(body, cons(env, EMPTY_LIST))));
}

struct object* cons(struct object* x, struct object* y) {
	struct object* ret =alloc();
	ret->type = LIST;
	ret->car = x;
	ret->cdr = y;
	return ret;
}

struct object* car(struct object* cell) {
	if (null(cell) || cell->type != LIST)
		return NIL;
	return cell->car;
}

struct object* cdr(struct object* cell) {
	if (null(cell) || cell->type != LIST)
		return NIL;
	return cell->cdr;
}

struct object* append(struct object* l1, struct object* l2) {
	if (null(l1))
		return l2;
	return cons(car(l1), append(cdr(l1), l2));
}

struct object* reverse(struct object* list, struct object* first) {
	if (null(list))
		return first;
	return reverse(cdr(list), cons(car(list), first));

}

bool is_equal(struct object* x, struct object* y) {

	if (x == y)
		return true;
	if (null(x) || null(y))
		return false;
	if (x->type != y->type)
		return false;
	switch(x->type) {
		case LIST:
			return false;
		case INTEGER:
			return x->integer == y->integer;
		case SYMBOL: case STRING:
			return !strcmp(x->string, y->string);
	}
	return false;
}

bool is_tagged(struct object* cell, struct object* tag) {
	if (null(cell) || cell->type != LIST)
		return false;
	return is_equal(car(cell), tag);
}
struct object* prim_get_env(struct object* args) {
	return ENV;
}
struct object* prim_set_env(struct object* args) {
	ENV = car(args);
	return NIL;
}

struct object* prim_list(struct object* args) {
	return (args);
}
struct object* prim_cons(struct object* args) {
	return cons(car(args), cadr(args));
}

struct object* prim_car(struct object* args) {
	return caar(args);
}

struct object* prim_setcar(struct object* args) {
	type(car(args), LIST);
	(args->car->car = (cadr(args)));
	return (car(args));
}
struct object* prim_setcdr(struct object* args) {
	type(car(args), LIST);
	(args->car->cdr = (cadr(args)));
	return (car(args));
}

struct object* prim_cdr(struct object* args) {
	return cdar(args);
}
struct object* prim_nullq(struct object* args) {
	return EOL(car(args)) ? TRUE : NIL;
}

struct object* prim_pairq(struct object* args) {
	return (car(args)->type == LIST) ? TRUE : NIL;
}

struct object* prim_listq(struct object* args) {
	struct object* list;
	if (car(args)->type != LIST)
		return NIL;
	for (list = car(args); !null(list); list = list->cdr)
		if (!null(list->cdr) && (list->cdr->type != LIST))
			return NIL;
	return (car(args)->type == LIST && prim_pairq(args) != TRUE) ? TRUE : NIL;
}

struct object* prim_atomq(struct object* sexp) {
	return atom(car(sexp)) ? TRUE : NIL;
}

struct object* prim_eq(struct object* args) {
	return is_equal(car(args), cadr(args)) ? TRUE : NIL;
}

struct object* prim_add(struct object* list) {
	type(car(list), INTEGER);
	int total = car(list)->integer;
	list = cdr(list);
	while (!EOL(car(list))) {
		type(car(list), INTEGER);
		total += car(list)->integer;
		list = cdr(list);
	}
	return make_integer(total);
}

struct object* prim_sub(struct object* list) {
	type(car(list), INTEGER);
	int total = car(list)->integer;
	list = cdr(list);
	while (!null(list)) {
		type(car(list), INTEGER);
		total -= car(list)->integer;
		list = cdr(list);
	}
	return make_integer(total);
}

struct object* prim_div(struct object* list) {
	type(car(list), INTEGER);
	int total = car(list)->integer;
	list = cdr(list);
	while (!null(list)) {
		type(car(list), INTEGER);
		total /= car(list)->integer;
		list = cdr(list);
	}
	return make_integer(total);
}

struct object* prim_mul(struct object* list) {
	type(car(list), INTEGER);
	int total = car(list)->integer;
	list = cdr(list);
	while (!null(list)) {
		type(car(list), INTEGER);
		total *= car(list)->integer;
		list = cdr(list);
	}
	return make_integer(total);
}
struct object* prim_gt(struct object* sexp) {
	return (car(sexp)->integer > cadr(sexp)->integer) ? TRUE : NIL;
}

struct object* prim_lt(struct object* sexp) {
	return (car(sexp)->integer < cadr(sexp)->integer) ? TRUE : NIL;
}


/*==============================================================================
Environment handling
==============================================================================*/

struct object* extend_env(struct object* var, struct object* val, struct object* env) {
	return cons(cons(var, val), env);
}

struct object* lookup_variable(struct object* var, struct object* env) {
	while (!null(env)) {
		struct object* frame = car(env);
		struct object* vars 	= car(frame);
		struct object* vals 	= cdr(frame);
		while(!null(vars)) {
			if (is_equal(car(vars), var))
				return car(vals);
			vars = cdr(vars);
			vals = cdr(vals);
		}
		env = cdr(env);
	}
	return NIL;
}

/* set_variable binds var to val in the first frame in which var occurs */
void set_variable(struct object* var, struct object* val, struct object* env) {
	while (!null(env)) {
		struct object* frame = car(env);
		struct object* vars 	= car(frame);
		struct object* vals 	= cdr(frame);
		while(!null(vars)) {
			if (is_equal(car(vars), var)) {
				vals->car = val;
				return;
			}
			vars = cdr(vars);
			vals = cdr(vals);
		}
		env = cdr(env);
	}
}

/* define_variable binds var to val in the *current* frame */
struct object* define_variable(struct object* var, struct object* val, struct object* env) {
	struct object* frame = car(env);
	struct object* vars = car(frame);
	struct object* vals = cdr(frame);

	while(!null(vars)) {
		if (is_equal(var, car(vars))) {
			vals->car = val;
			return;
		}
		vars = cdr(vars);
		vals = cdr(vals);
	}
	frame->car = cons(var, car(frame));
	frame->cdr = cons(val, cdr(frame));
	return val;
}


/*==============================================================================
Recursive descent parser 
==============================================================================*/

char SYMBOLS[] = "~!@#$%^&*_-+\\:,.<>|{}[]?=";

int peek(FILE* in) {
	int c = getc(in);
	ungetc(c, in);
	return c;
}

/* skip characters until end of line */
void skip(FILE* in) {
	int c;
	for(;;) {
		c = getc(in);
		if (c == '\n' || c == EOF)
			return;
	}
}

struct object* read_string(FILE* in) {
	char buf[256];
    int i = 0;
    int c;
    while ((c = getc(in)) != '\"') {
    	if (c == EOF)
    		return NIL;
        if (i >= 256)
            error("String too long");
        buf[i++] = (char)c;
    }
    buf[i] = '\0';
    struct object* s = make_symbol(buf);
    s->type = STRING;
    return s;
}


struct object* read_symbol(FILE* in, char start) {
	char buf[128];
	buf[0] = start;
    int i = 1;
    while (isalnum(peek(in)) || strchr(SYMBOLS, peek(in))) {
        if (i >= 128)
            error("Symbol name too long");
        buf[i++] = getc(in);
    }
    buf[i] = '\0';
    return make_symbol(buf);
}

int read_int(FILE* in, int start) {
	while(isdigit(peek(in))) 
		start = start * 10 + (getc(in) - '0');
	return start;
}

struct object* read_list(FILE* in) {
	struct object* obj;
	struct object* cell = EMPTY_LIST;
	for (;;) {
		obj = read_exp(in);
		
		if (obj == EMPTY_LIST)
			return reverse(cell, EMPTY_LIST);
		cell = cons(obj, cell);
	}
	return EMPTY_LIST;
}

struct object* read_quote(FILE* in) {
	return cons(QUOTE, cons(read_exp(in), NIL));
}

int depth = 0;
struct object* read_exp(FILE* in) {
	int c;
	for(;;) {
		c = getc(in);
		if (c == '\n' || c== '\r' || c == ' ' || c == '\t') {
			if ((c== '\n' || c == '\r') && in == stdin) {
				int i; 
				for (i = 0; i < depth; i++)
					printf("..");	
			}
			continue;
		}
		if (c == ';') {
			skip(in);
			continue;
		}
		if (c == EOF)
			return NULL;
		if (c == '\"')
			return read_string(in);
		if (c == '\'')
			return read_quote(in);
		if (c == '(') {
			depth++;
			return read_list(in);
		}
		if (c == ')') {
			depth--;
			return EMPTY_LIST;
		}
		if (isdigit(c))
			return make_integer(read_int(in, c - '0'));
		if (c == '-' && isdigit(peek(in))) 
			return make_integer(-1 * read_int(in, getc(in) - '0'));
		if (isalpha(c) || strchr(SYMBOLS, c)) 
			return read_symbol(in, c);
	}
	return NIL;
}

void print_exp(struct object* e) {
	if (null(e)) {
		printf("'()");
		return;
	}
	switch(e->type) {
		case STRING:
			printf("\"%s\"", e->string);
			break;
		case SYMBOL:
			printf("%s", e->string);
			break;
		case INTEGER:
			printf("%d", e->integer);
			break;
		case PRIMITIVE:
			printf("<function>");
			break; 
		case LIST:
			if (is_tagged(e, PROCEDURE)) {
				printf("<closure>");
				return;
			}
			printf("(");
			struct object** t = &e;
			while(!null(*t)) {
				print_exp((*t)->car);
				if (!null((*t)->cdr)) {
					printf(" ");
					if ((*t)->cdr->type == LIST) {
						t = &(*t)->cdr;						
					}
					else{
						printf(". ");
						print_exp((*t)->cdr);
						break;
					}
				} else
					break;
			}				
			printf(")");
	}
}

/*==============================================================================
LISP evaluator 
==============================================================================*/

struct object* evlis(struct object* exp, struct object* env) {
	if ( null(exp))
		return NIL;
	return cons(eval(car(exp), env), evlis(cdr(exp), env));
}

struct object* eval_sequence(struct object* exps, struct object* env) {
	if (null(cdr(exps))) 
		return eval(car(exps), env);
	eval(car(exps), env);
	return eval_sequence(cdr(exps), env);
}

struct object* eval(struct object* exp, struct object* env) {

tail:
	if (null(exp) || exp == EMPTY_LIST) {
		return NIL;
	} else if (exp->type == INTEGER || exp->type == STRING) {
		return exp;
	} else if (exp->type == SYMBOL){
		return lookup_variable(exp, env);
	} else if (is_tagged(exp, QUOTE)) {
		return cadr(exp);
	} else if (is_tagged(exp, LAMBDA)) { 
		return make_procedure(cadr(exp), cddr(exp), env);
	} else if (is_tagged(exp, DEFINE)) {
		if (atom(cadr(exp))) 
			define_variable(cadr(exp), eval(caddr(exp), env), env);
		else {
			struct object* closure = eval(make_lambda(cdr(cadr(exp)), cddr(exp)), env);
			define_variable(car(cadr(exp)), closure, env);
		}
		return make_symbol("ok");
	} else if (is_tagged(exp, BEGIN)) {
 		struct object* args = cdr(exp);
 		for (; !null(cdr(args)); args = cdr(args))
 			eval(car(args), env);
 		exp = car(args);
 		goto tail;
  	} else if (is_tagged(exp, IF))  {
  		struct object* predicate = eval(cadr(exp), env);
 		struct object* consequent = caddr(exp);
 		struct object* alternative = cadddr(exp);
 		if (!is_equal(predicate, NIL)) 
 			exp = consequent;
 		else 
 			exp = alternative;
 		goto tail;
 	} else if (is_tagged(exp, SET)) {
		if (atom(cadr(exp))) 
			set_variable(cadr(exp), eval(caddr(exp), env), env);
		else {
			struct object* closure = eval(make_lambda(cdr(cadr(exp)), cddr(exp)), env);
			set_variable(car(cadr(exp)), closure, env);
		}
		return make_symbol("ok");
	} else if (is_tagged(exp, make_symbol("gc"))) {
		gc_traverse();
		return NIL;

	} else if (is_tagged(exp, LET)) {
		/* We go with the strategy of transforming let into a lambda function*/
		struct object** tmp;
		struct object* vars = NIL;
		struct object* vals = NIL;
		if (null(cadr(exp)))
			return NIL;
		/* NAMED LET */
		if (atom(cadr(exp))) {
			for (tmp =&exp->cdr->cdr->car; !null(*tmp); tmp=&(*tmp)->cdr) {
				vars = cons(caar(*tmp), vars);
				vals = cons(cadar(*tmp), vals);
			}
			/* Define the named let as a lambda function */
			define_variable(cadr(exp), eval(make_lambda(vars, cdr(cddr(exp))), extend_env(vars, vals, env)), env);
			/* Then evaluate the lambda function with the starting values */

			exp = cons(cadr(exp), vals);
			goto tail;
		}
		for (tmp =&exp->cdr->car; !null(*tmp); tmp=&(*tmp)->cdr) {
			vars = cons(caar(*tmp), vars);
			vals = cons(cadar(*tmp), vals);
		}
		exp = cons(make_lambda(vars, cddr(exp)), vals);
		goto tail;
	} else {
		/* procedure structure is as follows:
		('procedure, (parameters), (body), (env)) */
		struct object* proc = eval(car(exp), env);
		struct object* args = evlis(cdr(exp), env);
		if (null(proc)) {
			return NIL;
		}
		if (proc->type == PRIMITIVE) 
			return proc->primitive(args);
		if (is_tagged(proc, PROCEDURE)) {
			env = extend_env(cadr(proc), args, cadddr(proc));
			exp = cons(BEGIN, caddr(proc));	/* procedure body */
			goto tail;
		}		
		error("Invalid arguments to eval()");
	}
	return NIL;
}

void init_env() {
	#define add_prim(s, c) \
		define_variable(make_symbol(s), make_primitive(c), ENV)

	ENV = extend_env(NIL, NIL, NIL);
	//EMPTY_LIST  = cons(NIL, NIL);
	TRUE 		= make_symbol("#t");
	QUOTE 		= make_symbol("quote");
	LAMBDA 		= make_symbol("lambda");
	PROCEDURE 	= make_symbol("procedure");
	DEFINE 		= make_symbol("define");
	SET 		= make_symbol("set!");
	LET 		= make_symbol("let");
	BEGIN 		= make_symbol("begin");
	IF 			= make_symbol("if");
	define_variable(TRUE, TRUE, ENV);
	define_variable(QUOTE, QUOTE, ENV);
	define_variable(LAMBDA, LAMBDA, ENV);
	define_variable(SET, SET, ENV);
	define_variable(make_symbol("true"), TRUE, ENV);
	define_variable(make_symbol("false"), NIL, ENV);
	add_prim("cons", prim_cons);
	add_prim("car", prim_car);
	add_prim("set-car!", prim_setcar);
	add_prim("set-cdr!", prim_setcdr);
	add_prim("cdr", prim_cdr);
	add_prim("list", prim_list);
	add_prim("list?", prim_listq);
	add_prim("null?", prim_nullq);
	add_prim("pair?", prim_pairq);
	add_prim("+", prim_add);
	add_prim("-", prim_sub);
	add_prim("*", prim_mul);
	add_prim("/", prim_div);
	add_prim("eq", prim_eq);
	add_prim("=", prim_eq);
	add_prim("<", prim_lt);
	add_prim(">", prim_gt);
	add_prim("load", load_file);
	add_prim("get-global-environment", prim_get_env);
	add_prim("set-global-environment", prim_set_env);
}

struct object* load_file(struct object* args) {
	struct object* exp;
	struct object* ret;
	char* filename = car(args)->string;
	printf("Evaluating file %s\n", filename);
	FILE* fp = fopen(filename, "r");
	for(;;) {
		exp = read_exp(fp);
		if (null(exp))
			break;
		ret = eval(exp, ENV);
	}
	fclose(fp);
	return ret;
}

int main(int argc, char** argv) {
	
	int NELEM = 8191;

	ht_init(NELEM);
	alloc_heap(sizeof(struct object) * NELEM);
	printf("HEAP SIZE: %lu\n", FREE_SPACE);
	init_env();
	struct object* exp;
	int i;
	for (i = 1; i < argc; i++) 
		load_file(cons(make_symbol(argv[i]), NIL));

	//gc_traverse();
	for(;;) {
		printf("user> ");
		exp = read_exp(stdin);
		printf("====> ");
		print_exp(eval(exp, ENV));
		printf("\n");
	}
}