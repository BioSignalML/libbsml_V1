/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-201  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsml_dictionary.h"
#include "bsml_string.h"


static bsml_dictionary_element *bsml_dictionary_element_get(bsml_dictionary *d, const char *key)
/*============================================================================================*/
{
  bsml_dictionary_element *e = d->elements ;
  while (e && strcmp(e->key, key)) e = e->next ;
  if (e == NULL) {                // Create a new entry if not found
    e = (bsml_dictionary_element *)calloc(sizeof(bsml_dictionary_element), 1) ;
    e->key = bsml_string_copy(key) ;
    e->next = d->elements ;       // Link into start of list
    d->elements = e ;
    d->length += 1 ;
    }
  return e ;
  }

bsml_dictionary_element *bsml_dictionary_element_find(bsml_dictionary *d, const char *key)
/*======================================================================================*/
{
  bsml_dictionary_element *e = d->elements ;
  while (e && strcmp(e->key, key)) e = e->next ;
  return e ;
  }

static void bsml_dictionary_element_free_pointers(bsml_dictionary_element *e)
/*=========================================================================*/
{
  if      (e->type == DICTIONARY_TYPE_STRING) bsml_string_free(e->svalue) ;
  else if (e->type == DICTIONARY_TYPE_POINTER && e->pfree) e->pfree(e->pointer) ;
  }

static void bsml_dictionary_element_free(bsml_dictionary_element *e)
/*================================================================*/
{
  if (e) {
    if (e->key) free((void *)e->key) ;
    bsml_dictionary_element_free_pointers(e) ;
    free(e) ;
    }
  }


bsml_dictionary *bsml_dictionary_create(void)
/*=========================================*/
{
  bsml_dictionary *d = (bsml_dictionary *)calloc(sizeof(bsml_dictionary), 1) ;
  d->usecount = 1 ;
  return d ;
  }

bsml_dictionary *bsml_dictionary_copy(bsml_dictionary *d)
/*=====================================================*/
{
  ++d->usecount ;
  return d ;
  }

void bsml_dictionary_free(bsml_dictionary *d)
/*=========================================*/
{
  if (--d->usecount < 1) {
    bsml_dictionary_element *e = d->elements ;
    while (e) {
      bsml_dictionary_element *this = e ;
      e = e->next ;
      bsml_dictionary_element_free(this) ;
      }
    free(d) ;
    }
  }


void bsml_dictionary_delete(bsml_dictionary *d, const char *key)
/*============================================================*/
{
  bsml_dictionary_element *prev = NULL ;
  bsml_dictionary_element *e = d->elements ;
  while (e && strcmp(e->key, key)) {
    prev = e ;
    e = e->next ;
    }
  if (e) {
    if (prev) prev->next = e->next ;
    else      d->elements = e->next ;
    bsml_dictionary_element_free(e) ;
    d->length -= 1 ;
    }
  }



void bsml_dictionary_iterate(bsml_dictionary *d, IterateDictionary *f, void *param)
/*===============================================================================*/
{
  bsml_dictionary_element *e = d->elements ;
  while (e) {
    f(e, param) ;
    e = e->next ;
    }
  }


void bsml_dictionary_iterate_break(bsml_dictionary *d, IterateDictionaryBreak *f, void *param)
/*==========================================================================================*/
{
  bsml_dictionary_element *e = d->elements ;
  int done = 0 ;
  while (e && !done) {
    done = f(e, param) ;
    e = e->next ;
    }
  }


void bsml_dictionary_set_long(bsml_dictionary *d, const char *key, long v)
/*======================================================================*/
{
  bsml_dictionary_element *e = bsml_dictionary_element_get(d, key) ;
  bsml_dictionary_element_free_pointers(e) ;
  e->type = DICTIONARY_TYPE_LONG ;
  e->lvalue = v ;
  }

void bsml_dictionary_set_double(bsml_dictionary *d, const char *key, double v)
/*==========================================================================*/
{
  bsml_dictionary_element *e = bsml_dictionary_element_get(d, key) ;
  bsml_dictionary_element_free_pointers(e) ;
  e->type = DICTIONARY_TYPE_DOUBLE ;
  e->dvalue = v ;
  }

void bsml_dictionary_set_string(bsml_dictionary *d, const char *key, char *v)
/*=========================================================================*/
{
  bsml_dictionary_element *e = bsml_dictionary_element_get(d, key) ;
  bsml_dictionary_element_free_pointers(e) ;
  e->type = DICTIONARY_TYPE_STRING ;
  e->svalue = bsml_string_copy(v) ;
  }

