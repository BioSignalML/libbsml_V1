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

#include <stdlib.h>

#include "bsml_internal.h"


bsml_datetime *bsml_datetime_create(void)
/*===========================*/
{
  return (bsml_datetime *)calloc(sizeof(bsml_datetime), 1) ;
  }


void bsml_datetime_free(bsml_datetime *dt)
/*============================*/
{
  free(dt) ;
  }


void dict_set_datetime(dict *d, const char *key, bsml_datetime *dt)
/*==========================================================*/
{
  dict_set_pointer(d, key, (void *)dt, BSML_KIND_DATETIME, (Value_Free *)bsml_datetime_free) ;
  }

bsml_datetime *dict_get_datetime(dict *d, const char *key)
/*=================================================*/
{
  int kind ;
  bsml_datetime *dt = (bsml_datetime *)dict_get_pointer(d, key, &kind) ;
  if (dt && kind == BSML_KIND_DATETIME) return dt ;
  return NULL ;
  }


void dict_set_uri(dict *d, const char *key, librdf_uri *uri)
/*========================================================*/
{
  dict_set_pointer(d, key, (void *)uri, BSML_KIND_URI, (Value_Free *)librdf_free_uri) ;
  }

librdf_uri *dict_get_uri(dict *d, const char *key)
/*==============================================*/
{
  int kind ;
  librdf_uri *uri = (librdf_uri *)dict_get_pointer(d, key, &kind) ;
  if (uri && kind == BSML_KIND_URI) return uri ;
  return NULL ;
  }


void dict_set_node(dict *d, const char *key, librdf_node *node)
/*===========================================================*/
{
  dict_set_pointer(d, key, (void *)node, BSML_KIND_NODE, (Value_Free *)librdf_free_node) ;
  }

librdf_node *dict_get_node(dict *d, const char *key)
/*================================================*/
{
  int kind ;
  librdf_node *node = (librdf_node *)dict_get_pointer(d, key, &kind) ;
  if (node && kind == BSML_KIND_NODE) return node ;
  return NULL ;
  }
