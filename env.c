#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdarg.h>
#include "lisp.h"

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

void env_init() {
	global_env = env_new();
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
	for (tmp = &global_env->alist; *tmp; tmp=&(*tmp)->cdr) 
		;
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