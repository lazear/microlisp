#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#define null(x) ((x) == NULL || (x) == NIL)
#define atom(x) (!null(x) && (x)->type != LIST)
#define cons(x, y) (new(LIST, x, y))

enum token_type { SYMBOL, INTEGER, STRING, LIST };
typedef struct sexp sexp_t;
sexp_t *NIL;
sexp_t *EMPTY_LIST;

struct sexp {
	enum token_type type;
	union {
		int number;
		char* string;
		struct {
			struct sexp* car;
			struct sexp* cdr;
		};
	};
};

sexp_t* new(enum token_type type, ...) {
	va_list ap;
	va_start(ap, type);
	sexp_t* object = malloc(sizeof(sexp_t));
	object->type = type;
	switch(type) {
		case SYMBOL: case STRING:
			object->string = strdup(va_arg(ap, char*));
			break;
		case INTEGER:
			object->number = va_arg(ap, int);
			break;
		case LIST:
			object->car = va_arg(ap, sexp_t*);
			object->cdr = va_arg(ap, sexp_t*);
			break;
	}
	return object;
}

void print_exp(sexp_t* exp) {
	if (null(exp))
		return;
	switch(exp->type) {
		case SYMBOL: case STRING:
			printf("%s", exp->string);
			break;
		case INTEGER:
			printf("%d", exp->number);
			break;
		case LIST:
			printf("(");
			sexp_t** t = &exp;
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

sexp_t* tokenize(char* token) {
	int i;
	enum token_type type;

	type = INTEGER;
	for (i = 0; i < strlen(token); i++) {
		if (isalpha(token[i]))
			type = SYMBOL;
		if (token[i] == '\"')
			type = STRING;
	}
	switch (type) {
		case SYMBOL: 
			return new(SYMBOL, token);
		case STRING:
			return new(STRING, token);
		case INTEGER:
			return new(INTEGER, atoi(token));
	}
	return NIL;
}

/* global variables for read() */
int PDEPTH = 0;

/* function read(in) reads one full closure from the input file stream, and 
returns a S-expression */
sexp_t* read(FILE* in) {
	char* buffer = malloc(256);
	char c = ' ';
	int index = 0;
	int literal = 1;
	static int stop = 1;
	int i;

/* macro for tokenizing buffer and resetting the input */
#define append() \
	if (index) {\
		buffer[index] = '\0';\
		exp->car = tokenize(buffer);\
		exp->cdr = malloc(sizeof(sexp_t));\
		exp = exp->cdr;\
		exp->type = LIST;\
		index = 0;\
	}
	sexp_t* exp = new(LIST, NULL, NULL);
	sexp_t* ret = exp;
	while((c = getc(in)) != EOF){
		
		if (c == ' ') {
			append();
			if (stop == 1)
				break;
			continue;
		}
		if (c == '\n') {
			append();
			for (i = 0; i < PDEPTH; i++)
				printf("..");
			if (stop)
				break;
			continue;
		} 
		if (c == '\'') {
			stop = (literal) ? 1 : 2;
			sexp_t* e = read(in);
			exp->car = cons(new(SYMBOL, "quote"), cons(e, NIL));
			exp->cdr = malloc(sizeof(sexp_t));
			exp = exp->cdr;
			exp->type = LIST;
			break;
		}		
		else if (c == '(') {	
			append();
			literal = 0;
			stop = 0;
			PDEPTH++;
			if (PDEPTH > 1) {
				exp->car = read(in);
				exp->cdr = malloc(sizeof(sexp_t));
				exp = exp->cdr;
				exp->type = LIST;
				index = 0;
			}
			if (PDEPTH == 0)
				break;
		}
		else if (c == ')') {
			PDEPTH--;
			if (PDEPTH == 0)
				break;
			if (stop)
				break;
		}
		else
			buffer[index++] = c;			
	}
	append();
	free(buffer);
	//stop = 0;
	if (null(ret->car))
		return NIL;
	if (literal) 
		return ret->car;

	return ret;
}

int main(int argc, char** argv) {
	printf("REPL\n");
	print_exp(cons(cons(new(INTEGER, 9), new(INTEGER, 10)), new(INTEGER, 82)));
	sexp_t* in;
	while(1) {
		
		printf("user> ");
		in = read(stdin);
	
		printf("=> ");
		print_exp(in);
		printf("\n");
		
	}
}
