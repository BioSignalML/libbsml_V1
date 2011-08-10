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
#include "list.h"


typedef struct ListElement ListElement ;

struct ListElement {
  Value value ;
  ListElement *prev ;
  ListElement *next ;
  } ;

struct List {
  int count ;
  ListElement *head ;
  ListElement *tail ;
  } ;



static ListElement *list_element_insert(list *l, int index)
//=========================================================
{
  if (index < 0 || index > l->count) return NULL ;

  ListElement *e = (ListElement *)calloc(sizeof(ListElement), 1) ;
  if (l->count == 0) {
    l->head = e ;
    l->tail = e ;
    }
  else if (index == 0) {
    e->next = l->head ;
    l->head->prev = e ;
    l->head = e ;
    }
  else if (index == l->count) {
    l->tail->next = e ;
    e->prev = l->tail ;
    l->tail = e ;
    }
  else {
    ListElement *t = l->head ;
    while (--index > 0) t = t->next ;
    e->prev = t ;
    e->next = t->next ;
    t->next->prev = e ;
    t->next = e ;
    }
  l->count += 1 ;
  return e ;
  }

static ListElement *list_element_get(list *l, int index)
//======================================================
{
  if (index < 0 || index > l->count) return NULL ;
  if (index == l->count) return l->tail  ;
  else {
    ListElement *t = l->head ;
    while (index-- > 0) t = t->next ;
    return t ;
    }
  }


static ListElement *list_element_set(list *l, int index, VALUE_TYPE t)
//====================================================================
{
  ListElement *e = list_element_insert(l, index) ;
  if (e) e->value.type = t ;
  return e ;
  }

static void ListElement_free(ListElement *e)
/*========================================*/
{
  if (e) {
    value_free(&e->value) ;
    free(e) ;
    }
  }


list *list_create(void)
/*===================*/
{
  return (list *)calloc(sizeof(list), 1) ;
  }

void list_free(list *l)
/*===================*/
{
  ListElement *e = l->head ;
  while (e) {
    ListElement *this = e ;
    e = e->next ;
    listElement_free(this) ;
    }
  free(l) ;
  }

int list_length(list *l)
//======================
{
  return l->count ;
  }


int list_append_pointer(list *l, void *p, int kind, Value_Free *delete)
//=====================================================================
{
  ListElement *e = list_element_set(l, l->count, TYPE_POINTER) ;
  if (e) {
    e->value.pointer = p ;
    e->value.pointerkind = kind ;
    e->value.delete = delete ;
    return l->count ;
    }
  return -1 ;
  }

int list_append_string(list *l, const char *s)
//============================================
{
  return list_append_copied_string(l, string_copy(s)) ;
  }

int list_append_copied_string(list *l, const char *s)
//===================================================
{
  ListElement *e = list_element_set(l, l->count, TYPE_STRING) ;
  if (e) {
    e->value.string = s ;
    return l->count ;
    }
  return -1 ;
  }

int list_append_integer(list *l, long i)
//======================================
{
  ListElement *e = list_element_set(l, l->count, TYPE_INTEGER) ;
  if (e) {
    e->value.integer = (long)i ;
    return l->count ;
    }
  return -1 ;
  }

int list_append_real(list *l, double f)
//=====================================
{
  ListElement *e = list_element_set(l, l->count, TYPE_REAL) ;
  if (e) {
    e->value.real = (double)f ;
    return l->count ;
    }
  return -1 ;
  }


Value *list_get_value(list *l, int index, VALUE_TYPE *type)
//=========================================================
{
  ListElement *e = list_element_get(l, index) ;
  if (e) {
    if (type) *type = e->value.type ;
    return &e->value ;
    }
  return NULL ;
  }

void *list_get_pointer(list *l, int index, int *kind)
//===================================================
{
  VALUE_TYPE vt ;
  Value *v = list_get_value(l, index, &vt) ;
  if (v && v->type == TYPE_POINTER) return value_get_pointer(v, kind) ;
  return NULL ;
  }


Value *list_pop_value(list *l, int index, VALUE_TYPE *type)
//=========================================================
{
  ListElement *e = list_element_get(l, index) ;
  if (e) {
    if (type) *type = e->value.type ;
    if (e->next) e->next->prev = e->prev ;
    if (e->prev) e->prev->next = e->next ;
    l->count -= 1 ;
    return &e->value ;
    }
  return NULL ;
  }


void list_iterate(list *l, List_Iterator *f, void *param)
/*=====================================================*/
{
  ListElement *e = l->head ;
  int n = 0 ;
  while (e) {
    f(n, &e->value, param) ;
    e = e->next ;
    ++n ;
    }
  }

void list_iterate_break(list *l, List_Iterator_Break *f, void *param)
/*=================================================================*/
{
  ListElement *e = l->head ;
  int n = 0 ;
  int done = 0 ;
  while (e && !done) {
    done = f(n, &e->value, param) ;
    e = e->next ;
    ++n ;
    }
  }

static void print_entry(int n, Value *v, void *p)
//===============================================
{
  void *ptr ;
  int kind ;
  switch (value_type(v)) {
   case TYPE_POINTER:
    ptr = value_get_pointer(v, &kind) ;
    printf("[%d]: 0x%08lx (%d)\n", n, ptr, kind) ;
    break ;
   case TYPE_STRING:
    printf("[%d]: '%s'\n", n, value_get_string(v)) ;
    break ;
   case TYPE_INTEGER:
    printf("[%d]: %d\n", n, value_get_integer(v)) ;
    break ;
   case TYPE_REAL:
    printf("[%d]: %g\n", n, value_get_real(v)) ;
    break ;
   default:
    break ;
    }
  }

void list_print(list *l)
/*====================*/
{
  list_iterate(l, print_entry, NULL) ;
  }


#ifdef LISTTEST


int main(void)
/*===========*/
{

  list *d = list_create() ;


  list_copy_string(d,  "1", "a") ;
  list_append_integer(d, "2", 2) ;
  list_append_real(d,   "3", 3.1) ;
  list_append_pointer(d, "4", (void *)main, 4, NULL) ;

  list_print(d) ;
  

  if (list_get_value(d, "XX", NULL)) printf("ERROR...!!\n") ;
  else printf("No element found, as expected\n") ;


  list_free(d) ;
  }

#endif

