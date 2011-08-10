/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _LIST_H
#define _LIST_H

#include "values.h"

typedef struct List list ;

typedef void (List_Iterator)(int, Value *, void *) ;
typedef int  (List_Iterator_Break)(int, Value *, void *) ;

#ifdef __cplusplus
extern "C" {
#endif

list *list_create(void) ;
void  list_free(list *) ;
int   list_length(list *) ;

int list_append_pointer(list *, void *, int, Value_Free *) ;
int list_append_string(list *, const char *) ;
int list_append_copied_string(list *, const char *) ;
int list_append_integer(list *, long) ;
int list_append_real(list *, double) ;

Value *list_get_value(list *, int, VALUE_TYPE *) ;
Value *list_pop_value(list *, int, VALUE_TYPE *) ;
void  *list_get_pointer(list *, int, int *) ;

void list_iterate(list *, List_Iterator *, void *) ;
void list_iterate_break(list *, List_Iterator_Break *, void *) ;

void list_print(list *) ;

#ifdef __cplusplus
} ;
#endif

#endif

