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


typedef enum {
  TYPE_POINTER = 1,
  TYPE_STRING,
  TYPE_INTEGER,
  TYPE_REAL
  } VALUE_TYPE ;

typedef struct Value      Value ;
typedef struct Dictionary dict ;

typedef void (Iterator_Function)(const char *, Value *, void *) ;
typedef void (Free_Function)(void *) ;

#ifdef __cplusplus
extern "C" {
#endif

const char *string_copy(const char *) ;

dict *dict_create(void) ;
dict *dict_copy(dict *) ;
void dict_free(dict *) ;

void dict_set_pointer(dict *, const char *, void *, int, Free_Function *) ;
void dict_set_string(dict *, const char *, const char *) ;
void dict_copy_string(dict *, const char *, const char *) ;
void dict_set_integer(dict *, const char *, long) ;
void dict_set_real(dict *, const char *, double) ;

Value      *dict_get_value(dict *, const char *, VALUE_TYPE *) ;
void       *dict_get_pointer(dict *, const char *, int *) ;
const char *dict_get_string(dict *, const char *) ;
long   dict_get_integer(dict *, const char *) ;
double dict_get_real(dict *, const char *) ;

VALUE_TYPE  value_type(Value *) ;
void       *value_get_pointer(Value *, int *) ;
const char *value_get_string(Value *) ;
long   value_get_integer(Value *) ;
double value_get_real(Value *) ;

void dict_delete(dict *, const char *) ;
void dict_iterate(dict *, Iterator_Function *, void *) ;

void dict_print(dict *) ;

#ifdef __cplusplus
} ;
#endif

#endif
