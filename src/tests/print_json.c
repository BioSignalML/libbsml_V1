#include <stdio.h>
#include <jansson.h>

#include "print_json.h"


void print_json(json_t *json, FILE *f)
/*==================================*/
{
  if (json_is_object(json)) {
    const char *key ;
    json_t *value ;
    int n = 0 ;
    fprintf(f, "{ ") ;
    json_object_foreach(json, key, value) {
      if (n > 0) fprintf(f, ", ") ;
      fprintf(f, "'%s': ", key) ;
      print_json(value, f) ;
      ++n ;
      }
    fprintf(f, " }") ;
    }
  else if (json_is_array(json)) {
    size_t len = json_array_size(json) ;
    size_t n = 0 ;
    fprintf(f, "[ ") ;
    while (n < len) {
      if (n > 0) fprintf(f, ", ") ;
      print_json(json_array_get(json, n), f) ;
      ++n ;
      }
    fprintf(f, " ]") ;
    }
  else if (json_is_string(json)) {
    fprintf(f, "'%s'", json_string_value(json)) ;
    }
  else if (json_is_integer(json)) {
    fprintf(f, "%" JSON_INTEGER_FORMAT, json_integer_value(json)) ;
    }
  else if (json_is_real(json)) {
    fprintf(f, "%g", json_real_value(json)) ;
    }
  else if (json_is_true(json)) {
    fprintf(f, "T") ;
    }
  else if (json_is_false(json)) {
    fprintf(f, "F") ;
    }
  else if (json_is_null(json)) {
    fprintf(f, "NULL") ;
    }
  }


void print_json_line(json_t *json, FILE *f)
/*=======================================*/
{
  print_json(json, f) ;
  fprintf(f, "\n") ;
  }
