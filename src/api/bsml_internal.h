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

#include "list.h"
#include "dictionary.h"

enum {                     // Kinds of pointer Values
  BSML_KIND_NONE     =   0,

  BSML_KIND_DATETIME =  10,

  BSML_KIND_RECORDING = 50,
  BSML_KIND_SIGNAL,
  BSML_KIND_EVENT,
  BSML_KIND_INSTANT,
  BSML_KIND_INTERVAL,
  BSML_KIND_TIMELINE,

  BSML_KIND_URI      = 100,
  BSML_KIND_NODE
  } ;


typedef struct {
  int sign ;
  struct tm datetime ;
  unsigned int usecs ;
  } bsml_datetime ;


#ifdef __cplusplus
extern "C" {
#endif

bsml_datetime *bsml_datetime_create(void) ;
void bsml_datetime_free(bsml_datetime *) ;

void dict_set_datetime(dict *, const char *, bsml_datetime *) ;
bsml_datetime *dict_get_datetime(dict *, const char *) ;

void dict_set_uri(dict *, const char *, librdf_uri *) ;
librdf_uri *dict_get_uri(dict *, const char *) ;

void dict_set_node(dict *, const char *, librdf_node *) ;
librdf_node *dict_get_node(dict *, const char *) ;

#ifdef __cplusplus
} ;
#endif

#endif
