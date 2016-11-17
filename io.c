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

sexp_t* read(FILE* in, int depth) {
	char* buffer = malloc(256);
	char c = ' ';
	int index = 0;
	int i;
	int literal = 1;

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
		}
		else if (c == '\n') {
			append();
			for (i = 0; i < depth; i++)
				printf("..");
			// if (depth == 0)
			// 	break;
		} else {	
			if (c == '\'') {
				exp->car = new(LIST, new(SYMBOL, "quote"), read(in, depth));
				exp->cdr = new(LIST, NIL, NULL);
				exp = exp->cdr;
			}		
			else if (c == '(') {
				literal = 0;
				append();
				depth++;
				if (depth > 1) {
					exp->car = read(in, depth);
					exp->cdr = malloc(sizeof(sexp_t));
					exp = exp->cdr;
					exp->type = LIST;
					index = 0;
				}
			}
			else if (c == ')') {
				depth--;
				break;
			}	
			else
				buffer[index++] = c;
		}		
	}
	append();
	if (null(ret->car))
		return NIL;

	return ret;
}

int main(int argc, char** argv) {
	printf("REPL\n");
	print_exp(cons(cons(new(INTEGER, 9), new(INTEGER, 10)), new(INTEGER, 82)));
	sexp_t* in;
	while(1) {
		
		printf("user> ");
		in = read(stdin, 0);
	
		printf("=> ");
		print_exp(in);
		printf("\n");
		
	}
}