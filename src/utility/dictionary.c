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


char *string_copy(const char *s)
/*============================*/
{
  if (s) {
    char *t = malloc(strlen(s)+1) ;
    strcpy(t, s) ;
    return t ;
    }
  else return NULL ;
  }

static DictElement *dict_add_element(Dictionary *d, const char *key)
/*================================================================*/
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

static void dictElement_free(DictElement *e)
/*========================================*/
{
  if (e) {
    if (e->key) free(e->key) ;
    if (e->value.type == TYPE_STRING) free(e->value.string) ;
    free(e) ;
    }
  }


Value *value_create(TYPE_VALUES type)
/*=================================*/
{
  Value *v = (Value *)calloc(sizeof(Value), 1) ;
  v->type = type ;
  return v ;
  }

Value *value_assign(Value *u, Value *v)
/*===================================*/
{
  if (u == NULL) u = (Value *)calloc(sizeof(Value), 1) ;
  else if (u->type == TYPE_STRING) free(u->string) ;
  memcpy(u, v, sizeof(Value)) ;
  if (v->type == TYPE_STRING) u->string = string_copy(v->string) ;
  return u ;
  }

Value *value_assign_string(Value *v, const char *s)
/*===============================================*/
{
  if (v == NULL) v = value_create(TYPE_STRING) ;
  else if (v->type == TYPE_STRING) free(v->string) ;
  v->type = TYPE_STRING ;
  v->string = string_copy(s) ;
  return v ;
  }

Value *value_assign_short(Value *v, short i)
/*========================================*/
{
  if (v == NULL) v = value_create(TYPE_SHORT) ;
  else if (v->type == TYPE_STRING) free(v->string) ;
  v->type = TYPE_SHORT ;
  v->integer = (long)i ;
  return v ;
  }

Value *value_assign_integer(Value *v, int i)
/*========================================*/
{
  if (v == NULL) v = value_create(TYPE_INTEGER) ;
  else if (v->type == TYPE_STRING) free(v->string) ;
  v->type = TYPE_INTEGER ;
  v->integer = (long)i ;
  return v ;
  }

Value *value_assign_long(Value *v, long i)
/*======================================*/
{
  if (v == NULL) v = value_create(TYPE_LONG) ;
  else if (v->type == TYPE_STRING) free(v->string) ;
  v->type = TYPE_LONG ;
  v->integer = (long)i ;
  return v ;
  }

Value *value_assign_float(Value *v, float f)
/*========================================*/
{
  if (v == NULL) v = value_create(TYPE_FLOAT) ;
  else if (v->type == TYPE_STRING) free(v->string) ;
  v->type = TYPE_FLOAT ;
  v->real = (double)f ;
  return v ;
  }

Value *value_assign_double(Value *v, double f)
/*==========================================*/
{
  if (v == NULL) v = value_create(TYPE_FLOAT) ;
  else if (v->type == TYPE_STRING) free(v->string) ;
  v->type = TYPE_DOUBLE ;
  v->real = (double)f ;
  return v ;
  }

void value_free(Value *v)
/*=====================*/
{
  if (v->type == TYPE_STRING) free(v->string) ;
  free(v) ;
  }


Dictionary *dict_create(void)
/*=========================*/
{
  Dictionary *d = (Dictionary *)calloc(sizeof(Dictionary), 1) ;
  d->usecount = 1 ;
  }

Dictionary *dict_copy(Dictionary *d)
/*================================*/
{
  ++d->usecount ;
  return d ;
  }

void dict_free(Dictionary *d)
/*=========================*/
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


void dict_set(Dictionary *d, const char *key, Value *value)
/*=======================================================*/
{
  DictElement *e = dict_add_element(d, key) ;
  value_assign(&e->value, value) ;
  }

void dict_set_string(Dictionary *d, const char *key, const char *s)
/*===============================================================*/
{
  DictElement *e = dict_add_element(d, key) ;
  value_assign_string(&e->value, s) ;
  }

void dict_set_short(Dictionary *d, const char *key, short i)
/*========================================================*/
{
  DictElement *e = dict_add_element(d, key) ;
  value_assign_short(&e->value, i) ;
  }

void dict_set_integer(Dictionary *d, const char *key, int i)
/*========================================================*/
{
  DictElement *e = dict_add_element(d, key) ;
  value_assign_integer(&e->value, i) ;
  }

void dict_set_long(Dictionary *d, const char *key, long i)
/*======================================================*/
{
  DictElement *e = dict_add_element(d, key) ;
  value_assign_long(&e->value, i) ;
  }

void dict_set_float(Dictionary *d, const char *key, float f)
/*========================================================*/
{
  DictElement *e = dict_add_element(d, key) ;
  value_assign_float(&e->value, f) ;
  }

void dict_set_double(Dictionary *d, const char *key, double f)
/*==========================================================*/
{
  DictElement *e = dict_add_element(d, key) ;
  value_assign_double(&e->value, f) ;
  }




Value *dict_get(Dictionary *d, const char *key)
/*===========================================*/
{
  DictElement *e = d->elements ;
  while (e && strcmp(e->key, key)) e = e->next ;
  return e ? &e->value : NULL ;
  }


void dict_delete(Dictionary *d, const char *key)
/*============================================*/
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


void dict_iterate(Dictionary *d, void(*f)(const char *, Value *))
/*=============================================================*/
{
  DictElement *e = d->elements ;
  while (e) {
    f(e->key, &e->value) ;
    e = e->next ;
    }
  }



#ifdef UNITTEST

#include <stdio.h>

void print(const char *k, Value *v)
/*===============================*/
{
  switch (v->type) {
   case TYPE_STRING:
    printf("'%s': '%s'\n", k, v->string) ;
    break ;

   case TYPE_INTEGER:
    printf("'%s': %d\n", k, (int)v->integer) ;
    break ;

   case TYPE_FLOAT:
    printf("'%s': %f\n", k, (float)v->real) ;
    break ;

   default:
    break ;
    }
  }


int main(void)
/*===========*/
{

  Dictionary *d = dict_create() ;


  dict_set_string(d,  "1", "a") ;
  dict_set_integer(d, "2", 2) ;
  dict_set_float(d,   "3", 3.0) ;

  dict_iterate(d, print) ;
  

  if (dict_get(d, "XX")) printf("ERROR...!!\n") ;
  else printf("No element found, as expected\n") ;


  dict_free(d) ;
  }

#endif
