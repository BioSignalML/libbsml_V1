/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "bsml-internal.h"


int bsml_initialise(void)
/*=====================*/
{
  bsml_rdfgraph_initialise() ;

  //bsml_rdfmapping_initialise() ;

  bsml_stream_initialise() ;

  bsml_repository_initialise() ;
  }



void bsml_finish(void)
/*===================*/
{
  bsml_repository_finish() ;

  bsml_stream_finish() ;

  //bsml_rdfmapping_finish() ;
  bsml_rdfgraph_finish() ;
  }


void bsml_log_error(const char *format, ...)
/*========================================*/
{
  va_list ap ;
  va_start(ap, format) ;
  vfprintf(stderr, format, ap) ;
  va_end(ap) ;
  }


const char *string_copy(const char *s)
/*==================================*/
{
  if (s) {
    int l = strlen(s) + 1 ;
    char *t = malloc(l) ;
    memcpy(t, s, l) ;
    return t ;
    }
  else return NULL ;
  }


const char *string_cat(const char *s, const char *t)
/*================================================*/
{
  if (s && t) {
    int l = strlen(s) ;
    int m = strlen(t) + 1 ;
    char *u = malloc(l + m) ;
    memcpy(u,     s, l) ;
    memcpy(u + l, t, m) ;
    return u ;
    }
  else if (s)
    return string_copy(s) ;
  else
    return string_copy(t) ;
  }

