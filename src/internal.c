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

#include <redland.h>
#include <libwebsockets.h>

#include "bsml-internal.h"

struct {
  librdf_world *rdfworld ;
  struct libwebsocket_context *ctx ;
  } bsml_world ;

bsml_world bsml = { NULL, NULL } ;

static struct libwebsocket_protocols protocols[] = {
    { STREAM_PROTOCOL, stream_callback, 0 },
    { NULL,            NULL,            0 }
  } ;


int bsml_initialise(void)
/*=====================*/
{
  bsml.world = librdf_new_world() ;
  if (bsml.world == NULL) {
    fprintf(stderr, "Creating Redland RDF world failed\n") ;
    return 1 ;
    }
  librdf_world_open(bsml.world) ;
  //bsml_rdfmapping_initialise() ;

  bsml.ctx = libwebsocket_create_context(CONTEXT_PORT_NO_LISTEN, NULL, protocols,
               libwebsocket_internal_extensions,  NULL, NULL, -1, -1, 0) ;
  if (bsml.ctx == NULL) {
    fprintf(stderr, "Creating libwebsocket context failed\n") ;
    return 1 ;
    }

  }


void bsml_finish(void)
/*===================*/
{
  if (bsml.ctx) libwebsocket_context_destroy(bsml.ctx) ;

  //bsml_rdfmapping_finish() ;
  if (bsml.world) librdf_free_world(bsml.world) ;
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

