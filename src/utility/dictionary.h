/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#ifndef _DICTIONARY_H
#define _DICTIONARY_H

#include "values.h"

typedef struct Dictionary dict ;

typedef void (Dict_Iterator)(const char *, Value *, void *) ;

#ifdef __cplusplus
extern "C" {
#endif

dict *dict_create(void) ;
dict *dict_copy(dict *) ;
void dict_free(dict *) ;

void dict_set_pointer(dict *, const char *, void *, int, Value_Free *) ;
void dict_set_string(dict *, const char *, const char *) ;
void dict_set_copied_string(dict *, const char *, const char *) ;
void dict_set_integer(dict *, const char *, long) ;
void dict_set_real(dict *, const char *, double) ;

Value      *dict_get_value(dict *, const char *, VALUE_TYPE *) ;
void       *dict_get_pointer(dict *, const char *, int *) ;
const char *dict_get_string(dict *, const char *) ;
long        dict_get_integer(dict *, const char *) ;
double      dict_get_real(dict *, const char *) ;

void dict_delete(dict *, const char *) ;
void dict_iterate(dict *, Dict_Iterator *, void *) ;

void dict_print(dict *) ;

#ifdef __cplusplus
} ;
#endif

#endif
