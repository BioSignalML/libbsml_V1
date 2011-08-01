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
#include "values.h"


const char *string_copy(const char *s)
/*==================================*/
{
  if (s) {
    char *t = malloc(strlen(s)+1) ;
    strcpy(t, s) ;
    return t ;
    }
  else return NULL ;
  }


void value_free(Value *v)
/*=====================*/
{
  if (v->type == TYPE_STRING) free((void *)v->string) ;
  else if (v->delete) v->delete(v->pointer) ;
  }


VALUE_TYPE value_type(Value *v)
/*===========================*/
{
  return v->type ;
  }

void *value_get_pointer(Value *v, int *kind)
/*========================================*/
{
  if (kind) *kind = v->pointerkind ;
  return v->pointer ;
  }

const char *value_get_string(Value *v)
/*==================================*/
{
  return v->string ;
  }

long value_get_integer(Value *v)
/*============================*/
{
  return (long)v->integer ;
  }

double value_get_real(Value *v)
/*===========================*/
{
  return (double)v->real ;
  }