void bsml_dictionary_set_copied_string(bsml_dictionary *d, const char *key, char *v)
/*================================================================================*/
{
  bsml_dictionary_element *e = bsml_dictionary_element_get(d, key) ;
  bsml_dictionary_element_free_pointers(e) ;
  e->type = DICTIONARY_TYPE_STRING ;
  e->svalue = v ;
  }

void bsml_dictionary_set_pointer(bsml_dictionary *d, const char *key, void *v, FreePointer *pfree)
/*==============================================================================================*/
{
  bsml_dictionary_element *e = bsml_dictionary_element_get(d, key) ;
  bsml_dictionary_element_free_pointers(e) ;
  e->type = DICTIONARY_TYPE_POINTER ;
  e->pfree = pfree ;
  e->pointer = v ;
  }


int bsml_dictionary_get_long(bsml_dictionary *d, const char *key, long *v)
/*======================================================================*/
{
  bsml_dictionary_element *e = bsml_dictionary_element_find(d, key) ;
  if (e && e->type == DICTIONARY_TYPE_LONG) {
    *v = e->lvalue ;
    return 1 ;
    }
  return 0 ;
  }

int bsml_dictionary_get_double(bsml_dictionary *d, const char *key, long *v)
/*========================================================================*/
{
  bsml_dictionary_element *e = bsml_dictionary_element_find(d, key) ;
  if (e && e->type == DICTIONARY_TYPE_DOUBLE) {
    *v = e->dvalue ;
    return 1 ;
    }
  return 0 ;
  }

const char *bsml_dictionary_get_string(bsml_dictionary *d, const char *key)
/*=======================================================================*/
{
  bsml_dictionary_element *e = bsml_dictionary_element_find(d, key) ;
  if (e && e->type == DICTIONARY_TYPE_STRING) return e->svalue ;
  return NULL ;
  }


void *bsml_dictionary_get_pointer(bsml_dictionary *d, const char *key)
/*==================================================================*/
{
  bsml_dictionary_element *e = bsml_dictionary_element_find(d, key) ;
  if (e && e->type == DICTIONARY_TYPE_POINTER) return e->pointer ;
  return NULL ;
  }


#ifdef TEST_DICTIONARY

static void print_entry(bsml_dictionary_element *e, void *p)
/*========================================================*/
{
  const char *k = e->key ;
  switch (e->type) {
   case DICTIONARY_TYPE_POINTER:
    printf("'%s': 0x%08lx\n", k, (long)e->pointer) ;
    break ;
   case DICTIONARY_TYPE_STRING:
    printf("'%s': '%s'\n",    k, e->svalue) ;
    break ;
   case DICTIONARY_TYPE_LONG:
    printf("'%s': %ld\n", k, e->lvalue) ;
    break ;
   case DICTIONARY_TYPE_DOUBLE:
    printf("'%s': %g\n", k, e->dvalue) ;
    break ;
   default:
    break ;
    }
  }

void bsml_dictionary_print(bsml_dictionary *d)
/*==========================================*/
{
  bsml_dictionary_iterate(d, print_entry, NULL) ;
  }


int main(void)
/*===========*/
{

  bsml_dictionary *d = bsml_dictionary_create() ;


  bsml_dictionary_set_string(d,  "1", "a") ;
  bsml_dictionary_set_long(d,    "2", 2) ;
  bsml_dictionary_set_double(d,  "3", 3.1) ;
  bsml_dictionary_set_pointer(d, "4", (void *)calloc(100, 1), free) ;
  bsml_dictionary_set_pointer(d, "5", (void *)main, NULL) ;
  bsml_dictionary_print(d) ;

  bsml_dictionary_set_long(d,    "2", 12) ;
  bsml_dictionary_set_string(d,  "1", "bb") ;
  bsml_dictionary_delete(d,      "4") ;
  bsml_dictionary_print(d) ;
  
  if (bsml_dictionary_element_find(d, "XX")) printf("ERROR...!!\n") ;
  else printf("No element found, as expected\n") ;

  if (bsml_dictionary_element_find(d, "3")) printf("Found element as expected\n") ;
  else printf("ERROR...!!!\n") ;

  bsml_dictionary_free(d) ;
  }

#endif
