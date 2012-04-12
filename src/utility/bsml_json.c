/*****************************************************
 *
 *  BioSignalML API
 *
 *  Copyright (c) 2010-2012  David Brooks
 *
 *  $ID$
 *
 *****************************************************/

#include "bsml_internal.h"
#include "utility/bsml_json.h"

const char *bsml_json_as_string(json_t *obj, const char *key)
/*=========================================================*/
{
  json_t *entry = json_object_get(obj, key) ;
  if (entry && json_is_string(entry)) return json_string_value(entry) ;
  else {
    bsml_log_error("Expected a string for '%s' JSON key\n", key) ;
    return NULL ;
    }
  }

int bsml_json_as_integer(json_t *obj, const char *key)
/*==================================================*/
{
  json_t *entry = json_object_get(obj, key) ;
  if (entry && json_is_integer(entry)) return json_integer_value(entry) ;
  else {
    bsml_log_error("Expected an integer for '%s' JSON key\n", key) ;
    return 0.0 ;
    }
  }

double bsml_json_as_number(json_t *obj, const char *key)
/*====================================================*/
{
  json_t *entry = json_object_get(obj, key) ;
  if (entry && json_is_number(entry)) return json_number_value(entry) ;
  else {
    bsml_log_error("Expected a number for '%s' JSON key\n", key) ;
    return 0.0 ;
    }
  }
