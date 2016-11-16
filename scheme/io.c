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

sexp_t *NIL;
sexp_t *EMPTY_LIST;

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
	if (null(exp)) {
		return;
	}
	switch(exp->type) {
		case SYMBOL: case STRING:
			printf("%s", exp->string);
			break;
		case INTEGER:
			printf("%d", exp->number);
			break;
		case LIST:
			printf("(");
			print_exp(exp->car);
			if (!null(exp->cdr)) {
				printf(" . ");
				print_exp(exp->cdr);
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
void push(sexp_t* list, sexp_t* obj) {
	sexp_t** t;
	if (list->car == NIL || list->car == EMPTY_LIST) {
		list->car = obj;
		return;
	}
	for (t = &list; !null(*t); t = &(*t)->cdr)
		;
	(*t) = new(LIST, obj, NIL); //new(LIST, NIL, NIL));
}

void eat(FILE* in) {
	int c;
	while ((c = getc(in))!= EOF) {
		if (c == ' ')
			continue;
		else if (c == ';') {
			while (((c = getc(in))!= EOF) && (c != '\n'))
				continue;
		}
		ungetc(c, in);
		break;
	} 
}
sexp_t* convert(char* string) {
	return new(STRING, string);
}


sexp_t* read(FILE* in) {
	char buffer[100];
	char c = ' ';
	int index = 0;

	while(c != EOF) {
		c = getc(in);
		if (c == ' ' || c == '\n')
			break;
		if (c == '(' || c == ')') {
			ungetc(c, in);
			break;
		}
		buffer[index++] = c;
	}
	buffer[index] = '\0';
	return tokenize(buffer);
}


sexp_t* repl(FILE* in) {
	char c = ' ';
	int i;
	int index = 0;
	int depth = 0;

	sexp_t* ret = new(LIST, NIL, NIL);

	while(c != EOF) {
		eat(in);
		c = getc(in);
		switch (c) {
			case '(':
				depth++;
				if 
				ret = cons()
				break;
			case ')':
				depth--;
				if (depth == 0)
					return ret;
				break;
			default:
				ungetc(c, in);
				push(ret, read(in));
				
				//print_exp(read(in));
		}

	}
	return ret;
}

int main(int argc, char** argv) {
	printf("REPL\n");
	//print_exp(cons(cons(new(INTEGER, 9), new(INTEGER, 10)), NIL));
	while(1) {
		//print_exp(read(stdin));
		print_exp(repl(stdin));
		printf("\n");
	}
}