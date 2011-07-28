/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/


#include <stdlib.h>
#include <string.h>

#include "dictionary.h"

struct Value {
  VALUE_TYPE type ;
  union {
    void *pointer ;
    const char *string ;       // So we don't have to use 'pointer' with a cast
    long integer ;
    double real ;
    } ;
  int pointerkind ;
  Free_Function *delete ;      // How to free a pointer
  } ;

typedef struct DictElement DictElement ;

struct DictElement {
  const char *key ;
  Value value ;
  DictElement *next ;
  } ;

struct Dictionary {
  int count ;
  DictElement *elements ;
  int usecount ;
  } ;



const char *string_copy(const char *s)
/*==================================*/
{
  if (s) {
    char *t = malloc(strlen(s)+1) ;
    strcpy(t, s) ;
    return t ;
    }
  else return NULL ;
  }


static DictElement *dict_element(dict *d, const char *key)
/*======================================================*/
{
  DictElement *e = d->elements ;
  while (e && strcmp(e->key, key)) e = e->next ;
  if (e == NULL) {
    e = (DictElement *)calloc(sizeof(DictElement), 1) ;
    e->key = string_copy(key) ;
    e->next = d->elements ;       // Link at start of list
    d->elements = e ;
    d->count += 1 ;
    }
  return e ;
  }

static void value_delete(Value *v)
/*==============================*/
{
  if (v->type == TYPE_STRING) free((void *)v->string) ;
  else if (v->delete) v->delete(v->pointer) ;
  }

static DictElement *dict_element_set(dict *d, const char *key, VALUE_TYPE t)
/*========================================================================*/
{
  DictElement *e = dict_element(d, key) ;
  value_delete(&e->value) ;
  e->value.type = t ;
  return e ;
  }

static void dictElement_free(DictElement *e)
/*========================================*/
{
  if (e) {
    if (e->key) free((void *)e->key) ;
    value_delete(&e->value) ;
    free(e) ;
    }
  }


dict *dict_create(void)
/*===================*/
{
  dict *d = (dict *)calloc(sizeof(dict), 1) ;
  d->usecount = 1 ;
  }

dict *dict_copy(dict *d)
/*====================*/
{
  ++d->usecount ;
  return d ;
  }

void dict_free(dict *d)
/*===================*/
{
  if (--d->usecount < 1) {
    DictElement *e = d->elements ;
    while (e) {
      DictElement *this = e ;
      e = e->next ;
      dictElement_free(this) ;
      }
    free(d) ;
    }
  }


void dict_set_pointer(dict *d, const char *key, void *p, int kind, Free_Function *delete)
/*=====================================================================================*/
{
  DictElement *e = dict_element_set(d, key, TYPE_POINTER) ;
  e->value.pointer = p ;
  e->value.pointerkind = kind ;
  e->value.delete = delete ;
  }

void dict_set_string(dict *d, const char *key, const char *s)
/*=========================================================*/
{
  DictElement *e = dict_element_set(d, key, TYPE_STRING) ;
  e->value.string = s ;
  }

void dict_copy_string(dict *d, const char *key, const char *s)
/*==========================================================*/
{
  DictElement *e = dict_element_set(d, key, TYPE_STRING) ;
  e->value.string = string_copy(s) ;
  }

void dict_set_integer(dict *d, const char *key, long i)
/*================================================*/
{
  DictElement *e = dict_element_set(d, key, TYPE_INTEGER) ;
  e->value.integer = (long)i ;
  }

void dict_set_real(dict *d, const char *key, double f)
/*==================================================*/
{
  DictElement *e = dict_element_set(d, key, TYPE_REAL) ;
  e->value.real = (double)f ;
  }


Value *dict_get_value(dict *d, const char *key, VALUE_TYPE *type)
/*=============================================================*/
{
  DictElement *e = d->elements ;
  while (e && strcmp(e->key, key)) e = e->next ;
  if (e) {
    if (type) *type = e->value.type ;
    return &e->value ;
    }
  return NULL ;
  }



void *dict_get_pointer(dict *d, const char *key, int *kind)
/*=======================================================*/
{
  VALUE_TYPE vt ;
  Value *v = dict_get_value(d, key, &vt) ;
  if (v && v->type == TYPE_POINTER) return value_get_pointer(v, kind) ;
  return NULL ;
  }

const char *dict_get_string(dict *d, const char *key)
/*=================================================*/
{
  VALUE_TYPE vt ;
  Value *v = dict_get_value(d, key, &vt) ;
  if (v && v->type == TYPE_STRING) return value_get_string(v) ;
  return NULL ;
  }

long dict_get_integer(dict *d, const char *key)
/*===========================================*/
{
  VALUE_TYPE vt ;
  Value *v = dict_get_value(d, key, &vt) ;
  if (v && v->type == TYPE_INTEGER) return value_get_integer(v) ;
  return 0 ;
  }

double dict_get_real(dict *d, const char *key)
/*==========================================*/
{
  VALUE_TYPE vt ;
  Value *v = dict_get_value(d, key, &vt) ;
  if (v && v->type == TYPE_REAL) return value_get_real(v) ;
  return 0.0 ;
  }



VALUE_TYPE value_type(Value *v)
/*===========================*/
{
  return v->type ;
  }

void *value_get_pointer(Value *v, int *kind)
/*========================================*/
{
  if (kind) *kind = v->pointerkind ;
  return v->pointer ;
  }

const char *value_get_string(Value *v)
/*==================================*/
{
  return v->string ;
  }

long value_get_integer(Value *v)
/*============================*/
{
  return (long)v->integer ;
  }

double value_get_real(Value *v)
/*===========================*/
{
  return (double)v->real ;
  }


void dict_delete(dict *d, const char *key)
/*======================================*/
{
  DictElement *prev = NULL ;
  DictElement *e = d->elements ;
  while (e && strcmp(e->key, key)) {
    prev = e ;
    e = e->next ;
    }
  if (e) {
    if (prev) prev->next = e->next ;
    else      d->elements = e->next ;
    dictElement_free(e) ;
    d->count -= 1 ;
    }
  }

void dict_iterate(dict *d, Iterator_Function *f, void *param)
/*=========================================================*/
{
  DictElement *e = d->elements ;
  int done = 0 ;
  while (e && !done) {
    done = f(e->key, &e->value, param) ;
    e = e->next ;
    }
  }


#include <stdio.h>

int print(const char *k, Value *v, void *p)
/*=======================================*/
{
  switch (value_type(v)) {
   case TYPE_STRING:
    printf("'%s': '%s'\n", k, value_get_string(v)) ;
    break ;
   case TYPE_INTEGER:
    printf("'%s': %d\n", k, value_get_integer(v)) ;
    break ;
   case TYPE_REAL:
    printf("'%s': %g\n", k, value_get_real(v)) ;
    break ;
   default:
    break ;
    }
  return 0 ;
  }


#ifdef DICTTEST


int main(void)
/*===========*/
{

  dict *d = dict_create() ;


  dict_copy_string(d,  "1", "a") ;
  dict_set_integer(d, "2", 2) ;
  dict_set_real(d,   "3", 3.1) ;

  dict_iterate(d, print, NULL) ;
  

  if (dict_get_value(d, "XX", NULL)) printf("ERROR...!!\n") ;
  else printf("No element found, as expected\n") ;


  dict_free(d) ;
  }

#endif
