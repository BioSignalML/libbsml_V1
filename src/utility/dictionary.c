/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BSML_INTERNALS  1
#include "dictionary.h"


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

static DictElement *dict_element_set(dict *d, const char *key, VALUE_TYPE t)
/*========================================================================*/
{
  DictElement *e = dict_element(d, key) ;
  value_free(&e->value) ;
  e->value.type = t ;
  return e ;
  }

static void dictElement_free(DictElement *e)
/*========================================*/
{
  if (e) {
    if (e->key) free((void *)e->key) ;
    value_free(&e->value) ;
    free(e) ;
    }
  }


dict *dict_create(void)
/*===================*/
{
  dict *d = (dict *)calloc(sizeof(dict), 1) ;
  d->usecount = 1 ;
  return d ;
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


void dict_set_pointer(dict *d, const char *key, void *p, int kind, Value_Free *delete)
/*==================================================================================*/
{
  DictElement *e = dict_element_set(d, key, VALUE_TYPE_POINTER) ;
  e->value.pointer = p ;
  e->value.pointerkind = kind ;
  e->value.delete = delete ;
  }

void dict_set_string(dict *d, const char *key, const char *s)
/*=========================================================*/
{
  DictElement *e = dict_element_set(d, key, VALUE_TYPE_STRING) ;
  e->value.string = string_copy(s) ;
  }

void dict_set_copied_string(dict *d, const char *key, const char *s)
/*================================================================*/
{
  DictElement *e = dict_element_set(d, key, VALUE_TYPE_STRING) ;
  e->value.string = s ;
  }

void dict_set_integer(dict *d, const char *key, long i)
/*================================================*/
{
  DictElement *e = dict_element_set(d, key, VALUE_TYPE_INTEGER) ;
  e->value.integer = (long)i ;
  }

void dict_set_real(dict *d, const char *key, double f)
/*==================================================*/
{
  DictElement *e = dict_element_set(d, key, VALUE_TYPE_REAL) ;
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
  return value_get_pointer(dict_get_value(d, key, NULL), kind) ;
  }

const char *dict_get_string(dict *d, const char *key)
/*=================================================*/
{
  return value_get_string(dict_get_value(d, key, NULL)) ;
  }

long dict_get_integer(dict *d, const char *key)
/*===========================================*/
{
  return value_get_integer(dict_get_value(d, key, NULL)) ;
  }

double dict_get_real(dict *d, const char *key)
/*==========================================*/
{
  return value_get_real(dict_get_value(d, key, NULL)) ;
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

void dict_iterate(dict *d, Dict_Iterator *f, void *param)
/*=====================================================*/
{
  DictElement *e = d->elements ;
  while (e) {
    f(e->key, &e->value, param) ;
    e = e->next ;
    }
  }


void dict_iterate_break(dict *d, Dict_Iterator_Break *f, void *param)
/*=================================================================*/
{
  DictElement *e = d->elements ;
  int done = 0 ;
  while (e && !done) {
    done = f(e->key, &e->value, param) ;
    e = e->next ;
    }
  }


static void print_entry(const char *k, Value *v, void *p)
/*=====================================================*/
{
  void *ptr ;
  int kind ;
  switch (value_type(v)) {
   case VALUE_TYPE_POINTER:
    ptr = value_get_pointer(v, &kind) ;
    printf("'%s': 0x%08lx (%d)\n", k, ptr, kind) ;
    break ;
   case VALUE_TYPE_STRING:
    printf("'%s': '%s'\n", k, value_get_string(v)) ;
    break ;
   case VALUE_TYPE_INTEGER:
    printf("'%s': %d\n", k, value_get_integer(v)) ;
    break ;
   case VALUE_TYPE_REAL:
    printf("'%s': %g\n", k, value_get_real(v)) ;
    break ;
   default:
    break ;
    }
  }

void dict_print(dict *d)
/*====================*/
{
  dict_iterate(d, print_entry, NULL) ;
  }


#ifdef DICTTEST


int main(void)
/*===========*/
{

  dict *d = dict_create() ;


  dict_copy_string(d,  "1", "a") ;
  dict_set_integer(d, "2", 2) ;
  dict_set_real(d,   "3", 3.1) ;
  dict_set_pointer(d, "4", (void *)main, 4, NULL) ;

  dict_print(d) ;
  

  if (dict_get_value(d, "XX", NULL)) printf("ERROR...!!\n") ;
  else printf("No element found, as expected\n") ;


  dict_free(d) ;
  }

#endif
