#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "lisp.h"


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
			if (eq(car(obj), new_sym("procedure"))) {
				printf("<procedure> ");
				//print_object(cadr(obj));
				//print_object(caddr(obj));
				return;
			}
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
			break;
		} 
		case PRIM:
			//printf("#<primitive>");
			break;
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
				return TRUE;
			if (*token == 'f')
				return FALSE;
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
	int paren_quote = 0;
	char word[128];
	memset(word, 0, 128);
	struct obj_list* ol = new_ol();

	/* second pass, split strings by the delimiter */
	for (i = 0; i < strlen(str); i++) {
		switch(str[i]) {
			case ' ':
				if (str[i + 1] == ' ')
					continue;

				word[idx] = '\0';
				if (idx) 
					push(ol->val, tok_to_obj(word));
				idx = 0;

				if (quoted && !paren_quote) {
					ol_pop(&ol);
					quoted = 0;
					paren_quote = 0;
				}

				continue;
			case '\'':
				quoted = 1;
	
				ol_push(&ol); 
				push(ol->val, new_sym("quote"));
				push(ol->prev->val, ol->val);
				
				word[idx] = '\0';
				if (idx) 
					push(ol->val, tok_to_obj(word));
				idx = 0;	
			
				continue;
			case '\n':
				continue;
			case '\0':
				word[idx] = '\0';
				if (idx) 
					push(ol->val, tok_to_obj(word));
				idx = 0;
				break;
			case '(':
				literal = 1;
				paren_quote = (quoted);
				d++;
				word[idx] = '\0';
				if (idx) 
					push(ol->val, tok_to_obj(word));
				
				if (d>1) {
					ol_push(&ol);
					push(ol->prev->val, ol->val);
				}

				idx = 0;
				continue;
			case ')':
				d--;
				word[idx] = '\0';
				if (idx)
					push(ol->val, tok_to_obj(word));
				if(d>0)
					ol_pop(&ol);

				if (quoted) {
					quoted = 0;
					ol_pop(&ol);
					paren_quote = 0;
				}

				idx=0;	
				continue;
			default:
				word[idx++] = str[i];
				continue;


		}
	}

	word[idx] = '\0';

	if (idx) 
		push(ol->val, tok_to_obj(word));
	if(!literal) {
		if (quoted) 
			return ol->val;
		return ol->val->car;
	}
		if (quoted) {
					quoted = 0;
					ol_pop(&ol);
					paren_quote = 0;
				}

	return ol->val; 
}

char* read(FILE* in) {
	char* buffer = malloc(256);
	char c;
	int depth = 0;
	int index = 0;
	int i = 0;
	while((c = getc(in)) != EOF) {
		//c = getc(in);
		if (c == '\n') {
			for (i = 0; i < depth; i++)
				printf("..");
			if (depth == 0)
				break;
			c =' ';
		}
		buffer[index++] = c;
		if (c == '(')
			depth++;
		if (c == ')') {
			depth--;
			if (depth == 0) 
			break;
		}
		
	}
	buffer[index] = '\0';
	return buffer; //tokenize(buffer);
}
