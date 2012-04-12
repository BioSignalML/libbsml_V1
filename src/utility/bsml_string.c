/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-201  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include <string.h>
#include <stdlib.h>

#include "bsml_string.h"


const char *bsml_string_copy(const char *s)
/*=======================================*/
{
  if (s) {
    char *t = malloc(strlen(s)+1) ;
    strcpy(t, s) ;
    return t ;
    }
  else return NULL ;
  }

const char *bsml_string_cat(const char *s, const char *t)
/*=====================================================*/
{
  if (s && t) {
    char *u = malloc(strlen(s)+strlen(t)+1) ;
    strcpy(u, s) ;
    strcat(u, t) ;
    return u ;
    }
  else if (s)
    return bsml_string_copy(s) ;
  else
    return bsml_string_copy(t) ;
  }

void bsml_string_free(const char *s)
/*================================*/
{
  if (s) free((void *)s) ;
  }
