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


DateTime *DateTime_create(void)
/*===========================*/
{
  return (DateTime *)calloc(sizeof(DateTime), 1) ;
  }


void DateTime_free(DateTime *dt)
/*============================*/
{
  free(dt) ;
  }


void dict_set_datetime(dict *d, const char *key, DateTime *dt)
/*==========================================================*/
{
  dict_set_pointer(d, key, (void *)dt, KIND_DATETIME, (Free_Function *)DateTime_free) ;
  }

DateTime *dict_get_datetime(dict *d, const char *key)
/*=================================================*/
{
  int kind ;
  DateTime *dt = (DateTime *)dict_get_pointer(d, key, &kind) ;
  if (dt && kind == KIND_DATETIME) return dt ;
  return NULL ;
  }


void dict_set_uri(dict *d, const char *key, librdf_uri *uri)
/*========================================================*/
{
  dict_set_pointer(d, key, (void *)uri, KIND_URI, (Free_Function *)librdf_free_uri) ;
  }

librdf_uri *dict_get_uri(dict *d, const char *key)
/*==============================================*/
{
  int kind ;
  librdf_uri *uri = (librdf_uri *)dict_get_pointer(d, key, &kind) ;
  if (uri && kind == KIND_URI) return uri ;
  return NULL ;
  }


void dict_set_node(dict *d, const char *key, librdf_node *node)
/*===========================================================*/
{
  dict_set_pointer(d, key, (void *)node, KIND_NODE, (Free_Function *)librdf_free_node) ;
  }

librdf_node *dict_get_node(dict *d, const char *key)
/*================================================*/
{
  int kind ;
  librdf_node *node = (librdf_node *)dict_get_pointer(d, key, &kind) ;
  if (node && kind == KIND_NODE) return node ;
  return NULL ;
  }
