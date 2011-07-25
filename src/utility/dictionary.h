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
  TYPE_STRING = 1,
  TYPE_SHORT,
  TYPE_INTEGER,
  TYPE_LONG,
  TYPE_FLOAT,
  TYPE_DOUBLE
  } TYPE_VALUES ;

typedef struct {
  TYPE_VALUES type ;
  char *string ;
  long integer ;
  double real ;
  } Value ;


typedef struct DictElement {
  char *key ;
  Value value ;
  struct DictElement *next ;
  } DictElement ;


typedef struct {
  int count ;
  DictElement *elements ;
  int usecount ;
  } Dictionary ;


#ifdef __cplusplus
extern "C" {
#endif

char *string_copy(const char *) ;

Dictionary *dict_create(void) ;
Dictionary *dict_copy(Dictionary *) ;
void dict_free(Dictionary *) ;
void dict_set(Dictionary *, const char *, Value *) ;
Value *dict_get(Dictionary *, const char *) ;
void dict_del(Dictionary *, const char *) ;
void dict_iterate(Dictionary *, void(*f)(const char *, Value *)) ;


#ifdef __cplusplus
} ;
#endif

#endif
