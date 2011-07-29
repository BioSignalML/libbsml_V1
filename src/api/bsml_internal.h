/******************************************************
 *
 *  BioSignalML Project API
 *
 *  Copyright (c) 2010-2011  David Brooks
 *
 *  $ID$
 *
 ******************************************************
 */

#ifndef _BSML_INTERNAL_H
#define _BSML_INTERNAL_H

#include <time.h>
#include <redland.h>

#include "../utility/dictionary.h"

enum {
  KIND_NONE     =   0,
  KIND_DATETIME =  10,
  KIND_URI      = 100,
  KIND_NODE
  } ;

typedef struct {
  int sign ;
  struct tm datetime ;
  unsigned int usecs ;
  } DateTime ;


#ifdef __cplusplus
extern "C" {
#endif

DateTime *DateTime_create(void) ;
void DateTime_free(DateTime *) ;

void dict_set_datetime(dict *, const char *, DateTime *) ;
DateTime *dict_get_datetime(dict *, const char *) ;

void dict_set_uri(dict *, const char *, librdf_uri *) ;
librdf_uri *dict_get_uri(dict *, const char *) ;

void dict_set_node(dict *, const char *, librdf_node *) ;
librdf_node *dict_get_node(dict *, const char *) ;

#ifdef __cplusplus
} ;
#endif

#endif
